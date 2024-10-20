#pragma once

#include "grd_base.h"
#include "grd_allocator.h"
#include "grd_string.h"
#include "grd_reflect.h"
#include "math/grd_basic_functions.h"

#include <type_traits>
#include <limits>

struct GrdSmallString {
	char buf[127];
	u8   length = 0;

	char& operator[](s64 idx) {
		return buf[idx];
	}
};
static_assert(sizeof(GrdSmallString) == 128);

s64 grd_len(GrdSmallString x) {
	return x.length;
}

GrdString grd_as_str(GrdSmallString* x) {
	return { x->buf, x->length };
}

void grd_append(GrdSmallString* res, char c) {
	if (res->length >= grd_static_array_count(res->buf)) {
		return;
	}
	res->buf[res->length] = c;
	res->length += 1;
}

void grd_append(GrdSmallString* res, const char* str) {
	auto length = strlen(str);
	for (auto i: grd_range(length)) {
		grd_append(res, str[i]);
	}
}

inline void grd_print_integer_number_to_char_buffer_reversed(auto num, int base, bool uppercase, char* buf, auto* cursor) {
	if (num == 0) {
		buf[*cursor] = '0';
		*cursor += 1;
	} else {
		while (num != 0) {
			s32 digit = abs(int(num % base));
			num /= base;

			if (digit > 9) {
				buf[*cursor] = (uppercase ? 'A' : 'a') + digit - 10;
			} else {
				buf[*cursor] = '0' + digit;
			}
			*cursor += 1;
		}
	}
}

inline void print_integer_number_to_char_buffer(auto number, int base, bool uppercase, char* buffer, auto* cursor) {
	auto start_position = *cursor;
	grd_print_integer_number_to_char_buffer_reversed(number, base, uppercase, buffer, cursor);
	grd_reverse(buffer + start_position, *cursor - start_position);
}

struct IntegerStringParams {
	int  base = 10;
	bool skip_base_prefix = false;
	bool uppercase = false;
};

template <typename T> requires (std::numeric_limits<T>::is_integer)
inline GrdSmallString grd_to_string(T num, IntegerStringParams p = {}) {
	static_assert(sizeof(T) <= 64 / 8);

	p.base = grd_clamp_s32(2, 16, p.base);
	if (p.base < 2 or p.base > 16) {
		p.base = 10;
	}

	GrdSmallString res;

	if (num < 0) {
		grd_append(&res, '-');
	}

	if (!p.skip_base_prefix) {
		switch (p.base) {
			case 2:  grd_append(&res, p.uppercase ? "0B" : "0b"); break;
			case 8:  grd_append(&res, "0"); break;
			case 16: grd_append(&res, p.uppercase ? "0X" : "0x"); break;
		}
	}
	print_integer_number_to_char_buffer(num, p.base, p.uppercase, res.buf, &res.length);
	return res;
}

