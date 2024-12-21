#pragma once

/*
 * SpookyHash
 *
 * Copyright (c) 2015, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 25/01/15 12:21
 *
 * ----------
 * SpookyHash
 * ----------
 *
 * Author(s)
 * Bob Jenkins (http://burtleburtle.net/bob/hash/spooky.html)
 *
 * Description
 * Very fast non cryptographic hash
 */

#include "../../grd_base.h"

#if defined(__GNUC__) || defined(__clang__)
	#define SPOOKYHASH_RESTRICT __restrict__
#elif defined(__INTEL_COMPILER) || defined(_MSC_VER)
	#define SPOOKYHASH_RESTRICT     __restrict
#else
#warning Impossible to force functions inlining. Expect performance issues.
	#define SPOOKYHASH_RESTRICT
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define SPOOKYHASH_MEMCPY   __builtin_memcpy
	#define SPOOKYHASH_MEMSET   __builtin_memset
#else
	#include <string.h>
	#define SPOOKYHASH_MEMCPY   memcpy
	#define SPOOKYHASH_MEMSET   memset
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	#define SPOOKYHASH_LITTLE_ENDIAN_64(b)   ((u64)b)
	#define SPOOKYHASH_LITTLE_ENDIAN_32(b)   ((uint32_t)b)
	#define SPOOKYHASH_LITTLE_ENDIAN_16(b)   ((uint16_t)b)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#if __GNUC__ * 100 + __GNUC_MINOR__ >= 403 || defined(__clang__)
	#define SPOOKYHASH_LITTLE_ENDIAN_64(b)   __builtin_bswap64(b)
	#define SPOOKYHASH_LITTLE_ENDIAN_32(b)   __builtin_bswap32(b)
	#define SPOOKYHASH_LITTLE_ENDIAN_16(b)   __builtin_bswap16(b)
#else
	#warning Using bulk byte swap routines. Expect performance issues.
	#define SPOOKYHASH_LITTLE_ENDIAN_64(b)   ((((b) & 0xFF00000000000000ull) >> 56) | (((b) & 0x00FF000000000000ull) >> 40) | (((b) & 0x0000FF0000000000ull) >> 24) | (((b) & 0x000000FF00000000ull) >> 8) | (((b) & 0x00000000FF000000ull) << 8) | (((b) & 0x0000000000FF0000ull) << 24ull) | (((b) & 0x000000000000FF00ull) << 40) | (((b) & 0x00000000000000FFull) << 56))
	#define SPOOKYHASH_LITTLE_ENDIAN_32(b)   ((((b) & 0xFF000000) >> 24) | (((b) & 0x00FF0000) >> 8) | (((b) & 0x0000FF00) << 8) | (((b) & 0x000000FF) << 24))
	#define SPOOKYHASH_LITTLE_ENDIAN_16(b)   ((((b) & 0xFF00) >> 8) | (((b) & 0x00FF) << 8))
#endif
#else
	#error Unknow endianness
#endif

#define SPOOKYHASH_MAJOR_VERSION   1
#define SPOOKYHASH_MINOR_VERSION   0
#define SPOOKYHASH_REVISION        7



#define SPOOKYHASH_ALLOW_UNALIGNED_READS   0
#define SPOOKYHASH_ROTATE(x, k) (((x) << (k)) | (((x) >> (64 - (k)))))

#define SPOOKYHASH_VARIABLES (12)


#define SPOOKYHASH_BLOCK_SIZE (SPOOKYHASH_VARIABLES * 8)
#define SPOOKYHASH_BUFFER_SIZE (2 * SPOOKYHASH_BLOCK_SIZE)
#define SPOOKYHASH_CONSTANT (0xdeadbeefdeadbeefLL)




typedef struct {
    u64    m_data[2 * SPOOKYHASH_VARIABLES];
    u64    m_state[SPOOKYHASH_VARIABLES];
    size_t m_length;
    u8     m_remainder;
} spookyhash_context;


GRD_EXPORT void spookyhash_context_init(spookyhash_context *context, u64 seed1, u64 seed2) {
    context->m_length = 0;
    context->m_remainder = 0;
    context->m_state[0] = seed1;
    context->m_state[1] = seed2;
}




