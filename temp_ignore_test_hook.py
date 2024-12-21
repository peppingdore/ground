import re
from pathlib import Path

def grd_test_hook(tester):
	@tester.add_file_filter
	def file_filter(path: Path):
		if re.search('grd_unicode_data_test.cpp|grd_regexp.h|prep_test.cpp|grd_crash_cases_test.cpp', str(path)):
			return True
