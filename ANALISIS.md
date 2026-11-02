# Análisis técnico — Bugs Bunny: Lost in Time (PC, 1999) → Windows 11

Fecha: 2026-10-30
Fuente: `Bugs Bunny Lost in Time.iso` (538.679.296 bytes, ISO 9660, etiqueta `BBLIT`)

## 1. Qué hay en el disco

```
Autorun.inf            open=Datas\BugsBunny.exe
Datas/BugsBunny.exe    lanzador/configurador (20 KB, OpenGL + DirectDraw)
Datas/bin/             bugs.exe (772.096 B), config.pc, saves, hv3dfx.dll
Datas/bze/             269 archivos .bze (niveles, modelos, texturas, sonidos)
Datas/track/           32 pistas .xa (música)
Datas/speeches/        3 contenedores .xa (voces, ~108 MB)
Datas/data/            default.bmp (fondo de carga)
Datas/Ereg/, Install/  registro, DirectX 6, Acrobat, Chat'n Play (no necesarios)
Help/                  readme + manual PDF
```

Es la edición **North America 1.6** (Inglés/Francés/Español), la más común.

## 2. El ejecutable

| Dato | Valor |
|---|---|
| Formato | PE32, Intel i386, GUI, sin empaquetar ni proteger |
| SHA-256 | `6e15f92076614e3ee218c34c5dee566bf273e5bfee77d831cfbf78aa2d29e235` |
| ImageBase / EntryPoint | `0x400000` / `0x51eb0` |
| Secciones | 6 (`.text` 369 KB de código, `.rdata`, `.data`, `.idata`, `.rsrc`, `.reloc`) |
| Compilador | MSVC 6.0 (CRT clásico, `_beginthreadex`, `GetVersionExA`) |
| Exportaciones | ninguna (todo interno, sin símbolos) |
| Timestamp | 1999-09-27 |

### Dependencias reales (tabla de imports)

| DLL | Qué usa | Implicación |
|---|---|---|
| `KERNEL32` | archivos, memoria, hilos, CD (`GetDriveTypeA`, `GetVolumeInformationA`) | portable |
| `USER32` | ventana, `PeekMessageA`, `GetKeyboardState` | **sin DirectInput** |
| `GDI32` | `ChoosePixelFormat`, `SetPixelFormat`, `SwapBuffers`, `StretchDIBits`, paletas | OpenGL vía GDI + blit software |
| `DDRAW` | solo `DirectDrawCreate`, `DirectDrawEnumerateA` | **sin Direct3D** |
| `DSOUND` | solo `DirectSoundCreate` | **sin DirectMusic** |
| `ole32` | `CoInitialize`, `CoCreateInstance` | probablemente el registro/instalador |
| `ADVAPI32` | claves `Software\Infogrames\Bugs Bunny Lost In Time` | configuración |

`opengl32.dll` **no** está en la tabla de imports: se carga con `LoadLibraryA` en tiempo
de ejecución (por eso funciona el truco de `hv3dfx.dll` para 3dfx).

### Renderers y opciones de línea de comandos

El motor trae cuatro caminos de dibujado, seleccionables por argumento:

| Flag | Renderer |
|---|---|
| `/opengl`, `/ogl` | OpenGL (hardware) |
| `/soft24`, `/soft16`, `/soft8`, `/softrgb`, `/softpal`, `/soft` | software por `StretchDIBits` |
| `/dll:<nombre>` | MiniGL alternativo (por defecto `hv3dfx.dll`) |
| `/pal:<archivo>` | paleta externa |

Otros flags encontrados en el binario: `/win` (ventana), `/full`, `/x<ancho>`,
`/y<alto>` (viewport interno), `/b<n>` (arrancar en un banco/nivel), `/r:`, `/p:`,
`/skip_intro`, `/no_skip`, `/no_sync`, `/fntp`, `/cons`.

Configuración por defecto: `config.pc` = 512×384, 16 bpp, fullscreen. El readme original
explica además las claves `CheckGDI=1` y `DLL=0` de `bin\bugs.ini`.

### Cómo carga los datos