GRD_EXPORT void spookyhash_short_end(u64 *SPOOKYHASH_RESTRICT h0, u64 *SPOOKYHASH_RESTRICT h1, u64 *SPOOKYHASH_RESTRICT h2, u64 *SPOOKYHASH_RESTRICT h3) {
    *h3 ^= *h2;
    *h2 = SPOOKYHASH_ROTATE(*h2, 15);
    *h3 += *h2;
    *h0 ^= *h3;
    *h3 = SPOOKYHASH_ROTATE(*h3, 52);
    *h0 += *h3;
    *h1 ^= *h0;
    *h0 = SPOOKYHASH_ROTATE(*h0, 26);
    *h1 += *h0;
    *h2 ^= *h1;
    *h1 = SPOOKYHASH_ROTATE(*h1, 51);
    *h2 += *h1;
    *h3 ^= *h2;
    *h2 = SPOOKYHASH_ROTATE(*h2, 28);
    *h3 += *h2;
    *h0 ^= *h3;
    *h3 = SPOOKYHASH_ROTATE(*h3, 9);
    *h0 += *h3;
    *h1 ^= *h0;
    *h0 = SPOOKYHASH_ROTATE(*h0, 47);
    *h1 += *h0;
    *h2 ^= *h1;
    *h1 = SPOOKYHASH_ROTATE(*h1, 54);
    *h2 += *h1;
    *h3 ^= *h2;
    *h2 = SPOOKYHASH_ROTATE(*h2, 32);
    *h3 += *h2;
    *h0 ^= *h3;
    *h3 = SPOOKYHASH_ROTATE(*h3, 25);
    *h0 += *h3;
    *h1 ^= *h0;
    *h0 = SPOOKYHASH_ROTATE(*h0, 63);
    *h1 += *h0;
}

GRD_EXPORT void spookyhash_short_mix(u64 *SPOOKYHASH_RESTRICT h0, u64 *SPOOKYHASH_RESTRICT h1, u64 *SPOOKYHASH_RESTRICT h2, u64 *SPOOKYHASH_RESTRICT h3) {
    *h2 = SPOOKYHASH_ROTATE(*h2, 50);
    *h2 += *h3;
    *h0 ^= *h2;
    *h3 = SPOOKYHASH_ROTATE(*h3, 52);
    *h3 += *h0;
    *h1 ^= *h3;
    *h0 = SPOOKYHASH_ROTATE(*h0, 30);
    *h0 += *h1;
    *h2 ^= *h0;
    *h1 = SPOOKYHASH_ROTATE(*h1, 41);
    *h1 += *h2;
    *h3 ^= *h1;
    *h2 = SPOOKYHASH_ROTATE(*h2, 54);
    *h2 += *h3;
    *h0 ^= *h2;
    *h3 = SPOOKYHASH_ROTATE(*h3, 48);
    *h3 += *h0;
    *h1 ^= *h3;
    *h0 = SPOOKYHASH_ROTATE(*h0, 38);
    *h0 += *h1;
    *h2 ^= *h0;
    *h1 = SPOOKYHASH_ROTATE(*h1, 37);
    *h1 += *h2;
    *h3 ^= *h1;
    *h2 = SPOOKYHASH_ROTATE(*h2, 62);
    *h2 += *h3;
    *h0 ^= *h2;
    *h3 = SPOOKYHASH_ROTATE(*h3, 34);
    *h3 += *h0;
    *h1 ^= *h3;
    *h0 = SPOOKYHASH_ROTATE(*h0, 5);
    *h0 += *h1;
    *h2 ^= *h0;
    *h1 = SPOOKYHASH_ROTATE(*h1, 36);
    *h1 += *h2;
    *h3 ^= *h1;
}

