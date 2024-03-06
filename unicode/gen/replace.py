from pathlib import Path
import re

# def replace(path, **kwargs):
# 	txt = Path(path).read_text()
# 	for cursor in range(len(txt)):
# 		start = txt.find("// %-", cursor)
# 		if start == -1:
# 			break
# 		name = txt[start + len("// %-"):].split('\n', maxsplit=1)
# 		start_after = start + len("// %-") + len(name) + 1
# 		if not name in kwargs:
# 			raise Exception(f"// %-{name} to replace is not found in **kwargs")
# 		end = txt.find("// %-end")
# 		if end == -1:
# 			raise Exception(f"Failed to find // %-end for // %-{name}")
# 		txt = txt[:start_after] + kwargs[name] + txt[end:]

def replace(path, **kwargs):
	txt = Path(path).read_text()
	for k, v in kwargs.items():
		txt = re.sub(f"(^\/\/ %-{k}\s*$\s)^(?s:.*)(^\/\/ %-end\s*$)", f"\\1{v}\n\\2", txt, flags=re.MULTILINE)
	Path(path).write_text(txt, encoding='utf-8')
