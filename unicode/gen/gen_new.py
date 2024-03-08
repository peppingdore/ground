#!/usr/bin/env python3

import requests
from pathlib import Path
from deduplicator import Deduplicator
import ucd
import replace

categories = Deduplicator(file=Path(__file__).parent / "category_values.txt")
bidi_categories = Deduplicator(file=Path(__file__).parent / "bidi_categories.txt")
decomposition_tags = Deduplicator()
digits = Deduplicator()
numerics = Deduplicator()

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

def new_gen(*, version):
	r = requests.get("https://www.unicode.org/Public/{version}/ucd/UnicodeData.txt".format(version=version))
	r.raise_for_status()
	codepoints = {}
	uniform_ranges = []

	lines = r.text.splitlines()
	for i in range(len(lines)):
		fields = lines[i].split(';')
		if fields[1].endswith(", First>"):
			next_fields = lines[i+1].split(';')
			name = fields[1].removeprefix("<").removesuffix(", First>")
			uniform_ranges.append((int(fields[0], base=16), int(next_fields[0], base=16), [name, *fields[2:]]))
		else:
			cp = Codepoint(fields)
			codepoints[cp.codepoint] = cp
	
	r = requests.get("https://www.unicode.org/Public/{version}/ucd/Scripts.txt".format(version=version))
	r.raise_for_status()
	script_ranges = ucd.parse(r.text)
	script_ranges_c = []
	scripts = Deduplicator(file=Path(__file__).parent / "script_values.txt")
	for rang in sorted(script_ranges, key=lambda it: it.start):
		scripts.get(rang.props[0]) # Check whether script is valid.
		script_ranges_c.append(f'{{ {hex(rang.start)}, {hex(rang.end)}, UnicodeScript::{rang.props[0]} }}')
	script_values_c = []
	for k, v in scripts.values.items():
		script_values_c.append(f"\t{k} = {v}")
	replace.replace(Path(__file__).parent / "scripts.h", script_ranges=',\n'.join(script_ranges_c), script_codes=',\n'.join(script_values_c))

new_gen(version="15.0.0")
