#!/usr/bin/env python3
"""Check that cross-section references in a prc-tools multi-section build
are consistent with where each function actually ended up.

usage: check-sections.py OBJDUMP OBJECT...

prc-tools compiles a reference to a function according to the section
attribute visible at the call site (see src/sections.h):

  * same section:   bsr/lea with a PC-relative DISP16 relocation
  * other section:  movea.l __text__<sec>(a5),a0 (an END16 relocation against
                    __text__ or __text__<sec>) followed by add.l/add.w #sym,a0
                    (a 32 or 16 relocation against the target)

If the declaration in scope names the wrong section, or has no section
attribute at all, the code still links but jumps to the wrong address at run
time. This script reads the relocations of every object and checks each
reference against the section the target symbol is really defined in.

It also flags static data (e.g. tables of function pointers) that points into
a code section other than .text: build-prc silently drops those relocations.

Symbols not defined in any of the given objects come from libgcc / libc,
which are always in .text.
"""
import re
import subprocess
import sys

CODE_SECTIONS = None  # filled from the objects: sections with CODE flag


def run(objdump, *args):
    return subprocess.run([objdump] + list(args), check=True,
                          capture_output=True, text=True).stdout


def sections(objdump, obj):
    """index (1-based, as in COFF symbol table) -> (name, vma, is_code)"""
    out = run(objdump, '-h', obj)
    result = {}
    lines = out.splitlines()
    for i, line in enumerate(lines):
        m = re.match(r'\s*(\d+)\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)', line)
        if m:
            flags = lines[i + 1] if i + 1 < len(lines) else ''
            result[int(m.group(1)) + 1] = (m.group(2), int(m.group(4), 16),
                                           'CODE' in flags)
    return result


SYM = re.compile(r'\[\s*\d+\]\(sec\s+(-?\d+)\)\(fl [^)]*\)\(ty\s+\w+\)'
                 r'\(scl\s+(\d+)\) \(nx \d+\) 0x([0-9a-f]+) (\S+)')


def symbols(objdump, obj, secs):
    """name -> (section name, value, storage class)"""
    result = {}
    for line in run(objdump, '-t', obj).splitlines():
        m = SYM.search(line)
        if not m:
            continue
        sec, scl, value, name = int(m.group(1)), int(m.group(2)), \
            int(m.group(3), 16), m.group(4)
        if sec <= 0 or sec not in secs:
            continue
        result.setdefault(name, (secs[sec][0], value, scl))
    return result


def relocs(objdump, obj):
    """section name -> [(offset, type, symbol)]"""
    result = {}
    current = None
    for line in run(objdump, '-r', obj).splitlines():
        m = re.match(r'RELOCATION RECORDS FOR \[(.+)\]:', line)
        if m:
            current = result.setdefault(m.group(1), [])
            continue
        m = re.match(r'([0-9a-f]{8})\s+(\S+)\s+(\S+)', line)
        if m and current is not None:
            # 'frotzio+0x1c' style targets refer to the section symbol
            sym = re.sub(r'[+-]0x[0-9a-f]+$', '', m.group(3))
            current.append((int(m.group(1), 16), m.group(2), sym))
    return result


def main():
    objdump, objs = sys.argv[1], sys.argv[2:]
    info = {}
    globals_ = {}
    for obj in objs:
        secs = sections(objdump, obj)
        syms = symbols(objdump, obj, secs)
        info[obj] = (secs, syms, relocs(objdump, obj))
        for name, (sec, _, scl) in syms.items():
            if scl == 2:  # external definition
                globals_[name] = sec

    code = {n for secs, _, _ in info.values() for n, _, c in secs.values() if c}

    def where(obj, sym):
        """section a relocation target lives in"""
        secs, syms, _ = info[obj]
        if sym in syms:
            return syms[sym][0]
        if sym in globals_:
            return globals_[sym]
        if any(sym == s[0] for s in secs.values()):
            return sym  # section symbol
        return '.text'  # library

    def function_at(obj, sec, offset):
        secs, syms, _ = info[obj]
        vma = next(v for n, v, _ in secs.values() if n == sec)
        best = None
        for name, (s, value, scl) in syms.items():
            if s == sec and value <= vma + offset and scl in (2, 3) \
                    and not name.startswith('.') and name != sec:
                if best is None or value > best[1]:
                    best = (name, value)
        return best[0] if best else '?'

    errors = []
    for obj, (secs, syms, rels) in info.items():
        for sec, items in rels.items():
            if sec not in code:
                # data: pointers into named code sections are dropped
                for offset, typ, sym in items:
                    target = where(obj, sym)
                    if target in code and target != '.text':
                        errors.append('%s: %s+0x%x: static pointer to %s '
                                      '(in section %s) will not be relocated'
                                      % (obj, sec, offset, sym, target))
                continue
            # section bases loaded with END16 __text__<sec>, by offset and
            # by function (gcc may load a base once and reuse it)
            bases_at = {}
            bases_in = {}
            for offset, typ, sym in items:
                if typ == 'END16' and sym.startswith('__text__'):
                    base = sym[len('__text__'):] or '.text'
                    bases_at[offset] = base
                    bases_in.setdefault(function_at(obj, sec, offset),
                                        set()).add(base)

            for offset, typ, sym in items:
                target = where(obj, sym)
                if target not in code:
                    continue
                func = function_at(obj, sec, offset)
                if typ == 'DISP16' and target != sec:
                    errors.append('%s: %s: PC-relative reference to %s, which '
                                  'is in section %s, not %s'
                                  % (obj, func, sym, target, sec))
                elif typ in ('32', '16'):
                    # movea.l __text__X(a5),a0 ; adda #sym,a0
                    base = bases_at.get(offset - 4)
                    if base is not None:
                        if base != target:
                            errors.append('%s: %s: reference to %s via section '
                                          '%s, but it is in section %s'
                                          % (obj, func, sym, base, target))
                    elif target not in bases_in.get(func, ()):
                        errors.append('%s: %s: reference to %s (section %s) '
                                      'without loading that section\'s base'
                                      % (obj, func, sym, target))

    for e in sorted(set(errors)):
        print(e, file=sys.stderr)
    if errors:
        print('check-sections: %d bad cross-section reference(s); declare '
              'the functions with their SEC_* attribute (see src/sections.h)'
              % len(set(errors)), file=sys.stderr)
        sys.exit(1)
    print('check-sections: %d objects OK' % len(objs))


if __name__ == '__main__':
    main()
