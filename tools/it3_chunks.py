"""it3_chunks.py <dir-or-file>... : walk the tagged chunks of .it3 files and summarise tags (count, sizes, nesting guess)."""
import collections, glob, os, struct, sys

def chunks(d, off=0, end=None):
    end = len(d) if end is None else end
    while off + 8 <= end:
        tag = d[off:off + 4]
        size, = struct.unpack_from("<I", d, off + 4)
        if not all(65 <= c <= 90 or 48 <= c <= 57 or c == 95 for c in tag) or off + 8 + size > end:
            yield ("??", off, end - off)
            return
        yield (tag.decode(), off, size)
        off += 8 + size

stats = collections.defaultdict(list)
bad = 0
files = []
for a in sys.argv[1:]:
    files += glob.glob(os.path.join(a, "**", "*.it3"), recursive=True) if os.path.isdir(a) else [a]
order = collections.Counter()
for p in files:
    d = open(p, "rb").read()
    seq = []
    for tag, off, size in chunks(d):
        stats[tag].append(size)
        seq.append(tag)
        if tag == "??":
            bad += 1
    order[" ".join(seq[:6])] += 1
print(len(files), "files,", bad, "with unparsed tails")
for tag, sizes in sorted(stats.items(), key=lambda kv: -len(kv[1])):
    print("%-6s n=%6d  min=%8d max=%8d" % (tag, len(sizes), min(sizes), max(sizes)))
print("common chunk orders:")
for s, n in order.most_common(8):
    print(" %5d  %s" % (n, s))