GRD_EXPORT void spookyhash_short(const void *SPOOKYHASH_RESTRICT message, size_t length, u64 *SPOOKYHASH_RESTRICT hash1, u64 *SPOOKYHASH_RESTRICT hash2) {
#if !SPOOKYHASH_ALLOW_UNALIGNED_READS
    u64 buffer[2 * SPOOKYHASH_VARIABLES];
#endif

    union {
        const u8 *p8;
        uint32_t *p32;
        u64 *p64;
        size_t i;
    } u;
    u.p8 = (const u8 *) message;

#if !SPOOKYHASH_ALLOW_UNALIGNED_READS
    if (u.i & 0x7) {
        SPOOKYHASH_MEMCPY(buffer, message, length);
        u.p64 = buffer;
    }
#endif

    size_t remainder = length % 32;
    u64 a = *hash1;
    u64 b = *hash2;
    u64 c = SPOOKYHASH_CONSTANT;
    u64 d = SPOOKYHASH_CONSTANT;

    if (length > 15) {
        const u64 *end = u.p64 + (length / 32) * 4;
        for (; u.p64 < end; u.p64 += 4) {
            c += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[0]);
            d += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[1]);
            spookyhash_short_mix(&a, &b, &c, &d);
            a += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[2]);
            b += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[3]);
        }

        if (remainder >= 16) {
            c += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[0]);
            d += SPOOKYHASH_LITTLE_ENDIAN_64(u.p64[1]);
            spookyhash_short_mix(&a, &b, &c, &d);
            u.p64 += 2;
            remainder -= 16;
        }
    }

    d += ((u64) length) << 56;
    switch (remainder) {
        default:
            break;
        case 15:
            d += ((u64) u.p8[14]) << 48;
        case 14:
            d += ((u64) u.p8[13]) << 40;
        case 13:
            d += ((u64) u.p8[12]) << 32;
        case 12:
            d += u.p32[2];
            c += u.p64[0];
            break;
        case 11:
            d += ((u64) u.p8[10]) << 16;
        case 10:
            d += ((u64) u.p8[9]) << 8;
        case 9:
            d += (u64) u.p8[8];
        case 8:
            c += u.p64[0];
            break;
        case 7:
            c += ((u64) u.p8[6]) << 48;
        case 6:
            c += ((u64) u.p8[5]) << 40;
        case 5:
            c += ((u64) u.p8[4]) << 32;
        case 4:
            c += u.p32[0];
            break;
        case 3:
            c += ((u64) u.p8[2]) << 16;
        case 2:
            c += ((u64) u.p8[1]) << 8;
        case 1:
            c += (u64) u.p8[0];
            break;
        case 0:
            c += SPOOKYHASH_CONSTANT;
            d += SPOOKYHASH_CONSTANT;
    }
    spookyhash_short_end(&a, &b, &c, &d);
    *hash1 = a;
    *hash2 = b;
}

GRD_EXPORT void spookyhash_mix(const u64 *SPOOKYHASH_RESTRICT data, u64 *SPOOKYHASH_RESTRICT s0, u64 *SPOOKYHASH_RESTRICT s1, u64 *SPOOKYHASH_RESTRICT s2, u64 *SPOOKYHASH_RESTRICT s3, u64 *SPOOKYHASH_RESTRICT s4, u64 *SPOOKYHASH_RESTRICT s5, u64 *SPOOKYHASH_RESTRICT s6, u64 *SPOOKYHASH_RESTRICT s7, u64 *SPOOKYHASH_RESTRICT s8, u64 *SPOOKYHASH_RESTRICT s9, u64 *SPOOKYHASH_RESTRICT s10, u64 *SPOOKYHASH_RESTRICT s11) {
    *s0 += SPOOKYHASH_LITTLE_ENDIAN_64(data[0]);
    *s2 ^= *s10;
    *s11 ^= *s0;
    *s0 = SPOOKYHASH_ROTATE(*s0, 11);
    *s11 += *s1;
    *s1 += SPOOKYHASH_LITTLE_ENDIAN_64(data[1]);
    *s3 ^= *s11;
    *s0 ^= *s1;
    *s1 = SPOOKYHASH_ROTATE(*s1, 32);
    *s0 += *s2;
    *s2 += SPOOKYHASH_LITTLE_ENDIAN_64(data[2]);
    *s4 ^= *s0;
    *s1 ^= *s2;
    *s2 = SPOOKYHASH_ROTATE(*s2, 43);
    *s1 += *s3;
    *s3 += SPOOKYHASH_LITTLE_ENDIAN_64(data[3]);
    *s5 ^= *s1;
    *s2 ^= *s3;
    *s3 = SPOOKYHASH_ROTATE(*s3, 31);
    *s2 += *s4;
    *s4 += SPOOKYHASH_LITTLE_ENDIAN_64(data[4]);
    *s6 ^= *s2;
    *s3 ^= *s4;
    *s4 = SPOOKYHASH_ROTATE(*s4, 17);
    *s3 += *s5;
    *s5 += SPOOKYHASH_LITTLE_ENDIAN_64(data[5]);
    *s7 ^= *s3;
    *s4 ^= *s5;
    *s5 = SPOOKYHASH_ROTATE(*s5, 28);
    *s4 += *s6;
    *s6 += SPOOKYHASH_LITTLE_ENDIAN_64(data[6]);
    *s8 ^= *s4;
    *s5 ^= *s6;
    *s6 = SPOOKYHASH_ROTATE(*s6, 39);
    *s5 += *s7;
    *s7 += SPOOKYHASH_LITTLE_ENDIAN_64(data[7]);
    *s9 ^= *s5;
    *s6 ^= *s7;
    *s7 = SPOOKYHASH_ROTATE(*s7, 57);
    *s6 += *s8;
    *s8 += SPOOKYHASH_LITTLE_ENDIAN_64(data[8]);
    *s10 ^= *s6;
    *s7 ^= *s8;
    *s8 = SPOOKYHASH_ROTATE(*s8, 55);
    *s7 += *s9;
    *s9 += SPOOKYHASH_LITTLE_ENDIAN_64(data[9]);
    *s11 ^= *s7;
    *s8 ^= *s9;
    *s9 = SPOOKYHASH_ROTATE(*s9, 54);
    *s8 += *s10;
    *s10 += SPOOKYHASH_LITTLE_ENDIAN_64(data[10]);
    *s0 ^= *s8;
    *s9 ^= *s10;
    *s10 = SPOOKYHASH_ROTATE(*s10, 22);
    *s9 += *s11;
    *s11 += SPOOKYHASH_LITTLE_ENDIAN_64(data[11]);
    *s1 ^= *s9;
    *s10 ^= *s11;
    *s11 = SPOOKYHASH_ROTATE(*s11, 46);
    *s10 += *s0;
}

