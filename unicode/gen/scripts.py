import ucd
import requests
from deduplicator import *
from pathlib import Path

UNICODE_SCRIPTS_TXT_URL = "https://www.unicode.org/Public/{version}/ucd/Scripts.txt"

def generate(version):
	r = requests.get(UNICODE_SCRIPTS_TXT_URL.format(version=version))
	try:
		r.raise_for_status()
	except Exception as e:
		raise Exception([Exception("Failed to fetch Scripts.txt"), e])
	
	with open(Path(__file__).parent / "Scripts.txt", "wb") as f:
		f.write(r.content)

	scripts = Deduplicator(file=Path(__file__).parent / )
	ranges = ucd.parse(r.text)
	for it in ranges:
		scripts.get(it.props[0])
	with open(Path(__file__).parent / "generated" / "scripts.h") as f:
		print("#pragma once", file=f)
		print('#include "../unicode_table_types.h"', file=f)
		print("", file=f)
		print("enum class UnicodeScript {", file=f)
		for k, v in scripts.values:
			print(f"\t{k} = {v},", file=f)
		print("};", file=f)
		print('''''', file=f)
		print("const UnicodeSimpleRange UNICODE_WHITE_SPACES_RANGES[] = {", file=f)
		for it in ranges:
			print(f'\t{{ { it.start }, { it.end } }},')
		print("};", file=f)

	for it in ranges:
		print(hex(it.start), hex(it.end), it.props, it.end - it.start + 1)
