"""Assemble one field script the way tools/scripts/make_script_bin.sh does, keeping the object file so
label offsets can be read. Used by port_fp_scripts.py.

assemble(source_text, workdir) -> (binary bytes, {label: offset})
"""
import os
import re
import subprocess

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
BUILD = os.path.join(REPO, 'build')
UCRT = r'C:\msys64\ucrt64\bin'
MSYS = r'C:\msys64\usr\bin'
GCC = os.path.join(UCRT, 'arm-none-eabi-gcc.exe')
OBJCOPY = os.path.join(UCRT, 'arm-none-eabi-objcopy.exe')
NM = os.path.join(UCRT, 'arm-none-eabi-nm.exe')
ENUMPROC = os.path.join(BUILD, 'tools', 'enumproc', 'enumproc.exe')
ENV = dict(os.environ, PATH=UCRT + os.pathsep + MSYS + os.pathsep + os.environ.get('PATH', ''))


class AsmError(Exception):
    pass


def assemble(source, workdir, name='script', extra_includes=()):
    os.makedirs(workdir, exist_ok=True)
    src = os.path.join(workdir, name + '.s')
    obj = os.path.join(workdir, name + '.o')
    binp = os.path.join(workdir, name + '.bin')
    open(src, 'w', encoding='utf-8', newline='\n').write(source)
    inc = ['-I' + d for d in extra_includes]
    inc += ['-I' + os.path.join(REPO, 'include'), '-I' + os.path.join(REPO, 'asm'), '-I' + BUILD]
    pre = subprocess.run([GCC, '-E', '-x', 'assembler-with-cpp'] + inc + [src], capture_output=True, env=ENV, cwd=BUILD)
    if pre.returncode:
        raise AsmError(pre.stderr.decode(errors='replace')[-3000:])
    enum = subprocess.run([ENUMPROC], input=pre.stdout, capture_output=True, env=ENV, cwd=BUILD)
    if enum.returncode:
        raise AsmError('enumproc: ' + enum.stderr.decode(errors='replace')[-2000:])
    asm = subprocess.run([GCC, '-x', 'assembler-with-cpp', '-o', obj, '-c', '-'], input=enum.stdout,
                         capture_output=True, env=ENV, cwd=BUILD)
    if asm.returncode:
        raise AsmError(asm.stderr.decode(errors='replace')[-3000:])
    r = subprocess.run([OBJCOPY, '-O', 'binary', '--file-alignment', '4', obj, binp], capture_output=True, env=ENV)
    if r.returncode:
        raise AsmError(r.stderr.decode(errors='replace'))
    nm = subprocess.run([NM, obj], capture_output=True, env=ENV, text=True)
    labels = {}
    for line in nm.stdout.splitlines():
        m = re.match(r'([0-9a-fA-F]+) [tTdDbB] (\S+)', line)
        if m:
            labels[m.group(2)] = int(m.group(1), 16)
    return open(binp, 'rb').read(), labels
