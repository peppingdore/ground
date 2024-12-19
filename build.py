#!/usr/bin/env python3

import os
import sys
import platform
import time
import subprocess
import argparse
from pathlib import Path
from collections import namedtuple
import linecache
from multiprocessing.pool import ThreadPool
from enum import Enum

MODULE_ROOT = Path(__file__).parent

class CompilerFlag:
	def __init__(self, *, clang, msvc):
		self.clang = clang
		self.msvc = msvc
	def __repr__(self):
		return f'{{{self.clang}, {self.msvc}}}'
	def __eq__(self, rhs):
		return isinstance(rhs, CompilerFlag) and (self.clang, self.msvc) == (rhs.clang, rhs.msvc)

DISABLE_LINKER = CompilerFlag(clang='-c', msvc='/c')
LATEST_CPP_VERSION = CompilerFlag(clang='-std=c++2b', msvc='/std:c++latest')
COLOR_OUTPUT=CompilerFlag(clang=('passthrough_clang_cl:-fcolor-diagnostics', 'passthrough_clang_cl:-fansi-escape-codes'), msvc='')
MISSING_RETURN_IS_ERROR=CompilerFlag(clang='-Werror=return-type', msvc='')
DEFINED_SIGNED_OVERFLOW=CompilerFlag(clang=('-fno-strict-overflow', '-fwrapv'), msvc='/d2UndefIntOverflow-')
ASSUME_ALIASING=CompilerFlag(clang='-fno-strict-aliasing', msvc='')
DEBUG_SYMBOLS=CompilerFlag(clang='-g', msvc='/Zi')
DISABLE_WARNINGS=CompilerFlag(clang='-Wno-everything', msvc='')
LIMIT_ERROR_SPAM=CompilerFlag(clang='-ferror-limit=3', msvc='')
WARNING_IS_ERROR=CompilerFlag(clang='-Werror', msvc='/WX')
ALLOW_UNUSED_CMDLINE_ARGUMENT=CompilerFlag(clang='-Wno-unused-command-line-argument', msvc='')
ALLOW_UNSAFE_CRT=CompilerFlag(clang='-Wno-deprecated-declarations', msvc='')
ALLOW_PRAGMA_ONCE_OUTSIDE_HEADER=CompilerFlag(clang='-Wno-pragma-once-outside-header', msvc='')
ALLOW_FILENAME_CASE_MISMATCH=CompilerFlag(clang='-Wnonportable-include-path', msvc='')
ALLOW_DEPRECATED=CompilerFlag(clang='-Wno-deprecated', msvc='')
ALLOW_MICROSOFT_INCLUDE=CompilerFlag(clang='-Wno-microsoft-include', msvc='')
ALLOW_UNDEFINED_INLINE=CompilerFlag(clang='-Wno-undefined-inline', msvc='')
ENABLE_WIDE_CMPXCHG=CompilerFlag(clang="-mcx16", msvc="")
COMPILE_TIME_TRACE=CompilerFlag(clang=("-ftime-trace"), msvc="")
COMPILE_TIME_TRACE_HIGH_GRANULARITY=CompilerFlag(clang=("-ftime-trace-granularity=0"), msvc="")

DEFAULT_COMPILER_FLAGS = [
	DISABLE_LINKER, LATEST_CPP_VERSION, MISSING_RETURN_IS_ERROR,
	DEFINED_SIGNED_OVERFLOW, ASSUME_ALIASING, DEBUG_SYMBOLS, LIMIT_ERROR_SPAM, WARNING_IS_ERROR,
	ALLOW_UNUSED_CMDLINE_ARGUMENT, ALLOW_UNSAFE_CRT, ALLOW_PRAGMA_ONCE_OUTSIDE_HEADER,
	ALLOW_DEPRECATED, ALLOW_FILENAME_CASE_MISMATCH, COLOR_OUTPUT, ALLOW_MICROSOFT_INCLUDE,
	ALLOW_UNDEFINED_INLINE, ENABLE_WIDE_CMPXCHG ]

OS_WINDOWS = "windows"
OS_LINUX   = "linux"
OS_ANDROID = "android"
OS_MACOS   = "macos"
OS_IOS     = "ios"

