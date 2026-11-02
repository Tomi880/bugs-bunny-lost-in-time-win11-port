#!/bin/sh
# Compila el lanzador de Windows desde macOS.
# Requiere: brew install mingw-w64
set -e
cd "$(dirname "$0")"

CC=${CC:-x86_64-w64-mingw32-gcc}
WINDRES=${WINDRES:-x86_64-w64-mingw32-windres}

OUT="../Bugs Bunny Lost in Time (Win11)/Bugs Bunny Lost in Time.exe"

"$WINDRES" --codepage=65001 -I. launcher.rc -O coff -o launcher.res
"$CC" -O2 -s -mwindows -finput-charset=UTF-8 \
    -o "$OUT" launcher.c launcher.res -lshell32 -ladvapi32

echo "OK: $OUT"
