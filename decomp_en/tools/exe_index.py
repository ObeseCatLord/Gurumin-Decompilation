"""exe_index.py : shared analysis of a Ghidra export (functions, call graph, referenced strings, constants, imports).

Used by classify.py and map_jp_to_en.py. Works for the EN exe and for the JP DLLs (APP.dll, Packs.dll, ...):
`Program(name)` reads re/bin/<name> and re/export/<name>/.
"""
import bisect, csv, os, re, struct
import capstone
from coff import PeImage
from enimg import ROOT

csv.field_size_limit(1 << 30)
_md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_md.detail = False


def _cstring(img, va, limit=200):
    raw = img.read(va, limit)
    end = raw.find(b"\0")
    if end < 4:
        return None
    s = raw[:end]
    # printable ASCII or Shift-JIS text; reject binary
    try:
        text = s.decode("cp932")
    except UnicodeDecodeError:
        return None
    if any(ord(c) < 0x20 and c not in "\t\r\n" for c in text):
        return None
    if sum(c.isalnum() for c in text) < 2:
        return None
    return text


class Program:
    def __init__(self, name):
        self.name = name
        self.img = PeImage(os.path.join(ROOT, "re", "bin", name))
        export = os.path.join(ROOT, "re", "export", name)
        self.funcs = {}       # address -> (name, size)
        self.callees = {}     # address -> [callee addresses] (from Ghidra)
        self.callers = {}
        self.ext_calls = {}   # address -> names of imported functions it calls
        for r in csv.reader(open(os.path.join(export, "functions.tsv"), encoding="utf-8", errors="replace"),
                            delimiter="\t"):
            if r[0] == "entry":
                continue
            a = int(r[0], 16)
            self.funcs[a] = (r[1], int(r[2]))
            self.callers[a] = self._addresses(r[3]) if len(r) > 3 else []
            self.callees[a] = self._addresses(r[4]) if len(r) > 4 else []
            self.ext_calls[a] = [x.rsplit("@", 1)[0] for x in r[4].split(",") if "@EXTERNAL" in x] if len(r) > 4 else []
        self.starts = sorted(self.funcs)
        self.imports = {}     # thunk/IAT address -> "dll!name"
        imp = os.path.join(export, "imports.tsv")
        if os.path.exists(imp):
            for r in csv.reader(open(imp, encoding="utf-8", errors="replace"), delimiter="\t"):
                if len(r) >= 2:
                    self.imports.setdefault(r[1], r[0])
        self.refs = {}        # function address -> list of referenced data addresses
        for r in csv.reader(open(os.path.join(export, "data_refs.tsv"), encoding="utf-8", errors="replace"),
                            delimiter="\t"):
            if r[0] == "from":
                continue
            f = self.containing(int(r[0], 16))
            if f is not None:
                self.refs.setdefault(f, []).append(int(r[2], 16))
        self._strings = {}

    @staticmethod
    def _addresses(field):
        out = []
        for x in field.split(","):
            if "@" in x and "EXTERNAL" not in x:
                try:
                    out.append(int(x.rsplit("@", 1)[1], 16))
                except ValueError:
                    pass
        return out

    def containing(self, va):
        i = bisect.bisect_right(self.starts, va) - 1
        if i < 0:
            return None
        a = self.starts[i]
        return a if va < a + max(1, self.funcs[a][1]) else None

    def strings(self, f):
        """C strings a function references (in reference order, duplicates removed)"""
        if f not in self._strings:
            out = []
            for t in self.refs.get(f, []):
                s = _cstring(self.img, t)
                if s and s not in out:
                    out.append(s)
            self._strings[f] = out
        return self._strings[f]

    def code(self, f):
        return self.img.read(f, self.funcs[f][1])

    def features(self, f):
        """instruction-level features: imported API calls in order, immediate constants, float constants read"""
        apis, consts = [], []
        for ins in _md.disasm(self.code(f), f):
            ops = ins.op_str
            if ins.mnemonic == "call" and "dword ptr [" in ops:
                m = re.search(r"\[0x([0-9a-f]+)\]", ops)
                if m:
                    apis.append(int(m.group(1), 16))
            for m in re.finditer(r"\b0x([0-9a-f]{3,8})\b", ops):
                v = int(m.group(1), 16)
                if not (0x400000 <= v < 0x3000000 or 0x10000000 <= v < 0x12000000):
                    consts.append(v)
            for m in re.finditer(r"\b([1-9][0-9]{2,})\b", ops):
                consts.append(int(m.group(1)))
        return apis, consts

    def float_constants(self, f):
        """float/double values the function loads from read-only data"""
        out = []
        for t in self.refs.get(f, []):
            sec = self.img.section_of(t)
            if sec not in (".rdata", ".rdata1", ".data"):
                continue
            raw = self.img.read(t, 8)
            if len(raw) < 4:
                continue
            v = struct.unpack_from("<f", raw)[0]
            if v == v and v not in (0.0,) and 1e-6 < abs(v) < 1e7 and abs(v - round(v, 4)) < 1e-6:
                out.append(round(v, 4))
        return out
