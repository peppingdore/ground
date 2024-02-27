import requests
import argparse
from pathlib import Path

DEFAULT_UNICODE_VERSION = "15.1.0"
UNICODE_DATA_TXT_URL = "https://www.unicode.org/Public/{version}/ucd/UnicodeData.txt"

MIRRORED_FIELD_INDEX = 9

class Deduplicator:
	def __init__(self):
		self.values = {}
	
	def get(self, key):
		if key in self.values:
			return self.values[key]
		self.values[key] = max(self.values.values(), default=0) + 1
		return self.values[key]

deduplicators = [Deduplicator() for i in range(15)]

class Codepoint:
	def __init__(self, og_fields):
		self.og_fields = og_fields
		if len(og_fields) != 15:
			raise Exception(f"Count of fields in line must be 15.\nLine: {og_fields}")
		self.fields     = [None for i in range(15)]
		self.fields[0]  = int(og_fields[0], base=16)
		for i in range(1, 15):
			self.fields[i] = deduplicators[i].get(og_fields[i])

	def pack(self):
		nums = [0]
		for i in range(1, len(self.fields)):
			if i == MIRRORED_FIELD_INDEX:
				if self.og_fields[MIRRORED_FIELD_INDEX] == "Y":
					nums[0] |= 1 << MIRRORED_FIELD_INDEX
				continue
			if self.og_fields[i]:
				nums[0] |= 1 << i
				nums.append(self.fields[i])
		return nums

class UnicodeRange:
	def __init__(self):
		self.is_uniform_range = False
		self.codepoints = []

	def add_codepoint(self, cp):
		self.codepoints.append(cp)

def gen_unicode_data_txt_tables(version):
	r = requests.get(UNICODE_DATA_TXT_URL.format(version=version))
	try:
		r.raise_for_status()
	except Exception as e:
		raise Exception([Exception("Failed to fetch UnicodeData.txt"), e])

	r.encoding = 'utf-8'
	lines = r.text.splitlines()
	lastcodepoint = -1
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
			if lastcodepoint != codepoint - 1:
				ranges.append(UnicodeRange())
			ranges[-1].add_codepoint(Codepoint(fields))
		lastcodepoint = codepoint

	with open(Path(__file__).parent / "unicode_data.h", "w") as f:
		print(
'''
#pragma once
struct UnicodeRange {
	int  first_codepoint;
	int  last_codepoint;
	int  codepoint_table_offset;
	bool is_uniform_range;
};
''', file=f)

		packed_cursor = 0
		packed_codepoints = []
		offset_into_packed = []
		for it in ranges:
			it.offset_into_packed_offsets_table = len(offset_into_packed)
			for cp in it.codepoints:
				offset_into_packed.append(packed_cursor)
				packed = cp.pack()
				packed_codepoints.append(packed)
				packed_cursor += len(packed)

		print("const UnicodeRange UNICODE_CODEPOINTS_RANGES[] = {", file=f)
		for idx in range(len(ranges)):
			r = ranges[idx]
			if len(r.codepoints) == 0:
				print("skipped range: ", r.codepoints, idx, len(ranges))
				print(vars(ranges[0]))
				print(vars(ranges[1]))
				print(vars(ranges[2]))
				print(vars(ranges[3]))
				continue
			print("UnicodeRange {{ {0}, {1}, {2}, {3} }}".format(
				hex(r.codepoints[0].fields[0]),
				hex(r.codepoints[-1].fields[0]),
				r.offset_into_packed_offsets_table,
				str(r.is_uniform_range).lower()) + (',' if idx != len(ranges) else ''),
			file=f)
		print("};", file=f)

		for i in range(1, 15):
			if i == MIRRORED_FIELD_INDEX: continue
			print(f"const const char* UNICODE_DEDUPLICATED_FIELD_{i}[] = {{", file=f)
			s = sorted(deduplicators[i].values.items(), key=lambda x: x[1])
			for idx in range(len(s)):
				print(f'"{s[idx][0]}"' + (',' if idx != len(s) else ''), file=f)
			print("};", file=f)

		print("const int UNICODE_CODEPOINTS_OFFSETS_INTO_PACKED[] = {", file=f)
		for idx in range(len(offset_into_packed)):
			print(hex(offset_into_packed[idx]) + (',' if idx != len(offset_into_packed) else ''), file=f)
		print("};", file=f)

		print("const int UNICODE_PACKED_CODEPOINTS[] = {", file=f)
		for idx in range(len(packed_codepoints)):
			print(','.join(map(hex, packed_codepoints[idx])) + (',' if idx != len(packed_codepoints) else ''), file=f)
		print("};", file=f)


def batch(iterable, n):
    l = len(iterable)
    for ndx in range(0, l, n):
        yield iterable[ndx:min(ndx + n, l)]

def main():
	argparser = argparse.ArgumentParser()
	argparser.add_argument("--version", default=DEFAULT_UNICODE_VERSION)
	args = argparser.parse_args()
	print("Unicode version: ", args.version)
	gen_unicode_data_txt_tables(version=args.version)

if __name__ == "__main__":
	main()
