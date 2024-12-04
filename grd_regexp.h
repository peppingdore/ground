#pragma once

#include "grd_build.h"

GRD_BUILD_RUN(R"CMD(
import sys
sys.path.append(__FILE__.parent)
import build_cmake
build_cmake.build_cmake(
	ctx,
	__FILE__.parent / "third_party/build_re2",
	track_paths=[
		__FILE__.parent/"third_party/re2/",
		__FILE__.parent/"third_party/abseil-cpp/",
	])

)CMD")

