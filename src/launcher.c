/* Lanzador con opciones para Bugs Bunny: Lost in Time (PC 1999).
   Escribe BIN\remaster.ini (BBLIT Remaster) y arranca BIN\bugs.exe.
   Compilado como app de escritorio Win32, sin dependencias externas. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include "resource.h"

#define NELEM(a) (int)(sizeof(a) / sizeof((a)[0]))

static const int kFps[]   = { 0, 60, 90, 120, 144, 30 };
static const int kMsaa[]  = { 4, 8, 2, 0 };
static const int kDraw[]  = { 200, 100, 150, 300 };
static const int kLight[] = { 18, 20, 10, 0 };
static const int kHud[]   = { 178, 133, 0 };

static const wchar_t *kFpsText[] = {
    L"Monitor (refresco del sistema)", L"60 fps", L"90 fps", L"120 fps",
    L"144 fps", L"30 fps (como el original)"
};
static const wchar_t *kMsaaText[] = {
    L"4x (recomendado)", L"8x", L"2x", L"Desactivado"
};
static const wchar_t *kDrawText[] = {
    L"200% (recomendado)", L"100% (original)", L"150%", L"300%"
};
static const wchar_t *kLightText[] = {
    L"18 (recomendado)", L"20 (igual que PS1)", L"10 (como el port de PC)", L"0 (sin cambio)"
};
static const wchar_t *kHudText[] = {
    L"16:9 (bordes de pantalla)", L"4:3 (posiciones originales)", L"Siempre en los bordes"
};

static wchar_t g_dir[MAX_PATH];
static wchar_t g_bin[MAX_PATH];
static wchar_t g_ini[MAX_PATH];

static void init_paths(void)
{
    DWORD n = GetModuleFileNameW(NULL, g_dir, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return;
    wchar_t *slash = wcsrchr(g_dir, L'\\');
    if (slash) *slash = 0;
    wsprintfW(g_bin, L"%s\\BIN", g_dir);
    wsprintfW(g_ini, L"%s\\remaster.ini", g_bin);
}

static int read_key(const char *key, int def)
{
    HANDLE h = CreateFileW(g_ini, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return def;

    DWORD size = GetFileSize(h, NULL), got = 0;
    char *buf = (char *)HeapAlloc(GetProcessHeap(), 0, size + 1);
    if (!buf) { CloseHandle(h); return def; }
    ReadFile(h, buf, size, &got, NULL);
    CloseHandle(h);
    buf[got] = 0;

    int result = def;
    size_t klen = strlen(key);
    for (char *p = buf; p && *p; ) {
        char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        while (len && (p[len - 1] == '\r' || p[len - 1] == ' ' || p[len - 1] == '\t')) len--;
        if (len > klen && !_strnicmp(p, key, klen) && p[klen] == '=')
            result = atoi(p + klen + 1);
        p = eol ? eol + 1 : NULL;
    }
    HeapFree(GetProcessHeap(), 0, buf);
    return result;
}

static BOOL write_ini(int fps, int msaa, int draw, int light, int hud, int xinput)
{
    char buf[1024];
    int n = wsprintfA(buf,
        "; Generado por el lanzador. Las claves no listadas usan el valor por defecto del mod.\r\n"
        "[remaster]\r\nfps=%d\r\nmsaa=%d\r\nxinput=%d\r\n\r\n"
        "[live]\r\ndraw_dist=%d\r\nps1_light=%d\r\nhud_aspect=%d\r\nhaze=0\r\n\r\n"
        "[level4]\r\nhaze=100\r\n\r\n[level5]\r\nhaze=100\r\n\r\n"
        "[level7]\r\nhaze=100\r\n\r\n[level8]\r\nhaze=100\r\n",
        fps, msaa, xinput, draw, light, hud);

    HANDLE h = CreateFileW(g_ini, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    DWORD written = 0;
    WriteFile(h, buf, (DWORD)n, &written, NULL);
    CloseHandle(h);
    return written == (DWORD)n;
}

static void fill_combo(HWND dlg, int id, const wchar_t **text, const int *vals,
                       int count, int current)
{
    HWND cb = GetDlgItem(dlg, id);
    int sel = 0;
    for (int i = 0; i < count; i++) {
        SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM)text[i]);
        if (vals[i] == current) sel = i;
    }
    SendMessageW(cb, CB_SETCURSEL, sel, 0);
}

static int combo_value(HWND dlg, int id, const int *vals)
{
    int sel = (int)SendMessageW(GetDlgItem(dlg, id), CB_GETCURSEL, 0, 0);
    return sel < 0 ? vals[0] : vals[sel];
}

static void apply_defaults(HWND dlg)
{
    SendMessageW(GetDlgItem(dlg, IDC_FPS), CB_SETCURSEL, 0, 0);
    SendMessageW(GetDlgItem(dlg, IDC_MSAA), CB_SETCURSEL, 0, 0);
    SendMessageW(GetDlgItem(dlg, IDC_DRAW), CB_SETCURSEL, 0, 0);
    SendMessageW(GetDlgItem(dlg, IDC_LIGHT), CB_SETCURSEL, 0, 0);
    SendMessageW(GetDlgItem(dlg, IDC_HUD), CB_SETCURSEL, 0, 0);
    CheckDlgButton(dlg, IDC_XINPUT, BST_CHECKED);
}

/* El juego exige la clave que escribe el instalador original; sin ella
   WinMain retorna 0 sin abrir ventana (comprobado en 0x405a6d). */
