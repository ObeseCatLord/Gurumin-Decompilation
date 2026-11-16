"""iti_scan.py <dir> : check the .iti header hypothesis (u32 a, u32 width?, u32 height?, u32 jpeg_size, JPEG...) on every file."""
import collections, glob, os, struct, sys
kinds = collections.Counter()
heads = collections.Counter()
for p in glob.glob(os.path.join(sys.argv[1], "**", "*.iti"), recursive=True):
    d = open(p, "rb").read()
    a, w, h, n = struct.unpack_from("<IIII", d, 0)
    jpeg = d[16:18] == b"\xff\xd8"
    exact = 16 + n == len(d)
    kinds[(jpeg, exact, 16 + n <= len(d))] += 1
    heads[(a, w, h)] += 1
print("(jpeg, size exact, size fits):", dict(kinds))
print("most common (a, w, h):", heads.most_common(10))
