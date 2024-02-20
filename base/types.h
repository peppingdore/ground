#pragma once

#include <stdint.h>
#include <float.h>

using s8   = int8_t;
using u8   = uint8_t;
using s16  = int16_t;
using u16  = uint16_t;
using s32  = int32_t;
using u32  = uint32_t;
using s64  = int64_t;
using u64  = uint64_t;
using f32  = float;
using f64  = double;

constexpr s8   s8_max  = 0x7f;
constexpr s8   s8_min  = -s8_max - 1;
constexpr u8   u8_max  = 0xff;
constexpr u8   u8_min  = 0;
constexpr s16  s16_max = 0x7fff;
constexpr s16  s16_min = -s16_max - 1;
constexpr u16  u16_max = 0xffff;
constexpr u16  u16_min = 0;
constexpr s32  s32_max = 0x7fff'ffff;
constexpr s32  s32_min = -s32_max - 1;
constexpr u32  u32_max = 0xffff'ffff;
constexpr u32  u32_min = 0;
constexpr s64  s64_max = 0x7fff'ffff'ffff'ffff;
constexpr s64  s64_min = -s64_max - 1;
constexpr u64  u64_max = 0xffff'ffff'ffff'ffff;
constexpr u64  u64_min = 0;

constexpr s8  s8_sign_mask    = s8 (1 << 7);
constexpr s16 s16_sign_mask   = s16(1 << 15);
constexpr s32 s32_sign_mask   = s32(1 << 31);
constexpr s64 s64_sign_mask   = s64(1LL << 63);

constexpr f32 f32_epsilon = FLT_EPSILON;
constexpr f64 f64_epsilon = DBL_EPSILON;

using Byte_Order = bool;
constexpr Byte_Order BYTE_ORDER_LITTLE_ENDIAN = false;
constexpr Byte_Order BYTE_ORDER_BIG_ENDIAN    = true;