static DWORD set_install_path(void)
{
    char path[MAX_PATH];
    HKEY key;
    DWORD err;
    WideCharToMultiByte(CP_ACP, 0, g_dir, -1, path, MAX_PATH, NULL, NULL);
    err = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Infogrames\\Bugs Bunny Lost In Time",
                          0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL);
    if (err != ERROR_SUCCESS) return err;
    err = RegSetValueExA(key, "installation path", 0, REG_SZ, (const BYTE *)path,
                         (DWORD)lstrlenA(path) + 1);
    RegCloseKey(key);
    return err;
}

static void log_launch(const wchar_t *cmd, BOOL ok, DWORD err, DWORD reg)
{
    wchar_t line[1200];
    char ansi[2400], file[MAX_PATH];
    wsprintfW(line, L"%s\r\nCreateProcess=%d err=%lu\r\nRegistro=%lu\r\n", cmd, ok ? 1 : 0,
              (unsigned long)err, (unsigned long)reg);
    int n = WideCharToMultiByte(CP_ACP, 0, line, -1, ansi, sizeof ansi, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, g_dir, -1, file, MAX_PATH, NULL, NULL);
    lstrcatA(file, "\\lanzador.log");
    HANDLE h = CreateFileA(file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(h, ansi, (DWORD)(n > 0 ? n - 1 : 0), &written, NULL);
    CloseHandle(h);
}

/* Arranca el juego con los argumentos que espera el mod. TRUE si pudo. */
static BOOL launch_game(HWND owner)
{
    wchar_t exe[MAX_PATH];
    wsprintfW(exe, L"%s\\bugs.exe", g_bin);
    if (GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(owner,
            L"No se encuentra BIN\\bugs.exe.\n\n"
            L"Descomprime la carpeta completa del juego antes de jugar.",
            L"Bugs Bunny: Lost in Time", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    wchar_t cmd[MAX_PATH + 64];
    DWORD reg = set_install_path();
    wsprintfW(cmd, L"\"%s\" /opengl /win /b0 /x800 /y600", exe);

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof si);
    si.cb = sizeof si;
    BOOL ok = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, g_bin, &si, &pi);
    log_launch(cmd, ok, ok ? 0 : GetLastError(), reg);
    if (!ok) {
        MessageBoxW(owner, L"No se pudo iniciar bugs.exe.", L"Bugs Bunny: Lost in Time",
                    MB_ICONERROR | MB_OK);
        return FALSE;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}

static void save_and_play(HWND dlg)
{
    int xinput = IsDlgButtonChecked(dlg, IDC_XINPUT) == BST_CHECKED ? 1 : 0;
    write_ini(combo_value(dlg, IDC_FPS, kFps),
              combo_value(dlg, IDC_MSAA, kMsaa),
              combo_value(dlg, IDC_DRAW, kDraw),
              combo_value(dlg, IDC_LIGHT, kLight),
              combo_value(dlg, IDC_HUD, kHud),
              xinput);
    if (launch_game(dlg))
        EndDialog(dlg, IDOK);
}

static INT_PTR CALLBACK dialog_proc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        fill_combo(dlg, IDC_FPS, kFpsText, kFps, NELEM(kFps), read_key("fps", 0));
        fill_combo(dlg, IDC_MSAA, kMsaaText, kMsaa, NELEM(kMsaa), read_key("msaa", 4));
        fill_combo(dlg, IDC_DRAW, kDrawText, kDraw, NELEM(kDraw), read_key("draw_dist", 200));
        fill_combo(dlg, IDC_LIGHT, kLightText, kLight, NELEM(kLight), read_key("ps1_light", 18));
        fill_combo(dlg, IDC_HUD, kHudText, kHud, NELEM(kHud), read_key("hud_aspect", 178));
        CheckDlgButton(dlg, IDC_XINPUT, read_key("xinput", 1) ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDOK:
            save_and_play(dlg);
            return TRUE;
        case IDC_RESET:
            apply_defaults(dlg);
            return TRUE;
        case IDC_OPENHELP: {
            wchar_t help[MAX_PATH];
            wsprintfW(help, L"%s\\LEEME.md", g_dir);
            ShellExecuteW(dlg, L"open", help, NULL, g_dir, SW_SHOWNORMAL);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    (void)lp;
    return FALSE;
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdline, int show)
{
    (void)prev;
    (void)cmdline;
    (void)show;
    init_paths();
    /* Si el diálogo no se puede crear, igual arrancamos con el .ini que haya. */
    if (DialogBoxW(inst, MAKEINTRESOURCEW(IDD_MAIN), NULL, dialog_proc) == -1)
        launch_game(NULL);
    return 0;
}
