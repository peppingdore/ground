#!/usr/bin/env python3
import requests
from pathlib import Path
from deduplicator import *
import replace

categories = Deduplicator(file=Path(__file__).parent / "category_values.txt")
bidi_categories = Deduplicator(file=Path(__file__).parent / "bidi_categories.txt")
decomposition_tags = Deduplicator()
digits = Deduplicator()
numerics = Deduplicator()

CATEGORY_SIZE = 0
CATEGORY_SHIFT = 0
BIDI_CATEGORY_SIZE = 0
BIDI_CATEGORY_SHIFT = 0

def build_mask(first_bit, size):
	val = 0
	for i in range(first_bit, first_bit + size):
		val |= (1 << i)
	return val

class Codepoint:
	def __init__(self, og_fields, *, is_fake=False):
		self.is_fake = is_fake
		self.og_fields = og_fields
		self.codepoint = int(og_fields[0], base=16)
		self.name = ""
		if is_fake:
			return
		self.name = og_fields[1]
		self.category_id = categories.get(og_fields[2])
		self.combining_class = int(og_fields[3]) if og_fields[3] else None
		self.bidi_category_id = bidi_categories.get(og_fields[4])
		self.decomposition_mapping = []
		mapping_fields = og_fields[5].split()
		for it in mapping_fields:
			self.decomposition_mapping.append(len(mapping_fields))
			if it.startswith("<") and it.endswith(">"):
				tag = it.removeprefix("<").removesuffix(">")
				self.decomposition_mapping.append(0xffffffff-decomposition_tags.get(tag))
			else:
				self.decomposition_mapping.append(int(it, base=16))
		self.decimal_digit = int(og_fields[6]) if og_fields[6] else None
		self.digit = digits.get(og_fields[7])
		self.numeric = numerics.get(og_fields[8])
		self.mirrored = og_fields[9] == "Y"
		self.uppercase = int(og_fields[12], base=16) if og_fields[12] else None
		self.lowercase = int(og_fields[13], base=16) if og_fields[13] else None
		self.titlecase = int(og_fields[14], base=16) if og_fields[14] else None

	def pack(self):
		assert not self.is_fake
		# First number is flags, usually representing fields that we pack.
		#  Except in the case of the field 'mirrored' flag represents whether a codepoint is mirrored.
		nums = [0]
		def maybe_pack_field(value, index):
			if value:
				nums[0] |= 1 << index
				nums.append(value)
		
		if self.combining_class:
			nums[0] |= 1 << 3
			nums.append(self.combining_class)
		if self.decomposition_mapping:
			nums[0] |= 1 << 5
			nums.extend(self.decomposition_mapping)
		maybe_pack_field(self.decimal_digit, 6)
		maybe_pack_field(self.digit, 7)
		maybe_pack_field(self.numeric, 8)
		if self.mirrored:
			nums[0] |= 1 << 9
		maybe_pack_field(self.uppercase, 12)
		maybe_pack_field(self.lowercase, 13)
		maybe_pack_field(self.titlecase, 14)

		nums[0] |= build_mask(CATEGORY_SHIFT, CATEGORY_SIZE) & (self.category_id << CATEGORY_SHIFT)
		nums[0] |= build_mask(BIDI_CATEGORY_SHIFT, BIDI_CATEGORY_SIZE) & (self.bidi_category_id << BIDI_CATEGORY_SHIFT)
		return nums

class UnicodeRange:
	def __init__(self):
		self.codepoints = []

class UnicodeUniformRange:
	def __init__(self, start, end, codepoint):
		self.start = start
		self.end = end
		self.codepoint = codepoint

def generate(*, version):
	r = requests.get(f"https://www.unicode.org/Public/{version}/ucd/UnicodeData.txt")
	r.raise_for_status()
	lines = r.text.splitlines()
	ranges = [UnicodeRange()]
	uniform_ranges = []
	for i in range(len(lines)):
		fields = lines[i].split(';')
		codepoint = int(fields[0], base=16)
		if fields[1].endswith(", First>"):
			next_fields = lines[i+1].split(';')
			fields[1] = fields[1].removeprefix("<").removesuffix(", First>")
			uniform_ranges.append(UnicodeUniformRange(int(fields[0], 16), int(next_fields[0], 16), Codepoint(fields)))
		elif fields[1].endswith(", Last>"):
			pass
		else:
			if len(ranges[-1].codepoints) > 0 and ranges[-1].codepoints[-1].codepoint != codepoint - 1:
				if codepoint - ranges[-1].codepoints[-1].codepoint <= 2000:
					for it in range(ranges[-1].codepoints[-1].codepoint + 1, codepoint):
						ranges[-1].codepoints.append(Codepoint([hex(it)], is_fake=True))
				else:
					ranges.append(UnicodeRange())
			ranges[-1].codepoints.append(Codepoint(fields))


	PACK_CURSOR = 15
	CATEGORY_SHIFT = PACK_CURSOR
	CATEGORY_SIZE = max(categories.values.values()).bit_length()
	PACK_CURSOR += CATEGORY_SIZE
	BIDI_CATEGORY_SHIFT = PACK_CURSOR
	BIDI_CATEGORY_SIZE = max(bidi_categories.values.values()).bit_length()

	names = []
	packed_cursor = 0
	packed_codepoints = []
	offset_into_packed = []

	def pack_codepoint(cp):
		nonlocal packed_cursor
		if cp.is_fake:
			xxxx = -1
		else:
			packed = cp.pack()
			if packed[0] == 0:
				xxxx = -2
			else:
				xxxx = packed_cursor
				packed_codepoints.append(packed)
				packed_cursor += len(packed)
		offset_into_packed.append(xxxx)
		names.append(cp.name)
	
	c_ranges = []
	for it in ranges:
		c_ranges.append(f"{{ {hex(it.codepoints[0].codepoint)}, {hex(it.codepoints[-1].codepoint)}, {len(offset_into_packed)} }}")
		for cp in it.codepoints:
			pack_codepoint(cp)
	c_uniform_ranges = []
	for it in uniform_ranges:
		c_uniform_ranges.append(f"{{ {hex(it.codepoint.codepoint)}, {hex(it.codepoint.codepoint)}, {len(offset_into_packed)} }}")
		pack_codepoint(it.codepoint)

	c_packed_codepoints = []
	for it in packed_codepoints:
		c_packed_codepoints.append(','.join(map(hex, it)))

	def prepare_string_array(x):
		x = sorted(x.values.items(), key=lambda it: it[1])
		x = list(map(lambda it: f'"{it[0]}"', x))
		return ['""', *x]

	replace.replace(Path(__file__).parent / "unicode_data_names.h", codepoint_names=',\n'.join(map(lambda it: f'"{it}"', names)))
	replace.replace(Path(__file__).parent / "unicode_data.h",
		category_mask=hex(build_mask(CATEGORY_SHIFT, CATEGORY_SIZE)),
		category_shift=CATEGORY_SHIFT,
		bidi_category_mask=hex(build_mask(BIDI_CATEGORY_SHIFT, BIDI_CATEGORY_SIZE)),
		bidi_category_shift=BIDI_CATEGORY_SHIFT,
		digit_values=  ',\n'.join(prepare_string_array(digits)),
		numeric_values=',\n'.join(prepare_string_array(numerics)),
		non_uniform_ranges=',\n'.join(c_ranges),
		uniform_ranges=',\n'.join(c_uniform_ranges),
		offsets_into_packed=',\n'.join(map(hex, offset_into_packed)),
		packed_codepoints=',\n'.join(c_packed_codepoints)
	)

if __name__ == "__main__":
	generate(version="15.0.0")