GRD_EXPORT void spookyhash_end_partial(u64 *SPOOKYHASH_RESTRICT h0, u64 *SPOOKYHASH_RESTRICT h1, u64 *SPOOKYHASH_RESTRICT h2, u64 *SPOOKYHASH_RESTRICT h3, u64 *SPOOKYHASH_RESTRICT h4, u64 *SPOOKYHASH_RESTRICT h5, u64 *SPOOKYHASH_RESTRICT h6, u64 *SPOOKYHASH_RESTRICT h7, u64 *SPOOKYHASH_RESTRICT h8, u64 *SPOOKYHASH_RESTRICT h9, u64 *SPOOKYHASH_RESTRICT h10, u64 *SPOOKYHASH_RESTRICT h11) {
    *h11 += *h1;
    *h2 ^= *h11;
    *h1 = SPOOKYHASH_ROTATE(*h1, 44);
    *h0 += *h2;
    *h3 ^= *h0;
    *h2 = SPOOKYHASH_ROTATE(*h2, 15);
    *h1 += *h3;
    *h4 ^= *h1;
    *h3 = SPOOKYHASH_ROTATE(*h3, 34);
    *h2 += *h4;
    *h5 ^= *h2;
    *h4 = SPOOKYHASH_ROTATE(*h4, 21);
    *h3 += *h5;
    *h6 ^= *h3;
    *h5 = SPOOKYHASH_ROTATE(*h5, 38);
    *h4 += *h6;
    *h7 ^= *h4;
    *h6 = SPOOKYHASH_ROTATE(*h6, 33);
    *h5 += *h7;
    *h8 ^= *h5;
    *h7 = SPOOKYHASH_ROTATE(*h7, 10);
    *h6 += *h8;
    *h9 ^= *h6;
    *h8 = SPOOKYHASH_ROTATE(*h8, 13);
    *h7 += *h9;
    *h10 ^= *h7;
    *h9 = SPOOKYHASH_ROTATE(*h9, 38);
    *h8 += *h10;
    *h11 ^= *h8;
    *h10 = SPOOKYHASH_ROTATE(*h10, 53);
    *h9 += *h11;
    *h0 ^= *h9;
    *h11 = SPOOKYHASH_ROTATE(*h11, 42);
    *h10 += *h0;
    *h1 ^= *h10;
    *h0 = SPOOKYHASH_ROTATE(*h0, 54);
}

