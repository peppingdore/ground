#include "../error.h"
#include "../log.h"
#include "../grd_defer.h"
#include "c_like_parser.h"
#include "ast_printer.h" 
#include "ssa.h"
#include "../file.h"

// https://twitter.com/zozuar/status/1755381710227755046
//
// float i,e,R,s,z;vec3 m,q,p,d=FC.rgb/r.y-.5;q=hsv(t/6./PI,2.,.4);for(q.z--;i++<1e2;q-=d*e*R*.4,o+=log(++R+sin(vec4(1,2,3,0)+z*z/2e4))/2e2,p=vec3(log(R=length(q))-t/2.,e=-q.z/R,atan(q.x,q.y)),z=s,i>60.?d/=d,e+=1e-4:e)for(s=2.;s<1e3;s/=-.5)e-=abs(cos(dot(m=cos(p*s),q/q)))/s,z-=m.y;

// float i, e, R, s, z;
// vec3 m, q, p, d = FC.rgb / r.y - .5;
// q = hsv(t / 6. / PI, 2., .4);
// for (q.z--; i++ < 1e2; q -= d * e * R * .4, o += log(++R + sin(vec4(1, 2, 3, 0) + z * z / 2e4)) / 2e2, p = vec3(log(R = length(q)) - t / 2., e = -q.z / R, atan(q.x, q.y)), z = s, i > 60. ? d /= d, e += 1e-4 : e)
//     for (s = 2.; s < 1e3; s /=- .5)
//         e -= abs(cos(dot(m = cos(p * s), q / q))) / s, z -= m.y;

#define PROGRAM_ID 0

UnicodeString PROGRAM = 
#if PROGRAM_ID == 0
UR"GRD_TAG(
float PI = 3.14f;

struct VertexOutput {
	float4 position [[position]];
};

[[fragment]] float4 main(
	float* t [[mtl_buffer(0)]] [[vk_uniform(0)]],
	float2* r [[mtl_constant(1)]] [[vk_uniform(1)]],
	float4* target_size [[mtl_constant(2)]] [[vk_uniform(0, 2)]],
	VertexOutput in [[stage_in]]
) {
	r = r;
	VertexOutput xx = in;
	float4 FC = in.position / *target_size;
	float i,e,R,s,z;
	float3 m,q,p,d=FC.rgb/r.y-.5;
	q=hsv(*t/6.0f/PI,2.0f,.4f);
	float4 o = float4(0f, 0f, 0f, 0f);
	for(q.z--;i++<1e2f;q-=d*e*R*.4,o+=log(++R+sin(float4(1f,2f,3f,0f)+z*z/2e4f))/2e2f,p=float3(log(R=length(q))-*t/2.f,e=-q.z/R,atan(q.x,q.y)),z=s,i>60.f?d/=d,e+=1e-4f:e)
		for(s=2.f;s<1e3f;s/=-.5f)
			e-=abs(cos(dot_vec3(m=cos(p*s),q/q)))/s,z-=m.y;
	return o;
}
)GRD_TAG"_b;
#elif PROGRAM_ID == 1
UR"GRD_TAG(
struct VertexOutput {
	float4 position [[position]];
};

[[fragment]] float4 main(
	float* t [[mtl_constant(0)]] [[vk_uniform(0)]],
	float2**** r [[mtl_constant(1)]] [[vk_uniform(1)]],
	float4* target_size [[mtl_constant(2)]] [[vk_uniform(0, 2)]],
	VertexOutput in [[stage_in]]
) {
	float i = r[0][0][0][0].y;
	r[0][0][0] = r[0][0][0];
}
)GRD_TAG"_b;
#elif PROGRAM_ID == 2
UR"GRD_TAG(
[[fragment]] float4 main(float4* buf) {
	float4 pen;
	pen.xyzw.ywzx.x = buf.yzwx.xxxx.x;
}
)GRD_TAG"_b;
#endif


int main() {
	auto [program, e] = parse_c_like(PROGRAM);
	if (e) {
		print_parser_error(e);
		return -1;
	}
	// println(print_ast_node(program));

	for (auto it: program->globals) {
		if (auto f = grd_reflect_cast<AstFunction>(it)) {
			if (!f->block) {
				continue;
			}

			auto [ssa, e] = emit_function_ssa(c_allocator, f);
			if (e) {
				print_parser_error(e);
				return -1;
			}

			auto [file, e1] = open_file(U"xxx.spv"_b, FILE_WRITE | FILE_CREATE_NEW);
			if (e1) {
				print_parser_error(e);1);
				return -1;
			}
			auto m = grd_make_spirv_emitter(f->p, c_allocator);			
			e = emit_spirv_function(&m, ssa);
			if (e) {
				print_parser_error(e);
				return -1;
			}
			print_ssa(ssa->entry);

			e = finalize_spirv(&m);
			if (e) {
				print_parser_error(e);
				return -1;
			}

			e = write_file(&file, m.spv.data, len(m.spv) * sizeof(u32));
			if (e) {
				print_parser_error(e);
				return -1;
			}
			println("Emitted ssa");
		}
	}

	return 0;
}
