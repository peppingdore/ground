import requests
import argparse
from pathlib import Path

DEFAULT_UNICODE_VERSION = "15.1.0"
UNICODE_DATA_TXT_URL = "https://www.unicode.org/Public/{version}/ucd/UnicodeData.txt"

def iterate_codepoints(chars, *, iterate_gaps=False):
	lastcodepoint = -1
	for i in range(len(chars)):
		if iterate_gaps:
			for codepoint in range(lastcodepoint, chars[i][0]):
				yield [codepoint]
		lastcodepoint = chars[i][0]
		if chars[i][1].endswith(", First>"):
			if len(chars) <= i + 1:
				raise Exception("', First>' row is not succeeded by next row")
			if not chars[i + 1][1].endswith(", Last>"):
				raise Exception("', First>' row is not succeeded by next row, that's name ends with ', Last>'")
			for codepoint in range(chars[i][0], chars[i + 1][0] + 1):
				name = chars[i][1].removeprefix("<").removesuffix(", First>")
				yield [codepoint, name, *chars[i][2:]]
			continue
		if chars[i][1].endswith(", Last>"):
			continue
		yield chars[i]

def gen_unicode_data_txt_tables(version):
	r = requests.get(UNICODE_DATA_TXT_URL.format(version=version))
	try:
		r.raise_for_status()
	except Exception as e:
		raise Exception([Exception("Failed to fetch UnicodeData.txt"), e])

	r.encoding = 'utf-8'
	lines = r.text.splitlines()

	chars = []
	for line in lines:
		fields = line.split(";")
		if len(fields) != 15:
			raise Exception(f"Count of fields in line must be 15.\nLine: {line}")
		fields[0] = int(fields[0], base=16)
		chars.append(fields)

	ranges = []
	ranges.append([0, 0, 0])

	with open(Path(__file__).parent / "name_table.h", "w") as f:
		print("#pragma once", file=f)
		print("const char* unicode_name_table[] = {", file=f)
		name_cursor = 0
		lastcodepoint = None
		for char in iterate_codepoints(chars):
			if char[0] - 1 != lastcodepoint:
				ranges.append([char[0], char[0], name_cursor])
			ranges[-1][1] = char[0]
			print(f'"{char[1]}",', file=f)
			name_cursor += 1
			lastcodepoint = char[0]
		print('""', file=f)
		print("};", file=f)

def main():
	argparser = argparse.ArgumentParser()
	argparser.add_argument("--version", default=DEFAULT_UNICODE_VERSION)
	args = argparser.parse_args()
	print("Unicode version: ", args.version)
	gen_unicode_data_txt_tables(version=args.version)

if __name__ == "__main__":
	main()