GRD_EXPORT void spookyhash_end(const u64 *SPOOKYHASH_RESTRICT data, u64 *SPOOKYHASH_RESTRICT h0, u64 *SPOOKYHASH_RESTRICT h1, u64 *SPOOKYHASH_RESTRICT h2, u64 *SPOOKYHASH_RESTRICT h3, u64 *SPOOKYHASH_RESTRICT h4, u64 *SPOOKYHASH_RESTRICT h5, u64 *SPOOKYHASH_RESTRICT h6, u64 *SPOOKYHASH_RESTRICT h7, u64 *SPOOKYHASH_RESTRICT h8, u64 *SPOOKYHASH_RESTRICT h9, u64 *SPOOKYHASH_RESTRICT h10, u64 *SPOOKYHASH_RESTRICT h11) {
    *h0 += SPOOKYHASH_LITTLE_ENDIAN_64(data[0]);
    *h1 += SPOOKYHASH_LITTLE_ENDIAN_64(data[1]);
    *h2 += SPOOKYHASH_LITTLE_ENDIAN_64(data[2]);
    *h3 += SPOOKYHASH_LITTLE_ENDIAN_64(data[3]);
    *h4 += SPOOKYHASH_LITTLE_ENDIAN_64(data[4]);
    *h5 += SPOOKYHASH_LITTLE_ENDIAN_64(data[5]);
    *h6 += SPOOKYHASH_LITTLE_ENDIAN_64(data[6]);
    *h7 += SPOOKYHASH_LITTLE_ENDIAN_64(data[7]);
    *h8 += SPOOKYHASH_LITTLE_ENDIAN_64(data[8]);
    *h9 += SPOOKYHASH_LITTLE_ENDIAN_64(data[9]);
    *h10 += SPOOKYHASH_LITTLE_ENDIAN_64(data[10]);
    *h11 += SPOOKYHASH_LITTLE_ENDIAN_64(data[11]);
    spookyhash_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
    spookyhash_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
    spookyhash_end_partial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
}

