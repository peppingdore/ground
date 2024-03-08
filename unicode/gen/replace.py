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
		txt = re.sub(f"(\/\* %-{k} \*\/)(?s:.*?)(\/\* %-end \*\/)", f"\\1 {v} \\2", txt, flags=re.MULTILINE)
	Path(path).write_text(txt, encoding='utf-8')
