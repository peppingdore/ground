#include "../error.h"
#include "../log.h"
#include "../defer.h"
#include "c_like_parser.h"
#include "ast_printer.h" 
#include "ast_runner.h"

// https://twitter.com/zozuar/status/1755381710227755046
//
// float i,e,R,s,z;vec3 m,q,p,d=FC.rgb/r.y-.5;q=hsv(t/6./PI,2.,.4);for(q.z--;i++<1e2;q-=d*e*R*.4,o+=log(++R+sin(vec4(1,2,3,0)+z*z/2e4))/2e2,p=vec3(log(R=length(q))-t/2.,e=-q.z/R,atan(q.x,q.y)),z=s,i>60.?d/=d,e+=1e-4:e)for(s=2.;s<1e3;s/=-.5)e-=abs(cos(dot(m=cos(p*s),q/q)))/s,z-=m.y;

// float i, e, R, s, z;
// vec3 m, q, p, d = FC.rgb / r.y - .5;
// q = hsv(t / 6. / PI, 2., .4);
// for (q.z--; i++ < 1e2; q -= d * e * R * .4, o += log(++R + sin(vec4(1, 2, 3, 0) + z * z / 2e4)) / 2e2, p = vec3(log(R = length(q)) - t / 2., e = -q.z / R, atan(q.x, q.y)), z = s, i > 60. ? d /= d, e += 1e-4 : e)
//     for (s = 2.; s < 1e3; s /=- .5)
//         e -= abs(cos(dot(m = cos(p * s), q / q))) / s, z -= m.y;

UnicodeString PROGRAM = UR"TAG(
float PI = 3.14;

void main() {
float i,e,R,s,z;
vec3 m,q,p,d=FC.rgb/r.y-.5;
q=hsv(t/6.0f/PI,2.0f,.4f);
for(q.z--;i++<1e2f;q-=d*e*R*.4,o+=log(++R+sin(vec4(1f,2f,3f,0f)+z*z/2e4f))/2e2f,p=vec3(log(R=length(q))-t/2.f,e=-q.z/R,atan(q.x,q.y)),z=s,i>60.f?d/=d,e+=1e-4f:e)
	for(s=2.f;s<1e3f;s/=-.5f)
		e-=abs(cos(dot_vec3(m=cos(p*s),q/q)))/s,z-=m.y;
}
)TAG"_b;


int main() {
	auto [program, e] = parse_c_like(PROGRAM);
	if (e) {
		if (auto error = reflect_cast<CLikeParserError>(e)) {
			print_parser_error(error);
		} else {
			print(e->text);
		}
		print("Generated at %", e->loc);
		return -1;
	}
	// print(print_ast_node(program));

	for (auto it: program->globals) {
		if (auto f = reflect_cast<AstFunction>(it)) {
			if (!f->block) {
				continue;
			}

			auto [ssa, e] = emit_function_ssa(c_allocator, f);
			if (e) {
				println(e);
				return -1;
			}
			println("Emitted ssa");
		}
	}

	return 0;
}