GRD_EXPORT void spookyhash_128(const void *SPOOKYHASH_RESTRICT message, size_t length, u64 *SPOOKYHASH_RESTRICT hash1, u64 *SPOOKYHASH_RESTRICT hash2) {
    if (length < SPOOKYHASH_BUFFER_SIZE) {
        spookyhash_short(message, length, hash1, hash2);
        return;
    }

    u64 h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
    u64 buf[SPOOKYHASH_VARIABLES];
    u64 *end;
    union {
        const u8 *p8;
        u64 *p64;
        size_t i;
    } u;
    size_t remainder;

    h0 = h3 = h6 = h9 = *hash1;
    h1 = h4 = h7 = h10 = *hash2;
    h2 = h5 = h8 = h11 = SPOOKYHASH_CONSTANT;

    u.p8 = (const u8 *) message;
    end = u.p64 + (length / SPOOKYHASH_BLOCK_SIZE) * SPOOKYHASH_VARIABLES;

    if (SPOOKYHASH_ALLOW_UNALIGNED_READS || ((u.i & 0x7) == 0)) {
        while (u.p64 < end) {
            spookyhash_mix(u.p64, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
            u.p64 += SPOOKYHASH_VARIABLES;
        }
    } else {
        while (u.p64 < end) {
            SPOOKYHASH_MEMCPY(buf, u.p64, SPOOKYHASH_BLOCK_SIZE);
            spookyhash_mix(buf, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
            u.p64 += SPOOKYHASH_VARIABLES;
        }
    }

    remainder = (length - ((const u8 *) end - (const u8 *) message));
    SPOOKYHASH_MEMCPY(buf, end, remainder);
    SPOOKYHASH_MEMSET(((u8 *) buf) + remainder, 0, SPOOKYHASH_BLOCK_SIZE - remainder);
    ((u8 *) buf)[SPOOKYHASH_BLOCK_SIZE - 1] = (u8) remainder;

    spookyhash_end(buf, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
    *hash1 = h0;
    *hash2 = h1;
}

GRD_EXPORT u64 spookyhash_64(const void *message, size_t length, u64 seed) {
    u64 hash1 = seed;
    spookyhash_128(message, length, &hash1, &seed);
    return hash1;
}

GRD_EXPORT uint32_t spookyhash_32(const void *message, size_t length, uint32_t seed) {
    u64 hash1 = seed, hash2 = seed;
    spookyhash_128(message, length, &hash1, &hash2);
    return (uint32_t) hash1;
}

GRD_EXPORT void spookyhash_update(spookyhash_context *SPOOKYHASH_RESTRICT context, const void *SPOOKYHASH_RESTRICT message, size_t length) {
    u64 h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
    size_t newLength = length + context->m_remainder;
    u8 remainder;
    union {
        const u8 *p8;
        u64 *p64;
        size_t i;
    } u;
    const u64 *end;

    if (newLength < SPOOKYHASH_BUFFER_SIZE) {
        SPOOKYHASH_MEMCPY(&((u8 *) context->m_data)[context->m_remainder], message, length);
        context->m_length = length + context->m_length;
        context->m_remainder = (u8) newLength;
        return;
    }

    if (context->m_length < SPOOKYHASH_BUFFER_SIZE) {
        h0 = h3 = h6 = h9 = context->m_state[0];
        h1 = h4 = h7 = h10 = context->m_state[1];
        h2 = h5 = h8 = h11 = SPOOKYHASH_CONSTANT;
    }
    else {
        h0 = context->m_state[0];
        h1 = context->m_state[1];
        h2 = context->m_state[2];
        h3 = context->m_state[3];
        h4 = context->m_state[4];
        h5 = context->m_state[5];
        h6 = context->m_state[6];
        h7 = context->m_state[7];
        h8 = context->m_state[8];
        h9 = context->m_state[9];
        h10 = context->m_state[10];
        h11 = context->m_state[11];
    }
    context->m_length = length + context->m_length;

    if (context->m_remainder) {
        u8 prefix = (u8) (SPOOKYHASH_BUFFER_SIZE - context->m_remainder);
        SPOOKYHASH_MEMCPY(&(((u8 *) context->m_data)[context->m_remainder]), message, prefix);
        u.p64 = context->m_data;
        spookyhash_mix(u.p64, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
        spookyhash_mix(&u.p64[SPOOKYHASH_VARIABLES], &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
        u.p8 = ((const u8 *) message) + prefix;
        length -= prefix;
    } else {
        u.p8 = (const u8 *) message;
    }

    end = u.p64 + (length / SPOOKYHASH_BLOCK_SIZE) * SPOOKYHASH_VARIABLES;
    remainder = (u8) (length - ((const u8 *) end - u.p8));
    if (SPOOKYHASH_ALLOW_UNALIGNED_READS || (u.i & 0x7) == 0) {
        while (u.p64 < end) {
            spookyhash_mix(u.p64, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
            u.p64 += SPOOKYHASH_VARIABLES;
        }
    }
    else {
        while (u.p64 < end) {
            SPOOKYHASH_MEMCPY(context->m_data, u.p8, SPOOKYHASH_BLOCK_SIZE);
            spookyhash_mix(context->m_data, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
            u.p64 += SPOOKYHASH_VARIABLES;
        }
    }

    context->m_remainder = remainder;
    SPOOKYHASH_MEMCPY(context->m_data, end, remainder);

    context->m_state[0] = h0;
    context->m_state[1] = h1;
    context->m_state[2] = h2;
    context->m_state[3] = h3;
    context->m_state[4] = h4;
    context->m_state[5] = h5;
    context->m_state[6] = h6;
    context->m_state[7] = h7;
    context->m_state[8] = h8;
    context->m_state[9] = h9;
    context->m_state[10] = h10;
    context->m_state[11] = h11;
}

GRD_EXPORT void spookyhash_final(spookyhash_context *SPOOKYHASH_RESTRICT context, u64 *SPOOKYHASH_RESTRICT hash1, u64 *SPOOKYHASH_RESTRICT hash2) {
    if (context->m_length < SPOOKYHASH_BUFFER_SIZE) {
        *hash1 = context->m_state[0];
        *hash2 = context->m_state[1];
        spookyhash_short(context->m_data, context->m_length, hash1, hash2);
        return;
    }

    const u64 *data = (const u64 *) context->m_data;
    u8 remainder = context->m_remainder;

    u64 h0 = context->m_state[0];
    u64 h1 = context->m_state[1];
    u64 h2 = context->m_state[2];
    u64 h3 = context->m_state[3];
    u64 h4 = context->m_state[4];
    u64 h5 = context->m_state[5];
    u64 h6 = context->m_state[6];
    u64 h7 = context->m_state[7];
    u64 h8 = context->m_state[8];
    u64 h9 = context->m_state[9];
    u64 h10 = context->m_state[10];
    u64 h11 = context->m_state[11];

    if (remainder >= SPOOKYHASH_BLOCK_SIZE) {
        spookyhash_mix(data, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
        data += SPOOKYHASH_VARIABLES;
        remainder -= SPOOKYHASH_BLOCK_SIZE;
    }

    SPOOKYHASH_MEMSET(&((u8 *) data)[remainder], 0, (SPOOKYHASH_BLOCK_SIZE - remainder));

    ((u8 *) data)[SPOOKYHASH_BLOCK_SIZE - 1] = remainder;

    spookyhash_end(data, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);

    *hash1 = h0;
    *hash2 = h1;
}
