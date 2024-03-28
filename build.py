#!/usr/bin/env python3

import os
import sys
import platform
import time
import subprocess
import threading
import argparse
from pathlib import Path
from collections import namedtuple
import traceback
import linecache
import uuid

sys.excepthook = traceback.print_exception

MODULE_ROOT = Path(__file__).parent

class Compiler_Flag:
	def __init__(self, *, clang, msvc):
		self.clang = clang
		self.msvc = msvc
	def __repr__(self):
		return f'{{{self.clang}, {self.msvc}}}'
	def __eq__(self, rhs):
		return isinstance(rhs, Compiler_Flag) and (self.clang, self.msvc) == (rhs.clang, rhs.msvc)

DISABLE_LINKER = Compiler_Flag(clang='-c', msvc='/c')
LATEST_CPP_VERSION = Compiler_Flag(clang='-std=c++2b', msvc='/std:c++latest')
COLOR_OUTPUT=Compiler_Flag(clang=('passthrough_clang_cl:-fcolor-diagnostics', 'passthrough_clang_cl:-fansi-escape-codes'), msvc='')
MISSING_RETURN_IS_ERROR=Compiler_Flag(clang='-Werror=return-type', msvc='')
DEFINED_SIGNED_OVERFLOW=Compiler_Flag(clang=('-fno-strict-overflow', '-fwrapv'), msvc='/d2UndefIntOverflow-')
ASSUME_ALIASING=Compiler_Flag(clang='-fno-strict-aliasing', msvc='')
DEBUG_SYMBOLS=Compiler_Flag(clang='-g', msvc='/Zi')
DISABLE_WARNINGS=Compiler_Flag(clang='-Wno-everything', msvc='')
LIMIT_ERROR_SPAM=Compiler_Flag(clang='-ferror-limit=3', msvc='')
WARNING_IS_ERROR=Compiler_Flag(clang='-Werror', msvc='/WX')
ALLOW_UNUSED_CMDLINE_ARGUMENT=Compiler_Flag(clang='-Wno-unused-command-line-argument', msvc='')
ALLOW_UNSAFE_CRT=Compiler_Flag(clang='-Wno-deprecated-declarations', msvc='')
ALLOW_PRAGMA_ONCE_OUTSIDE_HEADER=Compiler_Flag(clang='-Wno-pragma-once-outside-header', msvc='')
FILENAME_CASE_MISMATCH_WARNING=Compiler_Flag(clang='-Wnonportable-include-path', msvc='')
ALLOW_DEPRECATED=Compiler_Flag(clang='-Wno-deprecated', msvc='')
ALLOW_MICROSOFT_INCLUDE=Compiler_Flag(clang='-Wno-microsoft-include', msvc='')

DEFAULT_COMPILER_FLAGS = [
	DISABLE_LINKER, LATEST_CPP_VERSION, MISSING_RETURN_IS_ERROR,
	DEFINED_SIGNED_OVERFLOW, ASSUME_ALIASING, DEBUG_SYMBOLS, LIMIT_ERROR_SPAM, WARNING_IS_ERROR,
	ALLOW_UNUSED_CMDLINE_ARGUMENT, ALLOW_UNSAFE_CRT, ALLOW_PRAGMA_ONCE_OUTSIDE_HEADER,
	ALLOW_DEPRECATED, FILENAME_CASE_MISMATCH_WARNING, COLOR_OUTPUT, ALLOW_MICROSOFT_INCLUDE ]

OS_WINDOWS = "windows"
OS_LINUX   = "linux"
OS_ANDROID = "android"
OS_MACOS   = "macos"
OS_IOS     = "ios"

ARCH_ARM64 = "arm64"
ARCH_X64   = "x64"

Target = namedtuple("Target", "os arch")

def parse_arch(arch):
	arch = arch.lower()
	if arch in ['x86_64', 'amd64']:  return ARCH_X64
	if arch in ['arm64', 'aarch64']: return ARCH_ARM64
	raise Exception(f'could not parse arch {arch}')

def native_arch():
	return parse_arch(platform.machine())

def native_os():
	if platform.system() == 'Windows':
		return OS_WINDOWS
	if platform.system() == 'Darwin':
		return OS_MACOS
	if platform.system() == 'Linux':
		return OS_LINUX	

