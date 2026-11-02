# Bugs Bunny: Lost in Time — edición Windows 10/11

Juego de PC de 1999 (Infogrames / Behaviour) extraído del disco original y preparado
para ejecutarse en Windows moderno con un solo doble clic, sin CD, sin dgVoodoo y sin
tocar la configuración del equipo.

## Cómo jugar

1. Copia esta carpeta completa a tu PC (por ejemplo `C:\Juegos\Bugs Bunny`).
2. Doble clic en **`Bugs Bunny Lost in Time.exe`**.

Se abre una ventana de opciones (refresco, antialiasing, distancia, brillo, HUD y
mando). Ahí eliges y pulsas **Jugar**. Esas opciones se guardan en `BIN\remaster.ini`,
así que la próxima vez arranca con lo que dejaste.

No hay instalador. No hace falta montar el ISO ni tener el CD.

`JUGAR.cmd` hace lo mismo sin ventana de opciones, por si quieres saltarte el menú
o el `.exe` no arranca en tu equipo.

Si quieres un acceso directo en el escritorio: clic derecho sobre
`Bugs Bunny Lost in Time.exe` → "Enviar a" → "Escritorio (crear acceso directo)".

## Requisitos

- Windows 10 u 11 de 64 bits.
- GPU con **OpenGL 4.6** (cualquier AMD, NVIDIA o Intel de los últimos ~10 años; con
  drivers al día). En GPUs más antiguas, ver "Si no arranca".
- Opcional: mando XInput (Xbox, Steam Deck, DualSense vía Steam Input). El teclado
  funciona igual que en el original.

## Resolución y refresco

- **Resolución:** el juego se dibuja a la **resolución nativa del monitor**, sin
  escalado ni barras negras. En un portátil de 1920×1080 se ve a 1080p; en un monitor
  4K, a 4K; en 2560×1440, a 1440p. El widescreen es Hor+: se ve más mundo a los lados,
  no se estira la imagen.
- **Refresco:** la lógica del juego corre siempre a 30 Hz (como el original). El
  refresco que eliges es el del **dibujado**: a 144 Hz se generan frames intermedios
  entre cada tick de lógica, así que el movimiento se ve fluido sin acelerar el juego.
  Opciones: monitor (lo que tengas), 60, 90, 120, 144 o 30 fps.
- **HUD:** se puede anclar a los bordes de la pantalla (16:9), dejar en las posiciones
  originales de 4:3, o forzarlo siempre a los bordes en pantallas ultrapanorámicas.

## Qué se hizo

| Elemento | Detalle |
|---|---|
| Origen | `Bugs Bunny Lost in Time.iso` (Infogrames, NA 1.6) |
| Ejecutable | `BIN\bugs.exe`, SHA-256 `6e15f920…d29e235`, sin parchear |
| Renderer moderno | `BIN\ddraw.dll` — **BBLIT Remaster 1.0-beta2** (MIT, código en github.com/InLineR495/BBLIT-Remaster) |
| Ajustes | `BIN\remaster.ini` |
| Lanzador | `Bugs Bunny Lost in Time.exe` (Win32 x64, 26 KB, fuente en `src/`) |
| Arranque | `bugs.exe /opengl /win /b0 /x800 /y600` |

El juego en sí **no está modificado**: si borras `ddraw.dll` vuelve a ser el original
de 1999 (4:3, 30 fps, requiere CD). El mod es un proxy de `ddraw.dll` que sustituye
solo el dibujado y los hooks de compatibilidad.

Qué aporta el mod:

- 60/120/144 Hz reales (la lógica sigue a 30 ticks; se interpolan los frames).
- Widescreen 16:9 / 16:10 sin estirar (Hor+, el HUD se ancla a los bordes).
- El doble de distancia de dibujado, niebla y brillo tipo PS1.
- 4× MSAA y corrección de las transparencias del port de PC.
- Mando XInput con movimiento analógico y cámara con el stick derecho.
- **Sin CD**: el chequeo de disco se desactiva en memoria.
- Correcciones de bugs del original (piedras, pozos del hub, sonidos en loop, etc.).

## Si no arranca

0. **No abras `BIN\bugs.exe`.** Ese es el motor y necesita los argumentos que le pasa
   el lanzador (`/opengl /win /b0 ...`). Sin ellos entra en una ruta de arranque que
   busca `LANGUAGE.BZE`, un archivo que este disco no trae, y se cierra al instante.
   Si el lanzador falla, usa `JUGAR.cmd`, que pasa los mismos argumentos.
   Al intentar jugar, el lanzador deja `lanzador.log` junto a él con la línea de
   comandos exacta que usó y el estado del registro: mándamela si algo falla.