- Rutas relativas al ejecutable: `..\bze\`, `..\bin\`, `..\track\`, `..\speeches\`.
- Los nombres se piden con sufijo ISO 9660 (`..\BZE\L01A.BZE;1`): el juego está escrito
  para leer **del CD**. Sin CD o sin parche, no arranca.
- Contenedor `.bze`: cabecera de 8 bytes (flag de compresión + número de entradas),
  tabla de entradas de 12 bytes `{id, tamaño real, tamaño alineado a 0x800}`, datos
  alineados a 0x800. Compresión propia de Behaviour (misma familia que *Jersey Devil*,
  *Bugs Bunny & Taz*, *The Grinch*). Hay 269 `.bze` en el disco.
- Audio: `.xa` con ADPCM de PlayStation, decodificado por el propio motor y reproducido
  por DirectSound.
- Lógica a **30 Hz fijos** (posición, cámara y animación avanzan por frame).

### Por qué se rompe en Windows moderno

1. DirectDraw en modo exclusivo y modos de video de 1999 (512×384, 16 bpp).
2. Ventana 4:3 con HUD dibujado en coordenadas fijas.
3. Chequeo de CD por volumen/etiqueta y rutas `;1`.
4. 30 fps atados a la lógica (subir el límite acelera el juego).
5. Escalado DPI de Windows y HDR.
6. DirectSound con formatos viejos (funciona, pero con latencia/glitches).

## 3. Opciones de port

### Opción A — Paquete "drop-in" con renderer moderno (elegida)

Un `ddraw.dll` proxy (BBLIT Remaster 1.0-beta2, MIT) intercepta el dibujado y lo
reemplaza por OpenGL 4.6: interpolación a cualquier refresco, widescreen Hor+, más
distancia de dibujado, MSAA, mando XInput y **sin CD** (parchea el chequeo en memoria).
El `bugs.exe` original no se toca.

- Esfuerzo: horas (ya está armado en `Bugs Bunny Lost in Time (Win11)/`).
- Resultado: doble clic en `JUGAR.cmd`, sin CD, sin dgVoodoo, 16:9, 60–144 Hz.
- Riesgo: el mod está en beta y **no fue probado en Windows por sus autores** (sí en
  Linux/Proton). Si falla, se borra `ddraw.dll` y queda el juego original.
- Ya soporta exactamente nuestro `bugs.exe` (hash `6e15f920…`).

### Opción B — Envoltorio propio, sin mods de terceros

Lanzador propio + dgVoodoo2/DXVK + `config.pc`/`bugs.ini` ajustados + parche no-CD
propio. Todo bajo control, pero mantiene 4:3 y 30 fps: no resuelve el problema de
fondo, solo la compatibilidad.

- Esfuerzo: 1–2 días (el parche no-CD hay que ubicarlo en el binario).
- Riesgo: bajo. Calidad: la del original.

### Opción C — Port nativo x64 desde decompilación

Partir de `quantumdude836/BugsDecomp` (decompilación del PC 1.0 y 1.6; código en DLL
inyectada que reemplaza funciones por dirección) y convertirla en un ejecutable nativo:
renderer propio (SDL3 + OpenGL/Vulkan), audio propio (miniaudio + decodificador XA),
input propio, y cargador de `.bze`/`.xa` en 64 bits.

- Esfuerzo: 2–6 meses de ingeniería inversa seria (Ghidra + REA).
- Ventaja: preservación real, sin depender de binarios de 1999 ni de Wine/Proxies.
- Riesgo: alto; el decompilado no tiene nombres originales y hay que reconstruir
  estructuras de datos del motor.

### Opción D — Reimplementación de motor (estilo OpenRCT2/OpenLara)

Escribir un motor nuevo que cargue los `.bze` y `.xa` y reimplemente la lógica de juego
a partir del comportamiento observado.

- Esfuerzo: 1–2 años, equipo.
- No recomendado para este objetivo.

### Opción E — Evolucionar el mod (punto medio)

Fork de BBLIT Remaster: empaquetar todo en un único `.exe` lanzador, compilar el proxy
en 64 bits, añadir upscale de texturas y un instalador. Semanas, no meses.

## 4. Recomendación

1. Usar la **Opción A** ya armada. Es la que cumple el objetivo hoy: un doble clic,
   sin CD, sin configuración, en Windows 11.
2. Si en tu PC el mod falla, caer a **Opción B** (borrar `ddraw.dll` y usar el renderer
   software o dgVoodoo2).
3. Si el objetivo a largo plazo es preservar el juego sin depender de binarios de 1999,
   abrir la **Opción C** usando `BugsDecomp` como base y este análisis como punto de
   partida.

## 5. Lanzador propio (`Bugs Bunny Lost in Time.exe`)

Compilado desde `src/launcher.c` con MinGW-w64 (x86_64, GUI, sin dependencias más
allá de la UCRT del sistema). Hace tres cosas:

1. Lee `BIN\remaster.ini` y muestra una ventana con refresco, MSAA, distancia de
   dibujado, brillo PS1, anclaje del HUD y mando XInput.
2. Reescribe el `.ini` con lo elegido (`[remaster]` y `[live]`).
3. Lanza `BIN\bugs.exe` con `/opengl /win /b0 /x800 /y600` y directorio de trabajo
   `BIN`, que es lo que espera el mod.
4. Escribe la clave de registro que antes ponía el instalador (ver 5.1).

No modifica el `bugs.exe` ni el `ddraw.dll`: solo escribe el texto de configuración que
el mod relee. `JUGAR.cmd` queda como vía alternativa sin ventana.

### 5.1 Requisitos de arranque del `bugs.exe` (descubiertos con REA)

`WinMain` (0x405950) sale **limpiamente** (`xor eax,eax; ret 0x10` en 0x405d14) en tres
casos, sin ventana, sin mensaje y sin excepción. Eso explica que el proceso termine 6 ms
después de cargar el mod sin dejar rastro:

| Comprobación | Dirección | Efecto |
|---|---|---|
| Ya hay una ventana `BBLIT Game` y no está minimizada | 0x4059a9 | sale (instancia única) |
| `RegOpenKeyExA(HKCU,"Software\\Infogrames\\Bugs Bunny Lost In Time",KEY_READ)` falla | 0x405a6d | sale |
| `RegQueryValueExA(...,"installation path",...)` falla | 0x405abf | sale |
| Detecta CD-ROM con etiqueta `BBLIT` (el mod lo parchea) | 0x405c26 | sale (diálogo) |

Con `installation path` el juego arma `<ruta>\bin` y busca ahí el motor; por eso debe
apuntar a la carpeta que contiene `BIN`, no a `BIN`. Nunca se ejecutó el `Setup.exe`
original (copiamos los archivos del ISO), así que la clave no existía: es la causa de que
el juego no abriera. El lanzador y `JUGAR.cmd` la escriben en cada arranque apuntando a su
propia carpeta, lo que además permite mover el juego a cualquier ruta o a otro PC.

## 7. macOS (evaluado, no implementado)

- El mod exige **OpenGL 4.6**; macOS llega como máximo a **4.1** (y está deprecado desde
  Mojave). Por eso el mod no puede correr en macOS ni con Wine.
- `bugs.exe` es **PE32 de 32 bits**. Rosetta 2 solo traduce x86-64, así que un Mac con
  Apple Silicon no lo ejecuta de forma nativa.
- CrossOver/Wine sí emulan 32 bits y hay reportes de que el juego arranca, pero sumando
  la traducción de OpenGL a Metal y el chequeo de CD (un ISO montado no se presenta como
  unidad CD-ROM), el resultado es experimental.
- Un port nativo real implica recompilar el motor desde la decompilación: mismo alcance
  que la Opción C (meses), no un ajuste de fin de semana.

## 8. Verificación pendiente (requiere Windows)

- Arranque real del lanzador y revisión de `BIN\remaster.log`.
- Comprobar que la ventana de opciones se ve bien a 1920×1080 y a 125/150 % de DPI.
- Prueba de las cinco eras (el mod solo probó a fondo la primera).
- Comprobar guardado/carga (`BIN\Savegame*.dat`).
- Medir fps y estabilidad con VSync activado.
