# Bugs Bunny: Lost in Time — edición Windows 11

*Bugs Bunny: Lost in Time* (PC, 1999) en Windows 10/11 modernos,
sin dgVoodoo, sin CD y sin configurar nada a mano.

El juego original se vendió para Windows 95/98 y hoy arranca mal o directamente no arranca:
512×384 fijo, 4:3, renderer software lento y chequeo de CD. Este paquete lo deja en un
`.exe` de doble clic, con resolución nativa, widescreen y refresco configurable.

## Cómo jugar

1. Descomprime la carpeta completa (no ejecutes nada desde dentro del ZIP).
2. Doble clic en **`Bugs Bunny Lost in Time.exe`**.
3. Elige refresco, antialiasing, HUD y mando → **Jugar**.

`JUGAR.cmd` es la vía de respaldo, sin ventana de opciones. **No abras `BIN\bugs.exe`**:
es el motor y necesita los argumentos que le pasa el lanzador.

Detalle completo (controles, ajustes, solución de problemas): `Bugs Bunny Lost in Time (Win11)/LEEME.md`.

## Requisitos

- Windows 10 u 11, 64 bits.
- GPU con OpenGL 4.6 (GTX 900 / RX 400 / Intel Gen9 en adelante, drivers al día).
- 500 MB de disco. Mando XInput opcional.

## Qué hay en el repo

| Ruta | Qué es |
|---|---|
| `Bugs Bunny Lost in Time (Win11)/` | El juego listo para jugar, con el mod y el lanzador |
| `src/` | Fuente del lanzador (C, Win32 puro) y del extractor de icono |
| `ANALISIS.md` | Ingeniería inversa: binarios, imports, assets, opciones de port y hallazgos del arranque |

## Cómo está hecho

- **Assets y motor:** copiados del ISO original (edición North America 1.6).
- **Renderer:** [BBLIT-Remaster](https://github.com/InLineR495/BBLIT-Remaster) 1.0-beta2 (MIT),
  un proxy `ddraw.dll` que sustituye DirectDraw por un pipeline OpenGL 4.6 con interpolación de
  frames, widescreen Hor+, distancia de dibujado ampliada y parche no-CD en memoria.
- **Lanzador:** propio, Win32 x64, sin dependencias. Lee `BIN\remaster.ini`, muestra las opciones,
  escribe la clave de registro que exigía el instalador original (`installation path`) y arranca
  `BIN\bugs.exe` con directorio de trabajo `BIN`.
- **Análisis:** hecho con REA sobre el `bugs.exe` (PE32, MSVC 6, sin empaquetar).

Recompilar el lanzador (desde macOS o Linux con MinGW-w64):

```sh
brew install mingw-w64
cd src && ./build.sh
```

## Licencias y créditos

- `src/` y `ANALISIS.md`: MIT.
- BBLIT-Remaster: MIT, © InLineR495. Texto en `Bugs Bunny Lost in Time (Win11)/LICENCIAS/`.
- **El juego y sus assets son propiedad de Infogrames / Behaviour Interactive.**