ARCH_ARM64 = "arm64"
ARCH_X64   = "x64"

Target = namedtuple("Target", "os arch")

def is_os_apple(os):
	return os in [OS_MACOS, OS_IOS]

def is_os_linux(os):
	return os in [OS_LINUX, OS_ANDROID]

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
		return "aarch64"
	if arch == ARCH_X64:
		return "x86_64"
	raise Exception("Unknown arch")

def make_target_triplet(target):
	if target.os == OS_WINDOWS:
		return f'{arch_to_compiler_arch(target.arch)}-win32-msvc'
	if is_darwin(target.os):
		return f'{arch_to_compiler_arch(target.arch)}-apple-darwin-macho'
	if is_linux(target.os):
		return f'{arch_to_compiler_arch(target.arch)}-linux-gnu'

def is_darwin(os):
	return os in [OS_MACOS, OS_IOS]

def is_linux(os):
	return os in [OS_LINUX, OS_ANDROID]

def is_posix(os):
	return is_darwin(os) or is_linux(os)

DEFAULT_COMPILER = 'clang-cl' if platform.system() == 'Windows' else 'clang++'

class CompilationParams:
	def __init__(self, *,
		compiler=None,
		target=None,
		defines=None,
		include_dirs=None,
		optimization_level=0,
		compiler_flags=None,
		use_windows_static_crt=False,
		use_windows_debug_crt=True,
	):
		self.compiler = compiler or DEFAULT_COMPILER
		self.target = target or native_target()
		self.defines = defines or []
		self.include_dirs = include_dirs or []
		self.optimization_level = optimization_level
		self.compiler_flags = compiler_flags or DEFAULT_COMPILER_FLAGS
		self.use_windows_static_crt = use_windows_static_crt
		self.use_windows_debug_crt = use_windows_debug_crt

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
	process = subprocess.run(cmd, stdout=stdout, stderr=stderr, stdin=stdin, shell=shell, cwd=cwd)
	elapsed = time.perf_counter() - start
	return (process, elapsed)

VERBOSE = 0
def verbose(*args):
	if VERBOSE:
		print(*args)

def build_compile_cmdline(unit, params, target, out_path):
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
	if not is_msvc_interface(params.compiler):
		# If we don't specify -x c++/objective-c++ clang will compile *.h files into a precompiled header.
		args.append('-x objective-c++' if is_darwin(target.os) else '-x c++')
	if target.os == OS_LINUX:
		args.append('-stdlib=libstdc++')
	if not is_msvc_interface(params.compiler):
		args.append(f'--target={make_target_triplet(target)}')
	if params.optimization_level < 0 or params.optimization_level > 3:
		raise Exception("optimization_level must be in [0, 3] range")
	if is_msvc_interface(params.compiler):
		if params.optimization_level == 0: args.append('/Od')
		if params.optimization_level == 1: args.append('/O2')
		if params.optimization_level == 2: args.append('/O2')
		if params.optimization_level == 3: args.append('/O2ix')
	else:
		args.append(f'-O{params.optimization_level}')
	if params.compiler != 'cl':
		args.append(("/clang:" if is_msvc_interface(params.compiler) else "") + "-fvisibility=internal")
	for it in params.compiler_flags:
		args.append(resolve_compiler_flag(it, params.compiler))
	if is_msvc_interface(params.compiler):
		args.append(f'/M{"T" if params.use_windows_static_crt else "D"}{"d" if params.use_windows_debug_crt else ""}')
	args.append(unit)
	verbose(args)
	return ' '.join(map(str, args))

class CompileResult:
	def __init__(self, unit, process, elapsed, target, out_path):
		self.unit = unit
		self.process = process
		self.elapsed = elapsed
		self.target = target
		self.out_path = out_path

def get_default_obj_path(src):
	return Path(src).parent / "built/inter" / f'{Path(src).stem}.obj' 

def compile_unit(unit, params, target, out_path=None):
	if not out_path: out_path = get_default_obj_path(unit)
	os.makedirs(os.path.dirname(out_path), exist_ok=True)
	cmdline = build_compile_cmdline(unit, params, target, out_path)
	process, elapsed = run(cmdline)
	return CompileResult(unit, process, elapsed, target, out_path)

