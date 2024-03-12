import ucd
import requests
from deduplicator import *
from replace import replace
from pathlib import Path
import unicode_version

def generate(version=unicode_version.UNICODE_VERSION):
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
	replace(Path(__file__).parent / "scripts.h", script_ranges=',\n'.join(script_ranges_c), script_codes=',\n'.join(script_values_c))
