#include "../grd_testing.h"
#include "../grd_format.h"

GRD_BUILD_RUN(R"RAW(
ctx.params.link_params.output_kind = ctx.module.LinkOutputKind.DynamicLibrary
)RAW");
