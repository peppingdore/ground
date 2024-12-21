#pragma once

#include "../grd_base.h"
#include "../grd_range.h"
#include "../grd_reflect.h"

template <typename T>
struct GrdVectorMembers1 {
	union {
		T x;
		T components[1];
	};

	GRD_REFLECT(GrdVectorMembers1) {
		type->name = grd_heap_sprintf("GrdVector1<%s>", grd_reflect_type_of<T>()->name);
		GRD_MEMBER(x);
	}
};

template <typename T>
struct GrdVectorMembers2 {
	union {
		struct {
			T x;
			T y;
		};
		T components[2];
	};

	GRD_REFLECT(GrdVectorMembers2) {
		type->name = grd_heap_sprintf("GrdVector2<%s>", grd_reflect_type_of<T>()->name);
		GRD_MEMBER(x);
		GRD_MEMBER(y);
	}
};

template <typename T>
struct GrdVectorMembers3 {
	union {
		struct {
			union {
				struct {
					T x;
					T y;
				};
				GrdVectorMembers2<T> v2;
			};
			T z;
		};
		T components[3];
	};

	GRD_REFLECT(GrdVectorMembers3) {
		type->name = grd_heap_sprintf("GrdVector3<%s>", grd_reflect_type_of<T>()->name);
		GRD_MEMBER(x);
		GRD_MEMBER(y);
		GRD_MEMBER(z);
	}
};

template <typename T, int N>
struct GrdVectorMembers4 {
	union {
		struct {
			union {
				struct {
					union {
						struct {
							T x;
							T y;
						};
						GrdVectorMembers2<T> v2;
					};
					T z;
				};
				GrdVectorMembers3<T> v3;
			};
			T w;
		};
		T components[N];
	};

	GRD_REFLECT(GrdVectorMembers4) {
		type->name = grd_heap_sprintf("GrdVector4<%s>", grd_reflect_type_of<T>()->name);
		GRD_MEMBER(x);
		GRD_MEMBER(y);
		GRD_MEMBER(z);
		GRD_MEMBER(w);
	}
};

template <int N, typename T>
struct GrdVectorMembers { T components[N]; };
template <typename T>
struct GrdVectorMembers<1, T>: GrdVectorMembers1<T> {};
template <typename T>
struct GrdVectorMembers<2, T>: GrdVectorMembers2<T> {};
template <typename T>
struct GrdVectorMembers<3, T>: GrdVectorMembers3<T> {};
template <int N, typename T> requires (N >= 4)
struct GrdVectorMembers<N, T>: GrdVectorMembers4<T, N> {};


template <int N, typename T>
struct GrdBaseVector: GrdVectorMembers<N, T> {
	static_assert(N > 0);

	using GrdVectorMembers<N, T>::components;

	T& operator[](int idx) {
		return components[idx];
	}

	template <typename... Pack>
	static GrdBaseVector grd_make(Pack... args) {
		T arr[N] = { (T) args... };
		GrdBaseVector vec;
		for (auto i: grd_range(N)) {
			vec.components[i] = arr[i];
		}
		return vec;
	}

	template <typename Thing> requires
		std::is_integral_v<T> &&
		std::is_floating_point_v<decltype(Thing().components[0])> 
	static GrdBaseVector grd_make(Thing thing) {
		static_assert(N == grd_static_array_count(thing.components));
		GrdBaseVector result;
		for (auto i: grd_range(N)) {
			result.components[i] = round(thing.components[i]);
		}
		return result;
	}

	template <int Start, int Length>
	constexpr auto slice() {
		static_assert(Length + Start <= N);
		GrdBaseVector<Length - Start, T> result;
		for (int i = Start; i < Length; i++) {
			result.components[i - Start] = components[i];
		}
		return result;
	}

	auto operator-(GrdBaseVector rhs) {
		auto result = *this;
		for (auto i: grd_range(N)) {
			result.components[i] -= rhs.components[i];
		}
		return result;
	}

	auto operator+(GrdBaseVector rhs) {
		auto result = *this;
		for (auto i: grd_range(N)) {
			result.components[i] += rhs.components[i];
		}
		return result;
	}

