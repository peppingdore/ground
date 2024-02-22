#pragma once

#include "base.h"

#define BUILD_RUN(code)\
constexpr auto CONCAT(__BUILD_RUN_ENTRY__, __COUNTER__) = code;\
constexpr auto CONCAT(__BUILD_RUN_FILE__, __COUNTER__) = __FILE__;\
constexpr auto CONCAT(__BUILD_RUN_LINE__, __COUNTER__) = __LINE__;

BUILD_RUN("params.add_define('_UNICODE')");
BUILD_RUN("params.add_define('UNICODE')");
BUILD_RUN(R"XX(
def get_static_lib_dir(target):
	return builder.MODULE_ROOT / "lib/static" / target.os / target.arch
)XX");
#if OS_WINDOWS
	BUILD_RUN("params.add_lib(get_static_lib_dir(params.get_target()) / 'icuuc.lib')");
	BUILD_RUN("params.add_lib('user32.lib')");
#endif

BUILD_RUN(R"XX(
import argparse
argparser = argparse.ArgumentParser(add_help=False)
argparser.add_argument('--release', action='store_true')
args, _ = argparser.parse_known_args()
if args.release:
	params.add_define(('DEBUG', 0))
	params.add_define('NDEBUG')
else:
	params.add_define('_DEBUG')
	params.add_define(('DEBUG', 1))
)XX");