template <typename T> requires (std::is_floating_point_v<T>)
inline GrdSmallString grd_to_string(T num, int max_decimal_digits = 99999999) {
	// Using implementation from:
	// https://blog.benoitblanchon.fr/lightweight-float-to-string/

	GrdSmallString res;

	if (num == 0) {
		if (signbit(num)) {
			grd_append(&res, "-0.0");
		} else {
			grd_append(&res, "0.0");
		}
		return res;
	} else if (isnan(num)) {
		if (signbit(num)) {
			grd_append(&res, "-NaN");
		} else {
			grd_append(&res, "NaN");
		}
		return res;
	} else if (num == INFINITY) {
		grd_append(&res, "Inf");
		return res;
	} else if (num == -INFINITY) {
		grd_append(&res, "-Inf");
		return res;
	}

	bool negative = false;

	if (num < 0) {
		negative = true;
		num = -num;
	}

	int exponent = 0;
	
	const f64 positive_exp_threshold = 1e7;
	const f64 negative_exp_threshold = 1e-5;
	if (num >= positive_exp_threshold) {
		if (num >= 1e256) {
	  		num /= 1e256;
	  		exponent += 256;
		}
		if (num >= 1e128) {
			num /= 1e128;
		 	exponent += 128;
		}
		if (num >= 1e64) {
			num /= 1e64;
			exponent += 64;
		}
		if (num >= 1e32) {
			num /= 1e32;
			exponent += 32;
		}
		if (num >= 1e16) {
			num /= 1e16;
			exponent += 16;
		}
		if (num >= 1e8) {
			num /= 1e8;
			exponent += 8;
		}
		if (num >= 1e4) {
			num /= 1e4;
			exponent += 4;
		}
		if (num >= 1e2) {
			num /= 1e2;
			exponent += 2;
		}
		if (num >= 1e1) {
			num /= 1e1;
		 	exponent += 1;
		}
	} else if (num <= negative_exp_threshold) {
		if (num < 1e-255) {
			num *= 1e256;
			exponent -= 256;
		}
		if (num < 1e-127) {
			num *= 1e128;
			exponent -= 128;
		}
		if (num < 1e-63) {
			num *= 1e64;
			exponent -= 64;
		}
		if (num < 1e-31) {
			num *= 1e32;
			exponent -= 32;
		}
		if (num < 1e-15) {
			num *= 1e16;
			exponent -= 16;
		}
		if (num < 1e-7) {
			num *= 1e8;
			exponent -= 8;
		}
		if (num < 1e-3) {
			num *= 1e4;
			exponent -= 4;
		}
		if (num < 1e-1) {
			num *= 1e2;
			exponent -= 2;
		}
		if (num < 1e0) {
		 	num *= 1e1;
			exponent -= 1;
		}
	}

	constexpr f64 decimal_part_extractor = 1e17;

	u64 integral_part = (u64) num;
	f64 remainder     = (num - integral_part);
	u64 decimal_part  = (u64) (remainder * decimal_part_extractor);


	// @TODO: maybe do rounding????

	// Doing everything in reverse order
	if (exponent != 0) {
		grd_print_integer_number_to_char_buffer_reversed(abs(exponent), 10, false, res.buf, &res.length);

		if (exponent < 0) {
			grd_append(&res, '-');
		}
		grd_append(&res, 'e');
	}

	// Print decimal part.
	if (max_decimal_digits > 0) {
		
		int start_position = res.length;
		int zeros_after_point = 0;

		if (decimal_part != 0) {
			f64 shit = decimal_part_extractor / f64(decimal_part);
			while (shit >= 10) {
				zeros_after_point += 1;
				shit /= 10;
			}
		}

		// Example.
		// This will turn 1234.5600000 to 1234.56
		while ((decimal_part != 0) && (decimal_part % 10 == 0)) {
			decimal_part /= 10;
		}

		grd_print_integer_number_to_char_buffer_reversed(decimal_part, 10, false, res.buf, &res.length);

		// Print zeros in zone marked by square brackets.  000.[000]323223
		for (auto i: grd_range(zeros_after_point)) {
			grd_append(&res, '0');
		}
		
		int decimal_digits_count = int(res.length) - start_position;
		if (decimal_digits_count > max_decimal_digits) {
			memmove(&res.buf[start_position], &res.buf[res.length - max_decimal_digits], max_decimal_digits);
			res.length = start_position + max_decimal_digits;
		}
		grd_append(&res, '.');
	}

	grd_print_integer_number_to_char_buffer_reversed(integral_part, 10, false, res.buf, &res.length);

	if (negative) {
		grd_append(&res, '-');
	}

	grd_reverse(res.buf, res.length);
	return res;
}

inline GrdSmallString grd_to_string(bool b) {
	GrdSmallString res;
	grd_append(&res, b ? "true" : "false");
	return res;
}



struct GrdParseIntegerParams {
	// normal prefix (as in OpenDDL) = '0o' or '0O'
	// c prefix = '0',
	bool use_c_octal_prefix = false;
	// Underscores can be placed between numbers for visual clarity.
	bool allow_underscores = true;
	s32* result_base = NULL;
	s32  base = 0;
};