def native_target():
	return Target(native_os(), native_arch())

def arch_to_compiler_arch(arch):
	if arch == ARCH_ARM64:
		return "arm64"
	if arch == ARCH_X64:
		return "x86_64"

def make_target_triplet(target):
	if target.os == OS_WINDOWS:
		return f'{arch_to_compiler_arch(target.arch)}-win32-msvc'
	if is_darwin(target.os):
		return f'{arch_to_compiler_arch(target.arch)}-apple-darwin-macho'
	if is_linux(target.os):
		return f'{arch_to_compiler_arch(target.arch)}-linux-gnu'

def build_exec_name(name, *, os=native_os()):
	if os == OS_WINDOWS: return f'{name}.exe'
	if is_darwin(os):    return f'{name}.app'
	return name

def is_darwin(os):
	return os in [OS_MACOS, OS_IOS]

def is_linux(os):
	return os in [OS_LINUX, OS_ANDROID]

def is_posix(os):
	return is_darwin(os) or is_linux(os)

DEFAULT_COMPILER = 'clang-cl' if platform.system() == 'Windows' else 'clang++'

class Compilation_Params:
	def __init__(self, *,
		compiler=DEFAULT_COMPILER,
		target=native_target(),
		defines=[],
		include_dirs=[],
		optimization_level=0,
		compiler_flags=DEFAULT_COMPILER_FLAGS,
	):
		self.compiler = compiler
		self.target = target
		self.defines = defines
		self.include_dirs = include_dirs
		self.optimization_level = optimization_level
		self.compiler_flags = compiler_flags

def is_msvc_interface(compiler):
	return compiler == 'cl' or compiler == 'clang-cl'

def lower_clang_cl_arg(arg):
	PREFIX = "passthrough_clang_cl:"
	if arg.startswith(PREFIX):
		return arg.removeprefix(PREFIX)
	return f'/clang:"{arg}"'

def resolve_compiler_flag(flag, compiler):
	x = flag.msvc if compiler == 'cl' else flag.clang
	x = (x,) if isinstance(x, str) else x
	if compiler == 'clang-cl': x = tuple(lower_clang_cl_arg(it) for it in x)
	x = tuple(it.removeprefix("passthrough_clang_cl:") for it in x)
	return ' '.join(x)

def run(cmd, *, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, shell=True, cwd=None):
	start = time.perf_counter()
	process = subprocess.run(str(cmd), stdout=stdout, stderr=stderr, stdin=stdin, shell=shell, cwd=cwd)
	elapsed = time.perf_counter() - start
	return (process, elapsed)

VERBOSE = 0
def verbose(*args):
	if VERBOSE:
		print(*args)

def build_compile_cmdline(unit, params, out_path):
	args = []
	args.append(params.compiler)
	if VERBOSE:
		if params.compiler == 'cl':
			args.append('/VERBOSE')
		else:
			args.append('/clang:-v' if is_msvc_interface(params.compiler) else '-v')
	if is_msvc_interface(params.compiler):
		args.append(f'/Fo"{str(out_path)}"')
	else:
		args.append(f'--output="{str(out_path)}"')
	for it in params.include_dirs:
		if is_msvc_interface(params.compiler):
			args.append(f'/I"{it}"')
		else:
			args.append(f'--include-directory="{it}"')
	for it in params.defines:
		if isinstance(it, tuple):
			args.append(('/' if is_msvc_interface(params.compiler) else '-') + f'D{it[0]}={it[1]}')
		else:
			args.append(('/' if is_msvc_interface(params.compiler) else '-') + f'D{it}')
	if is_msvc_interface(params.compiler):
		args.append('/TP')
	if is_darwin(params.target.os):
		args.append('-x objective-c++')
	if not is_msvc_interface(params.compiler):
		args.append(f'--target={make_target_triplet(params.target)}')
	if params.optimization_level < 0 or params.optimization_level > 3:
		raise Exception("optimization_level must be in [0, 3] range")
	if is_msvc_interface(params.compiler):
		if params.optimization_level == 0: args.append('/Od')
		if params.optimization_level == 1: args.append('/O2')
		if params.optimization_level == 2: args.append('/O2')
		if params.optimization_level == 3: args.append('/O2ix')
	else:
		args.append(f'-O{params.optimization_level}')
	for it in params.compiler_flags:
		args.append(resolve_compiler_flag(it, params.compiler))
	args.append(unit)
	verbose(args)
	return ' '.join(map(str, args))

