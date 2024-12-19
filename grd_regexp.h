#if 0
	`dirname "$0"`/build.sh $0 $@; exit
#endif
#pragma once

#include "grd_build.h"
#include "third_party/re2_wrapper/re2_wrapper.h"

GRD_BUILD_RUN(R"CMD(
import sys
sys.path.append(__FILE__.parent)
import build_subproj
import argparse

STATIC_LIB_PREF = '' if sys.platform == 'win32' else 'lib'
STATIC_LIB_EXT = '.lib' if sys.platform == 'win32' else '.a'

def get_absl_path(path, sub, build_type):
	return path / 'abseil-cpp' / sub / build_subproj.get_binary_path_segments(build_type)

def absl_link_dir(ctx, path, sub, build_type):
	root = get_absl_path(path, sub, build_type)
	for it in root.glob(f'*{STATIC_LIB_EXT}'):
		ctx.params.add_lib(root / it)
'''
def re2_callback(ctx, *, build_path, build_type, did_build, **kwargs):
	path = build_path / 're2' /  build_subproj.get_binary_path_segments(build_type) / f"{STATIC_LIB_PREF}re2{STATIC_LIB_EXT}"
	ctx.verbose('re2 path:', path)
	if build_subproj.need_to_build("re2_wrapper") or did_build:
		sub_ctx = ctx.module.BuildCtx(__FILE__.parent / "third_party/re2_wrapper/re2_wrapper.cpp", stdout=ctx.stdout)
		sub_ctx.params.add_lib(path)
		absl_link_dir(sub_ctx, build_path, 'absl/log', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/strings', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/container', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/numeric', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/hash', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/synchronization', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/debugging', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/base', build_type)
		absl_link_dir(sub_ctx, build_path, 'absl/time', build_type)
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/abseil-cpp")
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/re2")
		sub_ctx.params.link_params.output_kind = ctx.module.LinkOutputKind.StaticLibrary
		res = sub_ctx.build()
		if not ctx.is_ok(res):
			raise Exception("Failed to build re2_wrapper")
		raise Exception("TODO: remove me")
'''

@ctx.add_pre_compile_hook
def build_re2(_):
	# build_path, build_type, did_build
	re2 = build_subproj.cmake(
		ctx,
		__FILE__.parent / "third_party/re2_build",
		#track_paths=[
		#	__FILE__.parent/"third_party/re2/",
		#	__FILE__.parent/"third_party/abseil-cpp/",
		#],
	)
	re2libpath = re2.build_path / 're2' /  build_subproj.get_binary_path_segments(re2.build_type) / f"{STATIC_LIB_PREF}re2{STATIC_LIB_EXT}"
	ctx.verbose('re2libpath path:', re2libpath)
	if build_subproj.need_to_build("re2_wrapper") or re2.did_build:
		sub_ctx = ctx.module.BuildCtx(__FILE__.parent / "third_party/re2_wrapper/re2_wrapper.cpp", stdout=ctx.stdout)
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/abseil-cpp")
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/re2")
		re2wrapper = sub_ctx.build(do_not_link=True)
	ctx.params.add_lib(ctx.module.get_default_obj_path(__FILE__.parent / "third_party/re2_wrapper/re2_wrapper.cpp"))

)CMD")

GrdTuple<GrdError*, GrdRe2*> re2_make(GrdString pattern) {
	GrdError* e = NULL;
	GrdRe2* re2 = NULL;
	re2_wrapper_make(pattern, &e, &re2);
	return {e, re2};
}

GrdOptional<GrdReMatch> re2_match(GrdRe2* re, GrdString str) {
	GrdReMatch match;
	bool did_match = re2_wrapper_match(re, str, &match);
	if (!did_match) {
		return {};
	}
	return match;
}

int main() {
	auto [e, re2] = re2_make(grd_make_string("(.*)"));
	if (e) {
		grd_println(e);
		return 1;
	}
	auto match = re2_match(re2, grd_make_string("hello world"));
	return 0;
}
