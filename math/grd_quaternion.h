#pragma once

#include "grd_math_base.h"

template <typename T>
struct GrdBaseQuaternion {
	T x = 0;
	T y = 0;
	T z = 0;
	T w = 1;

	auto operator*(GrdBaseQuaternion rhs) {
		return GrdBaseQuaternion {
			.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			.y = w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
			.z = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
			.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
		};
	}

	auto& operator*=(GrdBaseQuaternion rhs) {
		*this = *this * rhs;
		return *this;
	}
	
	template <typename U>
	static GrdBaseQuaternion make(GrdBaseQuaternion<U> x) {
		return { x.x, x.y, x.z, x.w };
	}

	// |axis| must be normalized.
	static GrdBaseQuaternion from_axis_angle(GrdBaseVector<3, T> axis, f64 angle) {
		f64 half_angle = angle / 2;
		f64 sin_half_angle = sin(half_angle);
		return { axis.x * sin_half_angle, axis.y * sin_half_angle, axis.z * sin_half_angle, cos(half_angle) };
	}

	static GrdBaseQuaternion from_euler_angles(GrdBaseVector<3, T> euler_angles) {
		f64 x = euler_angles.x / 2;
		f64 y = euler_angles.y / 2;
		f64 z = euler_angles.z / 2;
		f64 cx = cos(x);
		f64 cy = cos(y);
		f64 cz = cos(z);
		f64 sx = sin(x);
		f64 sy = sin(y);
		f64 sz = sin(z);
		return {
			.x = sx * cy * cz - cx * sy * sz,
			.y = cx * sy * cz + sx * cy * sz,
			.z = cx * cy * sz - sx * sy * cz,
			.w = cx * cy * cz + sx * sy * sz
		};
	}

	GRD_REFLECT(GrdBaseQuaternion) {
		type->name = grd_heap_sprintf("GrdBaseQuanternion<%s>", grd_reflect_type_of<T>()->name);
		GRD_MEMBER(x);
		GRD_MEMBER(y);
		GRD_MEMBER(z);
		GRD_MEMBER(w);
	}
};

using GrdQuaternion_f32 = GrdBaseQuaternion<f32>;
using GrdQuaternion_f64 = GrdBaseQuaternion<f64>;
using GrdQuaternion = GrdQuaternion_f64;

template <typename T>
GRD_DEF operator*(GrdBaseVector<3, T> a, GrdBaseQuaternion<T> b) -> GrdBaseVector<3, T> {
	return GrdBaseVector<3, T> {
		a.x * b.w + a.y * b.z - a.z * b.y,
		a.y * b.w + a.z * b.x - a.x * b.z,
		a.z * b.w + a.x * b.y - a.y * b.x
	};
}

template <typename T>
GRD_DEF grd_to_axis_angle(GrdBaseQuaternion<T> q) -> GrdTuple<GrdBaseVector<3, T>, T> {
	T norm = sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
    if (norm < 1e-6) {
		return { { 1, 0, 0 }, 0 };
    }
    q.w /= norm;
    q.x /= norm;
    q.y /= norm;
    q.z /= norm;

	// Clamp to avoid numerical issues in acos
    T w_clamped = grd_max(-1.0, grd_min(1.0, q.w));
    // Compute the angle in radians
    T angle = 2.0 * acos(w_clamped);
    // Compute the axis components
    T s = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
    if (s < 1e-6) {
		return { { 1, 0, 0 }, 0 };
    } else {
		return { { q.x / s, q.y / s, q.z / s }, angle };
    }
}