	auto operator*(T x) {
		auto result = *this;
		for (auto i: grd_range(N)) {
			result.components[i] *= x;
		}
		return result;
	}

	auto operator/(T x) {
		auto result = *this;
		for (auto i: grd_range(N)) {
			result.components[i] *= x;
		}
		return result;
	}

	auto operator-() {
		auto result = *this;
		for (auto i: grd_range(N)) {
			result.components[i] = -result.components[i];
		}
		return result;
	}

	bool operator==(GrdBaseVector rhs) {
		for (auto i: grd_range(N)) {
			if (components[i] != rhs.components[i]) {
				return false;
			}
		}
		return true;
	}

	// inline static StructType* grd_reflect_type(GrdBaseVector* x, StructType* type) {
	// 	GrdVectorMembers<N, T>::grd_reflect_type(x, type);
	// }
};

template <typename T> using GrdVector2Impl = GrdBaseVector<2, T>;
template <typename T> using GrdVector3Impl = GrdBaseVector<3, T>;
template <typename T> using GrdVector4Impl = GrdBaseVector<4, T>;

using GrdVector2_f32 = GrdVector2Impl<f32>;
using GrdVector2_s32 = GrdVector2Impl<s32>;
using GrdVector2_f64 = GrdVector2Impl<f64>;
using GrdVector2_s64 = GrdVector2Impl<s64>;

using GrdVector3_f32 = GrdVector3Impl<f32>;
using GrdVector3_s32 = GrdVector3Impl<s32>;
using GrdVector3_f64 = GrdVector3Impl<f64>;
using GrdVector3_s64 = GrdVector3Impl<s64>;


using GrdVector4_f32 = GrdVector4Impl<f32>;
using GrdVector4_s32 = GrdVector4Impl<s32>;
using GrdVector4_f64 = GrdVector4Impl<f64>;
using GrdVector4_s64 = GrdVector4Impl<s64>;


using GrdVector2 = GrdVector2_f64;
using GrdVector3 = GrdVector3_f64;
using GrdVector4 = GrdVector4_f64;

using GrdVector2i = GrdVector2_s64;
using GrdVector3i = GrdVector3_s64;
using GrdVector4i = GrdVector4_s64;


GRD_DEDUP auto grd_make_vector2(auto x, auto y) {
	return GrdVector2::grd_make(x, y);	
}
GRD_DEDUP auto grd_make_vector3(auto x, auto y, auto z) {
	return GrdVector3::grd_make(x, y, z);	
}
GRD_DEDUP auto grd_make_vector4(auto x, auto y, auto z, auto w) {
	return GrdVector4::grd_make(x, y, z, w);	
}

template <int N, typename T>
GRD_DEDUP auto grd_dot(GrdBaseVector<N, T> a, GrdBaseVector<N, T> b) {
	T sum = 0;
	for (auto i: grd_range(grd_static_array_count(a.components))) {
		sum += a[i] * b[i];
	}
	return sum;
}

template <int N, typename T>
GRD_DEDUP auto grd_project(GrdBaseVector<N, T> line_start, GrdBaseVector<N, T> line_end, GrdBaseVector<N, T> point) {
	auto line          = line_end - line_start;
	auto line_to_point = point    - line_start;
	return line_start + line * (grd_dot(line_to_point, line) / grd_dot(line, line));
}

template <int N, typename T>
GRD_DEDUP auto grd_magnitude_squared(GrdBaseVector<N, T> v) {
	T sum = 0;
	for (auto i: grd_range(N)) {
		sum += v.components[i] * v.components[i];
	}
	return sum;
}

template <int N, typename T>
GRD_DEDUP auto grd_magnitude(GrdBaseVector<N, T> v) {
	return sqrt(grd_magnitude_squared(v));
}

template <int N, typename T>
GRD_DEDUP auto grd_normalize(GrdBaseVector<N, T> v) {
	auto mag = grd_magnitude(v);
	if (mag == 0) {
		return v;
	}
	return v / mag;
}

template <int N, typename T>
GRD_DEDUP auto grd_lerp(GrdBaseVector<N, T> a, GrdBaseVector<N, T> b, f64 c) {
	return a + (b - a) * c;
}
