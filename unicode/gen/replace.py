from pathlib import Path
import re

# /* %-{X} */
# {N}
# /* %-end */
#
# Where X is kwarg key, and N is the value of kwarg.
# All the text between opening and closing lines gets replaced by N.
def replace(path, **kwargs):
	txt = Path(path).read_text()
	for k, v in kwargs.items():
		match = re.search(f"(\/\* %-{k} \*\/)(?s:.*?)(\/\* %-end \*\/)", txt, flags=re.MULTILINE)
		pad = ''
		if match:
			if '\n' in txt[match.start():match.end()]:
				pad = '\n'
			txt = txt[:match.start()] + f"{match.group(1)}{pad}{v}{pad}{match.group(2)}" + txt[match.end():]
	Path(path).write_text(txt, encoding='utf-8')