class Compile_Result:
	def __init__(self, unit, process, elapsed, out_path):
		self.unit = unit
		self.process = process
		self.elapsed = elapsed
		self.out_path = out_path

def compile_unit(unit, params, out_path=None):
	if not out_path:
		out_path = Path(unit).parent / "build_temp" / f'{Path(unit).stem}.obj' 
	os.makedirs(os.path.dirname(out_path), exist_ok=True)
	cmdline = build_compile_cmdline(unit, params, out_path)
	process, elapsed = run(cmdline)
	return Compile_Result(unit, process, elapsed, out_path)

def compile_units_parallel(units, params):
	threads = []
	results = []
	def proc(unit):
		res = compile_unit(unit, params)
		results.append(res)
	for it in units:
		thread = threading.Thread(target=proc, args=[it])
		thread.start()
		threads.append(thread)
	for it in threads:
		it.join()
	return results

def did_all_units_compile_successfully(results):
	return all(it.process.returncode == 0 for it in results)

# def get_standard_libraries(target):
# 	arr = []
# 	if is_darwin(target.os):
# 		arr.extend(['freetype', 'ssl', 'objc', 'z'])
# 	if is_darwin(target.os) or is_linux(target.os):
# 		arr.extend(['pthread', 'icuuc'])
# 	if target.os == OS_LINUX:
# 		arr.append('clip')
# 	if target.os == OS_WINDOWS:
# 		# Specifying absolute path, because Windows provides it's own icuuc.lib,
# 		#   but a different version.
# 		arr.append(os.path.join(get_standard_lib_dir(target), "icuuc.lib"))
# 	return arr

# def get_standard_lib_dir(target):
# 	return str(compiler_root / "lib/static" / target.os / target.arch)

# def get_standard_lib_directories(target):
# 	dirs = [get_standard_lib_dir(target)]
# 	if is_darwin(target.os):
# 		dirs.extend(['/usr/local/lib', '/usr/local/opt/icu4c/lib'])
# 	return dirs

# def get_standard_apple_frameworks(target):
# 	if not is_darwin(target.os):
# 		return []
# 	arr = [
# 		"Foundation",
# 		"AVFoundation",
# 		"CoreVideo",
# 		"MetalKit",
# 		"Metal",
# 		"Cocoa",
# 		"IOSurface",
# 		"IOKit"
# 	]
# 	if target.os == OS_MACOS:
# 		arr.extend(["AppKit", "Quartz"])
# 	if target.os == OS_IOS:
# 		arr.extend(["UIKit"])
# 	return arr

class Link_Params:
	def __init__(self, *,
		compiler=DEFAULT_COMPILER,
		natvis_files=[],
		libraries=[],
		lib_directories=[],
		apple_frameworks=[],
		use_windows_subsystem=False,
		use_windows_static_crt=False,
		flags = [],
		use_windows_debug_crt=True,
	):
		self.compiler = compiler
		self.natvis_files = natvis_files
		self.libraries = libraries
		self.lib_directories = lib_directories
		self.apple_frameworks = apple_frameworks
		self.use_windows_subsystem = use_windows_subsystem
		self.use_windows_static_crt = use_windows_static_crt
		self.flags = flags
		self.use_windows_debug_crt = use_windows_debug_crt

class Link_Result:
	def __init__(self, process, elapsed, output_path):
		self.process = process
		self.elapsed = elapsed
		self.output_path = output_path