def compile_units_parallel(units, params, target):
	def proc(unit):
		res = compile_unit(unit, params, target)
		return res
	pool = ThreadPool()
	res = pool.map_async(proc, units)
	while True:
		if res.ready(): break
		time.sleep(0.1)
	results = res.get()
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

class LinkOutputKind(Enum):
	Executable = 1,
	StaticLibrary = 2,
	DynamicLibrary = 3,

class LinkParams:
	def __init__(self, *,
		compiler=None,
		natvis_files=None,
		output_kind=LinkOutputKind.Executable,
		libraries=None,
		static_libraries=None,
		lib_directories=None,
		apple_frameworks=None,
		use_windows_subsystem=False,
		flags = None,
	):
		self.compiler = compiler or DEFAULT_COMPILER
		self.natvis_files = natvis_files or []
		self.output_kind = output_kind
		self.libraries = libraries or []
		self.static_libraries = static_libraries or []
		self.lib_directories = lib_directories or []
		self.apple_frameworks = apple_frameworks or []
		self.use_windows_subsystem = use_windows_subsystem
		self.flags = flags or []
	
class LinkResult:
	def __init__(self, process, elapsed, output_path, params):
		self.process = process
		self.elapsed = elapsed
		self.output_path = output_path
		self.params = params

# supply os properly to these 2 functions.
def get_binary_prefix(kind, os=native_os()):
	if os == OS_WINDOWS: return ''
	if kind == LinkOutputKind.Executable: return ''
	if kind == LinkOutputKind.StaticLibrary: return 'lib'
	if kind == LinkOutputKind.DynamicLibrary: return ''
	raise Exception(f'Unknown output kind {kind}')

def get_binary_ext(kind, os=native_os()):
	if kind == LinkOutputKind.Executable and os == OS_WINDOWS: return '.exe'
	if kind == LinkOutputKind.Executable: return ''
	if kind == LinkOutputKind.StaticLibrary and os == OS_WINDOWS: return '.lib'
	if kind == LinkOutputKind.StaticLibrary: return '.a'
	if kind == LinkOutputKind.DynamicLibrary and os == OS_WINDOWS: return '.dll'
	if kind == LinkOutputKind.DynamicLibrary and is_os_apple(os): return '.dylib'
	if kind == LinkOutputKind.DynamicLibrary and is_os_linux(os): return '.so'
	raise Exception(f'Unknown output kind {kind}')

def link(objects, params, target, output_path):
	os.makedirs(Path(output_path).parent, exist_ok=True)
	args = []
	args.append(params.compiler)
	for it in objects:
		x = it
		if isinstance(x, CompileResult):
			x = x.out_path
		args.append(x)
	for it in params.static_libraries:
		args.append(it)
	prefix = '/clang:' if is_msvc_interface(params.compiler) else ""
	if VERBOSE:
		args.append(f'{prefix}-v')
	if params.compiler != 'cl':
		args.append(f'{prefix}--target={make_target_triplet(target)}')
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
	if params.output_kind == LinkOutputKind.DynamicLibrary:
		args.append(f'{prefix}-shared')
	elif params.output_kind == LinkOutputKind.StaticLibrary:
		args.append(f'{prefix}-static')
		if is_msvc_interface(params.compiler):
			args.append(f'{prefix}-fuse-ld=llvm-lib')
	elif params.output_kind == LinkOutputKind.Executable:
		pass
	else:
		raise Exception(f'Unknown output kind {params.output_kind}')
	if is_msvc_interface(params.compiler):
		# args.append(f'/M{"T" if params.use_windows_static_crt else "D"}{"d" if params.use_windows_debug_crt else ""}')
		if params.output_kind != LinkOutputKind.StaticLibrary:
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
	if not is_msvc_interface(params.compiler):
		args.append('-fuse-ld=lld')
	verbose(args)
	cmdline = ' '.join(str(it) for it in args)
	process, elapsed = run(cmdline)
	return LinkResult(process, elapsed, output_path, params)

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

class BuildRun:
	def __init__(self, code, file, line):
		self.code = code
		self.file = file
		self.line = line
	def __str__(self):
		return f'{self.file}:{self.line} -----\n{self.code}\n-----'

