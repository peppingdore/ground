#pragma once

#include "grd_build.h"
#include "third_party/re2_wrapper/re2_wrapper.h"

GRD_BUILD_RUN(R"CMD(
import sys
sys.path.append(__FILE__.parent)
import build_subproj
import argparse

def re2_callback(ctx, *, build_path, build_type, did_build, **kwargs):
	path = build_path / 're2' /  build_subproj.get_binary_path_segments(build_type) / f"re2.{'lib' if sys.platform == 'win32' else 'a'}"
	ctx.verbose('re2 path:', path)
	if build_subproj.need_to_build("re2_wrapper") or did_build:
		sub_ctx = ctx.module.BuildCtx(__FILE__.parent / "third_party/re2_wrapper/re2_wrapper.cpp", stdout=ctx.stdout)
		sub_ctx.params.add_lib(path)
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/abseil-cpp")
		sub_ctx.params.add_include_dir(__FILE__.parent / "third_party/re2")
		sub_ctx.params.link_params.output_kind = ctx.module.LinkOutputKind.StaticLibrary
		res = sub_ctx.build()
		if not ctx.is_ok(res):
			raise Exception("Failed to build re2_wrapper")

build_subproj.cmake(
	ctx,
	__FILE__.parent / "third_party/re2_build",
	#track_paths=[
	#	__FILE__.parent/"third_party/re2/",
	#	__FILE__.parent/"third_party/abseil-cpp/",
	#],
	callback=re2_callback,
)

)CMD")

int main() {
	return 0;
}
