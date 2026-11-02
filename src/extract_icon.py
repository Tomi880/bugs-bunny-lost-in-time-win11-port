#!/usr/bin/env python3
"""Extrae el primer grupo de iconos de un PE a un .ico (sin dependencias)."""
import struct, sys

def main(pe_path, out_path):
    d = open(pe_path, 'rb').read()
    e = struct.unpack_from('<I', d, 0x3c)[0]
    nsec = struct.unpack_from('<H', d, e + 6)[0]
    optsz = struct.unpack_from('<H', d, e + 20)[0]
    sec = e + 24 + optsz
    rsrc = None
    for i in range(nsec):
        o = sec + i * 40
        if d[o:o+8].rstrip(b'\0') == b'.rsrc':
            vsz, va, rsz, roff = struct.unpack_from('<IIII', d, o + 8)
            rsrc = (va, roff, rsz)
    if not rsrc:
        sys.exit("sin .rsrc")

    va, roff, rsz = rsrc
    base = roff

    def off_of(rva):
        return base + (rva - va)

    def entries(node_off):
        n_named, n_id = struct.unpack_from('<HH', d, node_off + 12)
        out = []
        for i in range(n_named + n_id):
            name, tgt = struct.unpack_from('<II', d, node_off + 16 + i * 8)
            out.append((name, tgt))
        return out

    root = base

    def walk(node_off):
        n_named, n_id = struct.unpack_from('<HH', d, node_off + 12)
        for i in range(n_named + n_id):
            name, tgt = struct.unpack_from('<II', d, node_off + 16 + i * 8)
            yield name, tgt

    def is_dir(tgt):
        return bool(tgt & 0x80000000)

    groups = {}   # id -> bytes
    icons = {}    # id -> bytes
    for name, tgt in walk(root):
        rtype = name & 0xffff if not (name & 0x80000000) else None
        if not is_dir(tgt):
            continue
        for name2, tgt2 in walk(base + (tgt & 0x7fffffff)):
            rid = name2 & 0xffff if not (name2 & 0x80000000) else None
            if not is_dir(tgt2):
                continue
            for name3, tgt3 in walk(base + (tgt2 & 0x7fffffff)):
                if is_dir(tgt3):
                    continue
                data = struct.unpack_from('<II', d, base + tgt3)[0:2]
                rva, size = data
                blob = d[off_of(rva):off_of(rva) + size]
                if rtype == 14:
                    groups[rid] = blob
                elif rtype == 3:
                    icons[rid] = blob
    if not groups:
        sys.exit("sin RT_GROUP_ICON")
    gid = sorted(groups)[0]
    grp = groups[gid]
    _, _, count = struct.unpack_from('<HHH', grp, 0)
    out = bytearray(struct.pack('<HHH', 0, 1, count))
    img = bytearray()
    offset = 6 + 16 * count
    for i in range(count):
        w, h, cc, res, planes, bpp, nbytes, iid = struct.unpack_from('<BBBBHHIH', grp, 6 + i * 14)
        blob = icons[iid]
        out += struct.pack('<BBBBHHII', w, h, cc, res, planes, bpp, len(blob), offset)
        img += blob
        offset += len(blob)
    open(out_path, 'wb').write(bytes(out + img))
    print(f"{out_path}: icono {count} tamano(s)")

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
