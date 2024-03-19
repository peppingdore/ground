#pragma once

#include "../base.h"
#include "../range.h"

template <typename T>
struct Vector_Members1 {
	union {
		T x;
		T components[1];
	};
};

template <typename T>
struct Vector_Members2 {
	union {
		struct {
			T x;
			T y;
		};
		T components[2];
	};
};

template <typename T>
struct Vector_Members3 {
	union {
		struct {
			union {
				struct {
					T x;
					T y;
				};
				Vector_Members2<T> v2;
			};
			T z;
		};
		T components[3];
	};
};

template <typename T, int N>
struct Vector_Members4 {
	union {
		struct {
			union {
				struct {
					union {
						struct {
							T x;
							T y;
						};
						Vector_Members2<T> v2;
					};
					T z;
				};
				Vector_Members3<T> v3;
			};
			T w;
		};
		T components[N];
	};
};

template <int N, typename T>
struct Vector_Members { T components[N]; };
template <typename T>
struct Vector_Members<1, T>: Vector_Members1<T> {};
template <typename T>
struct Vector_Members<2, T>: Vector_Members2<T> {};
template <typename T>
struct Vector_Members<3, T>: Vector_Members3<T> {};
template <int N, typename T> requires (N >= 4)
struct Vector_Members<N, T>: Vector_Members4<T, N> {};


template <int N, typename T>
struct Base_Vector: Vector_Members<N, T> {
	static_assert(N > 0);

	using Vector_Members<N, T>::components;

	T& operator[](int idx) {
		return components[idx];
	}

	template <typename... Pack>
	static Base_Vector make(Pack... args) {
		T arr[N] = { args... };
		Base_Vector vec;
		for (auto i: range(N)) {
			vec.components[i] = arr[i];
		}
		return vec;
	}

	template <typename Thing> requires
		std::is_integral_v<T> &&
		std::is_floating_point_v<decltype(Thing().components[0])> 
	static Base_Vector make(Thing thing) {
		static_assert(N == array_count(thing.components));
		Base_Vector result;
		for (auto i: range(N)) {
			result.components[i] = round(thing.components[i]);
		}
		return result;
	}

	template <int Start, int Length>
	constexpr auto slice() {
		static_assert(Length + Start <= N);
		Base_Vector<Length - Start, T> result;
		for (int i = Start; i < Length; i++) {
			result.components[i - Start] = components[i];
		}
		return result;
	}

	auto operator-(Base_Vector rhs) {
		auto result = *this;
		for (auto i: range(N)) {
			result.components[i] -= rhs.components[i];
		}
		return result;
	}

	auto operator+(Base_Vector rhs) {
		auto result = *this;
		for (auto i: range(N)) {
			result.components[i] += rhs.components[i];
		}
		return result;
	}

	auto operator*(T x) {
		auto result = *this;
		for (auto i: range(N)) {
			result.components[i] *= x;
		}
		return result;
	}

	auto operator/(T x) {
		auto result = *this;
		for (auto i: range(N)) {
			result.components[i] *= x;
		}
		return result;
	}

	auto operator-() {
		auto result = *this;
		for (auto i: range(N)) {
			result.components[i] = -result.components[i];
		}
		return result;
	}

	bool operator==(Base_Vector rhs) {
		for (auto i: range(N)) {
			if (components[i] != rhs.components[i]) {
				return false;
			}
		}
		return true;
	}


	REFLECT_NAME(Base_Vector,
		heap_sprintf("Vector%d<%s>", N, reflect.type_of<T>()->name)
	) {
		MEMBER(components);
	}
};

template <typename T> using Vector2_Impl = Base_Vector<2, T>;
template <typename T> using Vector3_Impl = Base_Vector<3, T>;
template <typename T> using Vector4_Impl = Base_Vector<4, T>;

using Vector2_f32 = Vector2_Impl<f32>;
using Vector2_s32 = Vector2_Impl<s32>;
using Vector2_f64 = Vector2_Impl<f64>;
using Vector2_s64 = Vector2_Impl<s64>;

using Vector3_f32 = Vector3_Impl<f32>;
using Vector3_s32 = Vector3_Impl<s32>;
using Vector3_f64 = Vector3_Impl<f64>;
using Vector3_s64 = Vector3_Impl<s64>;


using Vector4_f32 = Vector4_Impl<f32>;
using Vector4_s32 = Vector4_Impl<s32>;
using Vector4_f64 = Vector4_Impl<f64>;
using Vector4_s64 = Vector4_Impl<s64>;


using Vector2 = Vector2_f64;
using Vector3 = Vector3_f64;
using Vector4 = Vector4_f64;

using Vector2i = Vector2_s64;
using Vector3i = Vector3_s64;
using Vector4i = Vector4_s64;


auto make_vector2(auto x, auto y) {
	return Vector2::make(x, y);	
}
auto make_vector3(auto x, auto y, auto z) {
	return Vector3::make(x, y, z);	
}
auto make_vector4(auto x, auto y, auto z, auto w) {
	return Vector4::make(x, y, z, w);	
}

template <int N, typename T>
auto dot(Base_Vector<N, T> a, Base_Vector<N, T> b) {
	T sum = 0;
	for (auto i: range(static_array_count(a.components))) {
		sum += a[i] * b[i];
	}
	return sum;
}

template <int N, typename T>
auto project(Base_Vector<N, T> line_start, Base_Vector<N, T> line_end, Base_Vector<N, T> point) {
	auto line          = line_end - line_start;
	auto line_to_point = point    - line_start;
	return line_start + line * (dot(line_to_point, line) / dot(line, line));
}

template <int N, typename T>
auto magnitude_squared(Base_Vector<N, T> v) {
	T sum = 0;
	for (auto i: range(N)) {
		sum += v.components[i] * v.components[i];
	}
	return sum;
}

template <int N, typename T>
auto magnitude(Base_Vector<N, T> v) {
	return sqrt(magnitude_squared(v));
}

template <int N, typename T>
auto normalize(Base_Vector<N, T> v) {
	auto mag = magnitude(v);
	if (mag == 0) {
		return v;
	}
	return v / mag;
}

template <int N, typename T>
auto lerp(Base_Vector<N, T> a, Base_Vector<N, T> b, f64 c) {
	return a + (b - a) * c;
}