# @TODO: rename build runs to something else.
#   maybe build exec?
def collect_build_runs(out):
	res = []
	import re
	matches = re.findall(r"GRD_BUILD_RUN_ENTRY.*?=(.*?);.*?GRD_BUILD_RUN_FILE.*?=(.*?);.*?GRD_BUILD_RUN_LINE.*?=(.*?);", out, flags=re.MULTILINE|re.S)
	for it in matches:
		if len(it) != 3:
			raise Exception(f'build run expects 3 items, got {len(it)}, {it}')
		code, file, line = it
		code = code.strip()
		if code.startswith('R"'):
			match = re.search(r'^R"(.*?)\((.*)\)(.*?)"$', code, flags=re.S).groups()
			if len(match) != 3:
				raise Exception(f'build run raw literal regex expects 3 items, got {len(match)}, {match}')
			if match[0] != match[2]:
				raise Exception(f'build run raw literal prefix and suffix must be the same, got {match[0]} and {match[2]}')
			code = match[1]
		else:
			code = code.removeprefix('"').removesuffix('"')
		code = code.encode('utf-8').decode('unicode_escape')
		file = file.removeprefix('"').removesuffix('"')
		res.append(BuildRun(code, file, int(line)))
	return res

def run_preprocessor(compiler, path):
	args = [ compiler ]
	args.append(f'"{path}"')
	if is_msvc_interface(compiler):
		args.append('/E /TP /C')
		if compiler == 'clang-cl':
			args.append('/clang:"-std=c++23"')
		else:
			args.append('/std:c++latest')
	else:
		if platform.system() == 'Darwin':
			args.append('-x objective-c++')
		args.append('-E')
		args.append('-std=c++23')
	cmdline = ' '.join(args)
	return run(cmdline)

class DefaultBuildParams:
	def __init__(self, ctx, *,
		units=None,
		compile_params=None,
		link_params=None,
		target=None,
	):
		self.units = units or []
		self.compile_params = compile_params or CompilationParams()
		self.link_params = link_params or LinkParams()
		self.target = target or native_target()
		self.ctx = ctx

	def set_target(self, target):
		self.target = target

	def set_optimization_level(self, level):
		self.compile_params.optimization_level = level

	def set_compiler(self, compiler):
		self.compile_params.compiler = compiler
		self.link_params.compiler = compiler

	def add_unit(self, unit):
		self.ctx.run_build_runs(unit)
		self.units.append(unit)

	def add_define(self, define):
		self.compile_params.defines.append(define)

	def add_include_dir(self, dir):
		self.compile_params.include_dirs.append(dir)

	def add_lib(self, lib):
		if str(lib).endswith('.a') or str(lib).endswith('.lib'):
			lib = Path(lib).resolve()
			if not lib in self.link_params.static_libraries:
				self.link_params.static_libraries.append(lib)
			return
		if lib in self.link_params.libraries: return
		self.link_params.libraries.append(lib)

	def add_natvis_file(self, file):
		self.link_params.natvis_files.append(file)

class OutputBinary:
	def __init__(self, path):
		self.path = path
	def __str__(self):
		return str(self.path)
	
