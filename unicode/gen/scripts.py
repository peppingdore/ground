import ucd
import requests
from deduplicator import *
from replace import replace
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

	scripts = Deduplicator(file=Path(__file__).parent / "script_values.txt")
	ranges = ucd.parse(r.text)
	for it in ranges:
		scripts.get(it.props[0])
	
	lines = []
	for k, v in scripts.values.items():
		lines.append(f"\t{k} = {v},")
	
	replace(Path(__file__).parent / "scripts.h", script_codes='\n'.join(lines))

	# for it in ranges:
	# 	print(hex(it.start), hex(it.end), it.props, it.end - it.start + 1)