// If number's base is not 10 than it's threated as unsigned.
template <typename T, GrdStringChar Char> requires (std::numeric_limits<T>::is_integer)
inline bool grd_parse_integer(GrdSpan<Char> str, T* result, GrdParseIntegerParams params = {}) {
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "Integer size is not supported");

	constexpr bool is_signed = std::numeric_limits<T>::is_signed;

	if (grd_len(str) <= 0) {
		return false;
	}

	using UnsignedT = typename std::make_unsigned<T>::type;

	int base = 10;
	int start = 0;
	int end = grd_len(str) - 1;

	if (params.base == 0) {
		if (grd_starts_with(str, "0b"_b) || grd_starts_with(str, "0B"_b)) {
			base   = 2;
			start += 2;
		} else if (grd_starts_with(str, "0x"_b) || grd_starts_with(str, "0X"_b)) {
			base   = 16;
			start += 2;
		} else {
			if (params.use_c_octal_prefix) {
				if (grd_starts_with(str, "0"_b)) {
					base   = 8;
					start += 1;
				}
			} else {
				if (grd_starts_with(str, "0o"_b) || grd_starts_with(str, "0O"_b)) {
					base   = 8;
					start += 2;
				}
			}
		}
	} else {
		base = params.base;
		if (base == 2) {
			if (grd_starts_with(str, "0b"_b) || grd_starts_with(str, "0B"_b)) {
				start += 2;
			}
		} else if (base == 8) {
			if (grd_starts_with(str, "0o"_b) || grd_starts_with(str, "0O"_b)) {
				start += 2;
			} else if (grd_starts_with(str, "0"_b)) {
				start += 1;
			}
		} else if (base == 16) {
			if (grd_starts_with(str, "0x"_b) || grd_starts_with(str, "0X"_b)) {
				start += 2;
			}
		}
	}

	// If base != 10, we parse unsigned version of a number
	//    and then bitcast it to a proper type.
	if (!is_signed || base != 10) {

		UnsignedT number = 0;
		UnsignedT limit;

#pragma push_macro("max")
#undef max
		if (base == 10) {
			limit = std::numeric_limits<T>::max();
		} else {
			limit = std::numeric_limits<UnsignedT>::max();
		}
#pragma pop_macro("max")


		UnsignedT exponent_limit = limit / base;
		UnsignedT exponent = 1;

		bool did_exponent_overflow = false;

		for (s32 i = end; i >= start; i--) {
			auto c = str[i];

			// Allow underscores only between digits and after base specifier
			//   (0b_0110 is valid).
			if (params.allow_underscores && i < end && i >= start) {
				if (c == '_') {
					continue;
				}
			}

			UnsignedT digit;
			if (c >= '0' && c <= '9') {
				digit = c - '0';
			} else if (c >= 'A' && c <= 'F') {
				digit = c - 'A' + 10;
			} else if (c >= 'a' && c <= 'f') {
				digit = c - 'a' + 10;
			} else {
				return false;
			}

			if (digit >= base) {
				return false;
			}

			if (digit != 0) {
				if (did_exponent_overflow) {
					return false;
				}
				
			#if 0
				// Why do we need this? And do we?
				if (limit / digit < exponent) return false;
			#endif

				UnsignedT new_number = number + digit * exponent;
				if (new_number < number) { 
					return false;
				}
				number = new_number;
			}

			bool exponent_overflow = exponent > exponent_limit;
			if (exponent > exponent_limit) {
				did_exponent_overflow = true;
			}
			exponent *= base;
		}

		*((UnsignedT*) result) = number;
		if (params.result_base) {
			*params.result_base = base;
		}
		return true;
	}

	bool negative = false;

	// Unsigned case should be handled in the code above.
	assert(std::is_signed_v<T>);

	if (str[0] == '-') {
		start   += 1;
		negative = true;
	}

	assert(base == 10);

#pragma push_macro("max")
#pragma push_macro("min")
#undef min
#undef max

	UnsignedT exponent = 1;
	UnsignedT limit;

	if (negative) {
		auto negative_limit = std::numeric_limits<T>::min();

		limit = grd_bitcast<UnsignedT>(negative_limit);
		limit = ~limit + 1; // Two's complement negation.
	} else {
		limit = (UnsignedT) std::numeric_limits<T>::max();
	}

