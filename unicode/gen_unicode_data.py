import requests
import argparse
from pathlib import Path

DEFAULT_UNICODE_VERSION = "15.1.0"
UNICODE_DATA_TXT_URL = "https://www.unicode.org/Public/{version}/ucd/UnicodeData.txt"

MIRRORED_FIELD = 9
COMMENT_FIELD = 11
UPPERCASE_MAPPING_FIELD = 12
LOWERCASE_MAPPING_FIELD = 13
TITLECASE_MAPPING_FIELD = 14

class Deduplicator:
	def __init__(self):
		self.values = {}
	
	def get(self, key):
		if not key:
			return 0
		if key in self.values:
			return self.values[key]
		self.values[key] = max(self.values.values(), default=0) + 1
		return self.values[key]

names = Deduplicator()
categories = Deduplicator()
bidi_categories = Deduplicator()
old_names = Deduplicator()
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
		if is_fake:
			return
		self.name_id = names.get(og_fields[1])
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
		self.old_name_id = old_names.get(og_fields[10])
		# Field 11 (10646 comment) is ignored, because it's empty in all codepoints (as of Unicode 15).
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
		
		maybe_pack_field(self.name_id, 1)
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
		maybe_pack_field(self.old_name_id, 10)
		maybe_pack_field(self.uppercase, 12)
		maybe_pack_field(self.lowercase, 13)
		maybe_pack_field(self.titlecase, 14)

		nums[0] |= build_mask(CATEGORY_SHIFT, CATEGORY_SIZE) & (self.category_id << CATEGORY_SHIFT)
		nums[0] |= build_mask(BIDI_CATEGORY_SHIFT, BIDI_CATEGORY_SIZE) & (self.bidi_category_id << BIDI_CATEGORY_SHIFT)
		return nums

class UnicodeRange:
	def __init__(self):
		self.is_uniform_range = False
		self.codepoints = []

	def add_codepoint(self, cp):
		self.codepoints.append(cp)

