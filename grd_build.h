#pragma once

#include "grd_base.h"

#define GRD_BUILD_RUN(code)\
constexpr static auto GRD_CONCAT(__GRD_BUILD_RUN_START__, __COUNTER__)=0;\
constexpr static auto GRD_CONCAT(__GRD_BUILD_RUN_ENTRY__, __COUNTER__)=code;\
constexpr static auto GRD_CONCAT(__GRD_BUILD_RUN_FILE__, __COUNTER__)=__FILE__;\
constexpr static auto GRD_CONCAT(__GRD_BUILD_RUN_LINE__, __COUNTER__)=__LINE__;\
constexpr static auto GRD_CONCAT(__GRD_BUILD_RUN_END__, __COUNTER__)= 0;

// GRD_BUILD_RUN("params.add_define('_UNICODE')");
// GRD_BUILD_RUN("params.add_define('UNICODE')");
// GRD_BUILD_RUN(R"XX(
// def get_static_lib_dir(target):
// 	return builder.MODULE_ROOT / "lib/static" / target.os / target.arch
// )XX");
// #if OS_WINDOWS
// 	GRD_BUILD_RUN("params.add_lib(get_static_lib_dir(params.get_target()) / 'icuuc.lib')");
// 	GRD_BUILD_RUN("params.add_lib('user32.lib')");
// #endif

// GRD_BUILD_RUN(R"XX(
// import argparse
// argparser = argparse.ArgumentParser(add_help=False)
// argparser.add_argument('--release', action='store_true')
// args, _ = argparser.parse_known_args()
// if args.release:
// 	params.add_define(('DEBUG', 0))
// 	params.add_define('NDEBUG')
// else:
// 	params.add_define('_DEBUG')
// 	params.add_define(('DEBUG', 1))
// )XX");
