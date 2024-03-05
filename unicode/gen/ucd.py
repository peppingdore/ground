class PropertyRange:
	def __init__(self, fields):
		range_fields = fields[0].split('..')
		if len(range_fields) == 2:
			self.start = int(range_fields[0], base=16)
			self.end   = int(range_fields[1], base=16) 
		else:
			self.start = int(range_fields[0], base=16)
			self.end   = int(range_fields[0], base=16)
		self.props = fields[1:]

def merge_ranges(ranges):
	i = 0
	while i < len(ranges) - 1:
		if ranges[i].end == ranges[i + 1].start - 1 and ranges[i].props == ranges[i + 1].props:
			ranges[i].end = ranges[i + 1].end
			del ranges[i + 1]
		else:
			i += 1

def parse(text):
	lines = text.splitlines()
	fields = filter(lambda x: x[0] != '', map(lambda x: list(map(lambda y: y.strip(), x.split('#')[0].split(';'))), lines))
	ranges = list(map(PropertyRange, fields))
	merge_ranges(ranges)
	return ranges