def gen_unicode_data_txt_tables(version):
	unicode_dir = Path(__file__).parent

	r = requests.get(UNICODE_DATA_TXT_URL.format(version=version))
	try:
		r.raise_for_status()
	except Exception as e:
		raise Exception([Exception("Failed to fetch UnicodeData.txt"), e])

	with open(unicode_dir / "UnicodeData.txt", "wb") as f:
		f.write(r.content)

	r.encoding = 'utf-8'
	lines = r.text.splitlines()
	ranges = [UnicodeRange()]
	for i in range(len(lines)):
		fields = lines[i].split(';')
		codepoint = int(fields[0], base=16)
		if fields[1].endswith(", First>"):
			next_fields = lines[i+1].split(';')
			name = fields[1].removeprefix("<").removesuffix(", First>")
			ranges.append(UnicodeRange())
			ranges[-1].is_uniform_range = True
			ranges[-1].add_codepoint(Codepoint([fields[0],      name, *fields[2:]]))
			ranges[-1].add_codepoint(Codepoint([next_fields[0], name, *fields[2:]]))
		elif fields[1].endswith(", Last>"):
			pass
		else:
			if len(ranges[-1].codepoints) > 0 and ranges[-1].codepoints[-1].codepoint != codepoint - 1:
				if codepoint - ranges[-1].codepoints[-1].codepoint <= 2000:
					for it in range(ranges[-1].codepoints[-1].codepoint + 1, codepoint):
						ranges[-1].add_codepoint(Codepoint([hex(it)], is_fake=True))
				else:
					ranges.append(UnicodeRange())
			ranges[-1].add_codepoint(Codepoint(fields))


	PACK_CURSOR = 15
	CATEGORY_SHIFT = PACK_CURSOR
	CATEGORY_SIZE = max(categories.values.values()).bit_length()
	PACK_CURSOR += CATEGORY_SIZE
	BIDI_CATEGORY_SHIFT = PACK_CURSOR
	BIDI_CATEGORY_SIZE = max(bidi_categories.values.values()).bit_length()

	packed_cursor = 0
	packed_codepoints = []
	offset_into_packed = []
	total_codepoints_count = 0
	fake_codepoints_count = 0
	for it in ranges:
		it.offset_into_packed_offsets_table = len(offset_into_packed)
		for cp in it.codepoints:
			total_codepoints_count += 1
			if cp.is_fake:
				fake_codepoints_count += 1
				offset_into_packed.append(0xffffffff)
			else:
				offset_into_packed.append(packed_cursor)
				packed = cp.pack()
				packed_codepoints.append(packed)
				packed_cursor += len(packed)

	print("Total codepoints count: ", total_codepoints_count)
	print("Fake codepoints count: ", fake_codepoints_count)

	with open(unicode_dir / "generated_name_table.h", "w") as f:
		print("#pragma once", file=f)
		print("const const char* UNICODE_CODEPOINT_NAME_TABLE[] = {", file=f)
		print('\t"",', file=f)
		for k, v in sorted(names.values.items(), key=lambda it: it[1]):
			print(f'\t"{k}",', file=f)
		print("};", file=f)
		print("", file=f)
		print("const const char* UNICODE_CODEPOINT_OLD_NAME_TABLE[] = {", file=f)
		print('\t"",', file=f)
		for k, v in sorted(old_names.values.items(), key=lambda it: it[1]):
			print(f'\t"{k}",', file=f)
		print("};", file=f)

	with open(unicode_dir / "generated_data_enums.h", "w") as f:
		print("#pragma once", file=f)
		print("", file=f)
		print(f"const int UNICODE_CATEGORY_SIZE = {CATEGORY_SIZE};", file=f)
		print("", file=f)
		print("enum class UnicodeGeneralCategory {", file=f)
		for k, v in categories.values.items():
			print(f'\t{k} = {v},', file=f)
		print("};", file=f)
		print("", file=f)
		print("enum class UnicodeBidiCategory {", file=f)
		for k, v in bidi_categories.values.items():
			print(f'\t{k} = {v},', file=f)
		print("};", file=f)
		print("", file=f)
		print("enum class UnicodeDecompositionTag {", file=f)
		for k, v in decomposition_tags.values.items():
			print(f'\t{k.title()} = {v},', file=f)
		print("};", file=f)
		print("", file=f)
		print("const const char* UNICODE_DIGIT_VALUES[] = {", file=f)
		print('\t"",', file=f)
		for k, v in sorted(digits.values.items(), key=lambda it: it[1]):
			print(f'\t"{k}",', file=f)
		print("};", file=f)
		print("", file=f)
		print("const const char* UNICODE_NUMERIC_VALUES[] = {", file=f)
		print('\t"",', file=f)
		for k, v in sorted(numerics.values.items(), key=lambda it: it[1]):
			print(f'\t"{k}",', file=f)
		print("};", file=f)
		print("", file=f)

		
	with open(unicode_dir / "generated_codepoint_table.h", "w") as f:
		print("#pragma once", file=f)
		print('#include "unicode_table_types.h"', file=f)
		print("", file=f)
		print("const UnicodeCodepointRange UNICODE_NONUNIFORM_CODEPOINT_RANGES[] = {", file=f)
		for r in filter(lambda x: not x.is_uniform_range, ranges):
			assert len(r.codepoints) > 0
			print("\t{{ {0}, {1}, {2} }},".format(
				hex(r.codepoints[0].codepoint),
				hex(r.codepoints[-1].codepoint - r.codepoints[0].codepoint),
				r.offset_into_packed_offsets_table),
			file=f)
		print("};", file=f)
		print("", file=f)
		print("const UnicodeCodepointRange UNICODE_UNIFORM_CODEPOINT_RANGES[] = {", file=f)
		for r in filter(lambda x: x.is_uniform_range, ranges):
			assert len(r.codepoints) > 0
			print("\t{{ {0}, {1}, {2} }},".format(
				hex(r.codepoints[0].codepoint),
				hex(r.codepoints[-1].codepoint - r.codepoints[0].codepoint),
				r.offset_into_packed_offsets_table),
			file=f)
		print("};", file=f)
		print("", file=f)
		print("const int UNICODE_CODEPOINTS_OFFSETS_INTO_PACKED[] = {", file=f)
		for idx in range(len(offset_into_packed)):
			print(hex(offset_into_packed[idx]) + ',', file=f)
		print("};", file=f)
		print("const int UNICODE_PACKED_CODEPOINTS[] = {", file=f)
		for idx in range(len(packed_codepoints)):
			print(','.join(map(hex, packed_codepoints[idx])) + ',', file=f)
		print("};", file=f)

def main():
	argparser = argparse.ArgumentParser()
	argparser.add_argument("--version", default=DEFAULT_UNICODE_VERSION)
	args = argparser.parse_args()
	print("Unicode version:", args.version)
	gen_unicode_data_txt_tables(version=args.version)

if __name__ == "__main__":
	main()