#pragma pop_macro("max")
#pragma pop_macro("min")


	UnsignedT accumulator = 0;
	UnsignedT exponent_limit = limit / base;

	bool did_exponent_overflow = false;

	for (s32 i = end; i >= start; i--) {
		auto c = str[i];

		// Allow underscores only between digits, but not before or after them.
		if (params.allow_underscores && i < end && i > start) {
			if (c == '_') {
				continue;
			}
		}

		T digit = c - '0';

		if (digit < 0 || digit > 9) {
			return false;
		}

		if (digit != 0) {
			if (did_exponent_overflow) {
				return false;
			}

			UnsignedT new_number = accumulator + digit * exponent;
			if (new_number > limit || new_number < accumulator) {
				return false;
			}
			
			accumulator = new_number;
		}

		if (exponent > exponent_limit) {
			did_exponent_overflow = true;
		}

		exponent *= base;
	}

	T number = grd_bitcast<T>(accumulator);

	if (negative) {
		number = ~number + 1; // Two's complement negation.
	}

	*((T*) result) = number;

	if (params.result_base) {
		*params.result_base = base;
	}

	return true;
}


struct Float_Parsing_Params {
	bool allow_underscores = true;
};

template <GrdStringChar Char>
inline bool grd_parse_float(GrdSpan<Char> str, f64* result, Float_Parsing_Params params = {}) {
	if (grd_len(str) <= 0) {
		return false;
	}

	if (grd_compare_ignore_case_ascii(str, "-nan"_b)) {
		*result = -NAN;
		return true;
	} else if (grd_compare_ignore_case_ascii(str, "nan"_b)) {
		*result =  NAN;
		return true;
	} else if (grd_compare_ignore_case_ascii(str, "-inf"_b)) {
		*result = -INFINITY;
		return true;
	} else if (grd_compare_ignore_case_ascii(str, "inf"_b)) {
		*result =  INFINITY;
		return true;
	}

	s32  position = 0;
	bool negative = false;

	if (str[0] == '-') {
		negative  = true;
		position += 1;
	} else if (str[0] == '+') {
		position += 1;
	}

	s32 index_after_sign = position;
	s32 significand_part_start = index_after_sign;
	s32 e_position   = -1;

	while (position < grd_len(str)) {
		auto c = str[position];

		if (c == 'e' || c == 'E') {
			if (e_position == -1) {
				e_position = position;
			}
			else {
				return false;
			}
		}
		position += 1;
	}


	s32 significand_part_end = (e_position != -1) ? e_position : grd_len(str);
	s32 significand_length = significand_part_end - significand_part_start;
	GrdSpan<Char> significand_part = { str.data + significand_part_start, significand_length }; 

	s32 exponent = 0;
	f64 value;

	// Parse significand.
	{
		u8  digits[18];
		s32 digits_count = 0;

		bool did_meet_dot = false;

		for (auto i: grd_range(grd_len(significand_part))) {
			auto c = significand_part[i];

			auto add_digit = [&](u8 digit) -> bool {
				if (digits_count == grd_static_array_count(digits)) {
					return false;
				}

				digits[digits_count] = digit;
				digits_count += 1;
				return true;
			};

			if (c == '.') {
				did_meet_dot = true;
				continue;
			}

			if (c == '_') {
				if (params.allow_underscores) {
					if ((i > 0) && (i < grd_len(significand_part) - 1)) { 
						continue;
					}
				}

				return false;
			}

			if (!(c >= '0' && c <= '9')) {
				return false;
			}
			
			u8 digit = c - '0';

			if (digit == 0) {
				if (digits_count == 0) {
					if (did_meet_dot) {
						exponent -= 1;
					}
					continue;
				}
			}
			
			if (add_digit(digit)) {
				if (did_meet_dot) {
					exponent -= 1;
				}
			}
		}

		u64 significand = 0;
		u64 exp = 1;
		
		for (auto i: grd_reverse(grd_range(digits_count))) {
			u8 digit = digits[i];
			u64 new_number = significand + exp * digit;
			assert(new_number >= significand);
			significand = new_number;
			exp *= 10;
		}

		value = (f64) significand;
	}


	if (e_position != -1) {
		auto   exponent_string = str[e_position + 1, {}];
		s32    exp = 1;
		bool   is_exponent_negative = false;

		if (grd_len(exponent_string) > 0) { 
			if (exponent_string[0] == '-') {
				is_exponent_negative = true;
				exponent_string = exponent_string[1, {}];
			} else if (exponent_string[0] == '+') {
				exponent_string = exponent_string[1, {}];
			}
		}

		if (grd_len(exponent_string) == 0) {
			return false;
		}

		for (auto i: grd_reverse(grd_range(grd_len(exponent_string)))) {

			auto c = exponent_string[i];
			if (c == '_') {
				if (params.allow_underscores) {
					if ((i > 0) && (i < grd_len(exponent_string) - 1)) {
						continue;
					}
				}
				return false;
			}

			if (!(c >= '0' && c <= '9')) {
				return false;
			}

			s32 digit = c - '0';

 			if (is_exponent_negative) {
				s32 new_exponent = exponent - exp * digit;
 				if (new_exponent > exponent) {
 					exponent = s32_min;
 					continue;
 				}
	 			exponent = new_exponent;

 			} else {
				s32 new_exponent = exponent + exp * digit;
 				if (new_exponent < exponent) {
 					exponent = s32_max;
 					continue;
 				}
	 			exponent = new_exponent;
 			}

			exp *= 10;
		}
	}


	u64 bits = grd_bitcast<u64>(value);

	constexpr bool use_separate_number_for_exponent = false;
	f64 exp_number = use_separate_number_for_exponent ? 1.0 : value;

	if (exponent != 0) {
		s32 abs_exponent = abs(exponent);

		if (abs_exponent >= 256) {
			if (exponent > 0) exp_number *= 1e256;
			else              exp_number /= 1e256;
			abs_exponent -= 256;
		}
		if (abs_exponent >= 128) {
			if (exponent > 0) exp_number *= 1e128;
			else              exp_number /= 1e128;
			abs_exponent -= 128;
		}
		if (abs_exponent >= 64) {
			if (exponent > 0) exp_number *= 1e64;
			else              exp_number /= 1e64;
			abs_exponent -= 64;
		}
		if (abs_exponent >= 32) {
			if (exponent > 0) exp_number *= 1e32;
			else              exp_number /= 1e32;
			abs_exponent -= 32;
		}
		if (abs_exponent >= 16) {
			if (exponent > 0) exp_number *= 1e16;
			else              exp_number /= 1e16;
			abs_exponent -= 16;
		}
		if (abs_exponent >= 8) {
			if (exponent > 0) exp_number *= 1e8;
			else              exp_number /= 1e8;
			abs_exponent -= 8;
		}
		if (abs_exponent >= 4) {
			if (exponent > 0) exp_number *= 1e4;
			else              exp_number /= 1e4;
			abs_exponent -= 4;
		}
		if (abs_exponent >= 2) {
			if (exponent > 0) exp_number *= 1e2;
			else              exp_number /= 1e2;
			abs_exponent -= 2;
		}
		if (abs_exponent >= 1) {
			if (exponent > 0) exp_number *= 1e1;
			else              exp_number /= 1e1;
			abs_exponent -= 1;
		}
	}

	if (use_separate_number_for_exponent) {
		value = value * exp_number;
	} else {
		value = exp_number;
	}

	// Even if exponent is not zeroed by now, it's fine,
	//   because exponent already must be overflown. 
	//   and the returned number will be infinity in case of position exponent,
	//   and a subnormal number in case of negative exponent.

	if (negative) {
		value = -value;
	}

	*result = value;
	return true;
}

inline bool grd_parse_float(auto str, f32* result, Float_Parsing_Params params = {}) {
	f64 num;
	bool success = grd_parse_float(str, &num, params);
	if (success) {
		*result = num;
	}
	return success;
}