def link(objects, params, output_path):
	os.makedirs(Path(output_path).parent, exist_ok=True)
	args = []
	args.append(params.compiler)
	if VERBOSE:
		args.append('/clang:-v' if is_msvc_interface(params.compiler) else '-v')
	for it in objects:
		x = it
		if isinstance(x, Compile_Result):
			x = x.out_path
		args.append(x)
	prefix = '/clang:' if is_msvc_interface(params.compiler) else ""
	args.append(f'{prefix}--output="{output_path}"')
	for it in params.lib_directories:
		if not is_msvc_interface(params.compiler):
			args.append(f'-L"{it}"')
	for it in params.libraries:
		args.append(f'{prefix}-l{it}')
	for it in params.apple_frameworks:
		args.append(f'-framework {it}')
	args.append(f'{prefix}-g') # do not strip away debug info.
	args.extend(params.flags)
	if is_msvc_interface(params.compiler):
		args.append(f'/M{"T" if params.use_windows_static_crt else "D"}{"d" if params.use_windows_debug_crt else ""}')
		args.append('/link') # Rest of |args| is passed to linker.
		args.append('/INCREMENTAL:NO')
		args.append('/PDBTMCACHE:NO')
		for it in params.lib_directories:
			args.append(f'/LIBPATH:"{it}"')
		if params.use_windows_subsystem:
			args.append('/ENTRY:mainCRTStartup')
			args.append('/SUBSYSTEM:WINDOWS')
		for it in params.natvis_files:
			args.append(f'/NATVIS:"{it}"')
	verbose(args)
	cmdline = ' '.join(str(it) for it in args)
	process, elapsed = run(cmdline)
	return Link_Result(process, elapsed, output_path)

def print_compile_results(stdout, results):
	for it in results:
		if it.process.returncode == 0:
			print(f"{it.unit} compiled in {it.elapsed:.2f}s", file=stdout)
		else:
			print(f"{it.unit} failed", file=stdout)
		stdout.write(it.process.stdout.decode('utf-8', errors='ignore'))

def print_link_result(stdout, result):
	if result.process.returncode != 0:
		print('Linking failed', file=stdout)
		print(result.process.stdout.decode('utf-8'), file=stdout)
	else:
		print(f'{result.output_path} linked in {result.elapsed:.2f}s', file=stdout)

def did_link_successfully(result):
	return result.process.returncode == 0

class Build_Run:
	def __init__(self, code, file, line):
		self.code = code
		self.file = file
		self.line = line
	def __str__(self):
		return f'{self.file}:{self.line} -----\n{self.code}\n-----'

def collect_build_runs(out):
	arr = []
	cursor = 0
	def search(x, do_not_advance=False):
		nonlocal cursor
		found = out.find(x, cursor)
		if found != -1:
			if not do_not_advance: cursor = found + len(x)
			return found
		else:
			return -1
	def skip_space():
		nonlocal cursor
		while cursor < len(out):
			if not out[cursor: cursor + 1].decode('utf-8').isspace():
				break
			cursor += 1
	def find_plain_string_end():
		nonlocal cursor
		while cursor < len(out):
			if out[cursor:].startswith(b'"'):
				if not out[cursor-1:].startswith(b'\\'):
					cursor += 1
					return cursor - 1
			cursor += 1
		return -1
	while True:
		idx = search(b"__BUILD_RUN_ENTRY__")
		if idx == -1: break
		if search(b"=") == -1: break

		# pieces are pieces of concatted C++ string literal.  
		pieces = []
		while True:
			skip_space()
			start = -1
			end = -1
			if out[cursor:].startswith(b'R"'):
				# raw string literal.
				tag_start = cursor + len('R"')
				tag_end = search(b'(')
				start = cursor
				tag = out[tag_start:tag_end]
				end = search(b')' + tag)
				cursor = end + len(b')') + len(tag) + len(b'"')
			elif out[cursor:].startswith(b'"'):
				cursor += 1
				start = cursor
				end = find_plain_string_end()
			else:
				break
			if end == -1:
				break
			p = out[start:end]
			pieces.append(p.decode('unicode_escape'))

		if len(pieces) == 0: continue
		if search(b"__BUILD_RUN_FILE__") == -1: continue
		if search(b"=") == -1: continue
		if search(b'"') == -1: continue
		file_name_start = cursor
		file_name_end = find_plain_string_end()
		if file_name_end == -1: continue
		file_name = out[file_name_start: file_name_end].decode('unicode_escape')

		if search(b"__BUILD_RUN_LINE__") == -1: continue
		if search(b"=") == -1: continue
		skip_space()
		line_num_start = cursor
		line_num_end = search(b';')
		if line_num_end == -1: continue
		line = int(out[line_num_start: line_num_end].decode('unicode_escape'))

		code = ''.join(pieces)
		arr.append(Build_Run(code, file_name, line))
	return arr