1. **El juego exige una clave del registro** que escribía el instalador original
   (`HKCU\Software\Infogrames\Bugs Bunny Lost In Time` → `installation path`).
   Sin ella se cierra sin abrir ventana. El lanzador y `JUGAR.cmd` la escriben
   solos en cada ejecución apuntando a la carpeta donde estén, así que la carpeta
   puede moverse o copiarse a otro PC sin reinstalar nada. Si moviste los archivos
   a mano, abre `JUGAR.cmd` una vez.
2. **Comprueba `BIN\remaster.log`.** Debe empezar con `BBLIT Remaster 1.0-beta2`.
   Si dice "unsupported bugs.exe", el `bugs.exe` no es el esperado: no lo parchees
   ni lo cambies, usa el de esta carpeta.
   En la línea `LateInit: cmdline=` deben aparecer `/opengl /win /b0 /x800 /y600`;
   si no están, el juego se lanzó sin el lanzador.
3. **GPU sin OpenGL 4.6** (equipos viejos, VMs, Remote Desktop): renombra
   `BIN\ddraw.dll` a `ddraw.dll.off` y ejecuta `JUGAR.cmd` otra vez. Arrancará el
   juego original en su renderer software (4:3, 30 fps, escalado por Windows).
   También puedes probar `/soft24` en lugar de `/opengl` dentro de `JUGAR.cmd`.
4. **Pantalla en negro o cierre al inicio:** activa VSync para `bugs.exe` en el panel
   del driver (NVIDIA: Sincronización vertical → Activada; AMD: Espera vertical →
   Siempre activada). El mod sincroniza los frames contra VSync.
5. **Escalado de Windows > 100 %:** el mod se pone DPI-aware solo; si aun así se ve
   borroso, pon el `.cmd` en "Propiedades → Compatibilidad → Cambiar configuración de
   PPP alta → Anular el comportamiento de escalado".
6. **HDR activado** puede lavar los colores: desactívalo para este juego.

## Ajustes (`BIN\remaster.ini`)

Se puede editar con el juego abierto: la sección `[live]` se relee cada segundo.
El lanzador reescribe este archivo con lo que elijas en su ventana; si prefieres
tocar valores a mano (por ejemplo `haze` por nivel), edita el `.ini` y arranca con
`JUGAR.cmd` para que no se sobrescriba.

| Clave | Sección | Por defecto | Qué hace |
|---|---|---|---|
| `fps` | `[remaster]` | `0` | 0 = refresco del monitor, -1 = múltiplo de 30 más cercano, número = tope propio |
| `msaa` | `[remaster]` | `4` | Antialiasing: 0, 2, 4, 8 |
| `xinput` | `[remaster]` | `1` | 1 = mando moderno, 0 = manejo original |
| `cam_speed`, `cam_invert` | `[remaster]` | `52`, `0` | Velocidad e inversión de la cámara |
| `draw_dist` | `[live]` | `200` | Distancia de dibujado, % del original |
| `actor_dist` | `[live]` | `400` | Distancia de personajes y objetos, % |
| `ps1_light` | `[live]` | `18` | Brillo del mundo ×10 (20 = como PS1, 10 = como el port de PC) |
| `hud_aspect` | `[live]` | `178` | Ancho máximo del HUD (178 = marco 16:9) |
| `haze` | `[live]`, `[levelN]` | `0`/`100` | Niebla de horizonte |

## Controles con mando

| Botón | Acción |
|---|---|
| Stick izquierdo | Mover (inclinación leve = caminar, total = correr) |
| Stick derecho | Girar cámara |
| LT / RT | Girar cámara |
| A / B / X / Y | ✕ / ○ / □ / △ de PS1 |
| LB / RB | L1 / R1 |
| Start | Pausa / "Press Enter" en el menú |
| Back | Pausa / volver en menús |

## Estado y limitaciones

- El mod está en **beta**. Está probado a fondo en Linux/Proton; **en Windows no está
  probado por sus autores**. Si algo falla, revisa `BIN\remaster.log`.
- Las texturas son las originales (no hay remaster de texturas).
- Solo funciona con las ediciones listadas por el mod; esta carpeta usa la NA 1.6,
  que es una de las soportadas.

## Legal

- Necesitas tu propia copia del juego. Esta carpeta se generó a partir del ISO que
  tienes; no contiene ni redistribuye nada que no viniera del disco.
- `ddraw.dll` es BBLIT Remaster, licencia MIT: ver `LICENCIAS/`.
- Bugs Bunny y Looney Tunes son marcas de Warner Bros. Este paquete no está
  afiliado a Warner Bros., Infogrames ni a los autores del mod.
