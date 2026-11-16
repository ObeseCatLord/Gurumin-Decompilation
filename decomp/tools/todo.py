"""List APP.dll functions not yet matched, mapped, skipped or recorded as walls.

usage: python tools/todo.py [min_size] [max_size] [--fp] [--limit N]
"""
import csv, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def known_addresses():
    done = set()
    for line in open(os.path.join(ROOT, 'obj_symbols.tsv'), encoding='utf-8'):
        parts = line.rstrip('\n').split('\t')
        if len(parts) > 1:
            done.add(parts[1].lower())
    for name in ('open.tsv', os.path.join('tools', 'skip.txt')):
        path = os.path.join(ROOT, name)
        if os.path.exists(path):
            for line in open(path, encoding='utf-8'):
                if line.strip():
                    done.add(line.split()[0].lower())
    pattern = re.compile(r'(?:FUN_|M_|@ |0x)(10[0-9a-fA-F]{6})')
    for folder in ('src', 'src_od'):
        base = os.path.join(ROOT, folder)
        if not os.path.isdir(base):
            continue
        for fn in os.listdir(base):
            if fn.endswith('.cpp'):
                text = open(os.path.join(base, fn), encoding='utf-8', errors='replace').read()
                done.update(a.lower() for a in pattern.findall(text))
    return done


def main():
    args = [a for a in sys.argv[1:]]
    want_fp = '--fp' in args
    limit = 40
    if '--limit' in args:
        limit = int(args[args.index('--limit') + 1])
    nums = [int(a) for a in args if a.isdigit()]
    lo = nums[0] if nums else 30
    hi = nums[1] if len(nums) > 1 else 250
    done = known_addresses()
    rows = []
    with open(os.path.join(ROOT, 'triage.tsv'), encoding='utf-8') as f:
        for r in csv.DictReader(f, delimiter='\t'):
            if r['address'] in done or r['address'] >= '101b2000':
                continue
            if not (lo <= int(r['size']) <= hi):
                continue
            if (r['fp'] != '0') != want_fp:
                continue
            rows.append(r)
    print(f'{len(rows)} left')
    for r in rows[:limit]:
        print(r['address'], r['size'], 'calls', r['calls'], 'loops', r['loops'])


if __name__ == '__main__':
    main()