class BuildCtx:
	def __init__(self, file, stdout=None):
		self.module = __import__(__name__)
		self.stdout = stdout or sys.stdout
		self.file_stack = [Path(file).resolve()]
		self.VERBOSE = False
		self.params = DefaultBuildParams(self)
		self.pre_compile_hooks = []
		self.pre_link_hooks = []
		self.run_build_runs(file)

	# You may want to use this hooks, to postpone some actions,
	#   that may, for example, depend on args parsed at the start of ctx.build()
	def add_pre_compile_hook(self, hook):
		self.pre_compile_hooks.append(hook)
	def add_pre_link_hook(self, hook):
		self.pre_link_hooks.append(hook)
	def run_pre_compile_hooks(self):
		for it in self.pre_compile_hooks: it(self)
	def run_pre_link_hooks(self):
		for it in self.pre_link_hooks: it(self)

	def verbose(self, *args):
		if self.VERBOSE:
			print(*args)

	@property
	def root(self):
		return self.file_stack[0]

	@property
	def file(self):
		return self.file_stack[-1]
	
	def set_build_main(self, build_main):
		if hasattr(self, 'build_main'): raise Exception("BuildCtx: set_build_main() was already called")
		# setattr(self, 'build_main', build_main)
		self.build_main = build_main

	def create_exec_scope(self):
		return { 'ctx': self, '__FILE__': self.file_stack[-1] }

	def run_build_runs(self, file):
		process, _ = run_preprocessor('clang++', file)
		runs = collect_build_runs(process.stdout.decode('utf-8', errors='ignore'))
		for it in runs:
			verbose(f'BUILD_RUN {it}')
			# Inform Python about source code location. 
			name = f'{it.file}: {it.line}'
			lines = it.code.splitlines(True)
			linecache.cache[name] = len(it.code), None, lines, name
			code = compile(it.code, name, 'exec')
			self.file_stack.append(Path(it.file).resolve())
			exec(code, self.create_exec_scope())
			self.file_stack.pop()

	def build(self):
		scope = self.create_exec_scope()
		if not hasattr(self, 'build_main'): self.set_build_main(default_build_main)
		return eval("ctx.build_main(ctx)", scope)
	
	def is_ok(self, build_res):
		if isinstance(build_res, OutputBinary):
			return True
		if build_res == 0:
			return True
		return False
	
def get_binary_path(file, output_kind):
	return file.parent / "built" / f'{str(Path(file).stem)}{get_binary_ext(output_kind)}'

def default_build_main(self):
	self.params.units.append(self.root)
	argparser = argparse.ArgumentParser()
	argparser.add_argument('--run', '-r', action='store_true', help="Runs executable after successful compiling")
	argparser.add_argument('--compiler')
	argparser.add_argument('--opt_level', type=int, default=0, help="Optimization level")
	argparser.add_argument('--arch')
	argparser.add_argument('extra', nargs='*')
	argparser.add_argument('--time_trace', action='store_true', help="Enables compile time tracing")
	argparser.add_argument('--time_trace_high_granularity', action='store_true', help="Enables compile time tracing with high granularity")
	args, extra = argparser.parse_known_args()

	target = native_target()

	if args.compiler:
		self.params.set_compiler(args.compiler)
	if args.arch:
		target = Target(target.os, args.arch)
	if args.time_trace or args.time_trace_high_granularity:
		self.params.compile_params.compiler_flags.append(COMPILE_TIME_TRACE)
	if args.time_trace_high_granularity:
		self.params.compile_params.compiler_flags.append(COMPILE_TIME_TRACE_HIGH_GRANULARITY)

	self.params.set_target(target)
	self.params.set_optimization_level(args.opt_level)
	self.params.add_natvis_file(MODULE_ROOT / "grd.natvis")
	
	self.run_pre_compile_hooks()
	self.params.add_define(('GRD_BUILD_COMPILING', 1))

	compile_results = compile_units_parallel(self.params.units, self.params.compile_params, self.params.target)
	print_compile_results(self.stdout, compile_results)
	if not did_all_units_compile_successfully(compile_results):
		return 1
	output_path = get_binary_path(self.root, self.params.link_params.output_kind)
	
	self.run_pre_link_hooks()

	link_result = link(compile_results, self.params.link_params, self.params.target, output_path)
	print_link_result(self.stdout, link_result)
	if not did_link_successfully(link_result):
		return 1
	if args.run:
		if len(extra) > 0: extra = extra[1:] # Skip double dash (--)
		run([str(Path(link_result.output_path).resolve()), *extra], stdout=sys.stdout, stdin=sys.stdin, cwd=Path(link_result.output_path).parent, shell=False)
	return OutputBinary(link_result.output_path)

def build_file(file, *, stdout=None):
	ctx = BuildCtx(file, stdout=stdout or sys.stdout)
	ctx.VERBOSE = VERBOSE
	return ctx.build()

def main():
	global VERBOSE
	argparser = argparse.ArgumentParser(add_help=False)
	argparser.add_argument('file')
	argparser.add_argument('--verbose', '-v', action='store_true')
	args, _ = argparser.parse_known_args()
	VERBOSE = args.verbose
	res = build_file(args.file)
	return res if isinstance(res, int) else 0

if __name__ == "__main__":
	exit(main())