def run_preprocessor(compiler, path):
	args = [ compiler ]
	args.append(f'"{path}"')
	if is_msvc_interface(compiler):
		args.append('/E /TP /C')
		if compiler == 'clang-cl':
			args.append('/clang:"-std=c++2b"')
		else:
			args.append('/std:c++latest')
	else:
		if platform.system() == 'Darwin':
			args.append('-x objective-c++')
		args.append('-E')
		args.append('-std=c++2b')
	cmdline = ' '.join(args)
	return run(cmdline)

def run_build_runs(file, scope):
	process, _ = run_preprocessor('clang++', file)
	runs = collect_build_runs(process.stdout)
	for it in runs:
		verbose(f'BUILD_RUN {it}')
		name = f'{it.file}: {it.line}'
		lines = it.code.splitlines(True)
		linecache.cache[name] = len(it.code), None, lines, name
		code = compile(it.code, name, 'exec')
		exec(code, scope)

class Default_Build_Params:
	def __init__(self, *,
		units=[],
		compile_params=Compilation_Params(),
		link_params=Link_Params(),
	):
		self.units = units.copy()
		self.compile_params = compile_params
		self.link_params = link_params

	def set_optimization_level(self, level):
		self.compile_params.optimization_level = level

	def set_compiler(self, compiler):
		self.compile_params.compiler = compiler
		self.link_params.compiler = compiler

	def add_unit(self, unit):
		run_build_runs(unit)
		self.units.append(unit)

	def set_target(self, target):
		self.compile_params.target = target

	def get_target(self):
		return self.compile_params.target

	def add_define(self, define):
		self.compile_params.defines.append(define)

	def add_include_dir(self, dir):
		self.compile_params.include_dirs.append(dir)

	def add_lib(self, lib):
		if lib in self.link_params.libraries: return
		self.link_params.libraries.append(lib)

class Runnable_Executable:
	def __init__(self, path):
		self.path = path
	def __str__(self):
		return str(self.path)

DEFAULT_BUILD_MAIN = """
from pathlib import Path
import sys
import argparse

def build_main():
	params.units.append(file)

	argparser = argparse.ArgumentParser()
	argparser.add_argument('--run', '-r', action='store_true', help="Runs executable after successful compiling")
	argparser.add_argument('--compiler')
	argparser.add_argument('--opt_level', type=int, default=0, help="Optimization level")
	args, _ = argparser.parse_known_args()

	if args.compiler:
		params.set_compiler(args.compiler)
	params.set_optimization_level(args.opt_level)

	compile_results = builder.compile_units_parallel(params.units, params.compile_params)
	builder.print_compile_results(stdout, compile_results)
	if not builder.did_all_units_compile_successfully(compile_results):
		return 1
	output_path = Path(file).parent / "runnable" / builder.build_exec_name(str(Path(file).stem))
	link_result = builder.link(compile_results, params.link_params, output_path)
	builder.print_link_result(stdout, link_result)
	if not builder.did_link_successfully(link_result):
		return 1
	if args.run:
		builder.run(Path(link_result.output_path).resolve(), stdout=sys.stdout, stdin=sys.stdin, cwd=Path(link_result.output_path).parent)
	return builder.Runnable_Executable(link_result.output_path)
"""

def build(file, *, stdout=sys.stdout, scope={}):
	scope.update({
		"builder": __import__(__name__),
		"params": Default_Build_Params(),
		"stdout": stdout,
		"file": file,
	})
	exec(DEFAULT_BUILD_MAIN, scope)
	run_build_runs(file, scope)
	return eval("build_main()", scope)

def main():
	global VERBOSE
	argparser = argparse.ArgumentParser(add_help=False)
	argparser.add_argument('file')
	argparser.add_argument('--verbose', '-v', action='store_true')
	args, _ = argparser.parse_known_args()
	VERBOSE = args.verbose
	res = build(args.file)
	# print(f'Running BUILD_RUN macros took {BUILD_RUN_TIME:.2f} sec')
	if isinstance(res, int):
		return res
	return 0

if __name__ == "__main__":
	exit(main())
