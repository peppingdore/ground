class Deduplicator:
	def __init__(self, *, file=None):
		self.values = {}
		with open(file, "r") as f:
			lines = f.readlines()
			for it in lines:
				if len(fields) != 2:
					continue
				fields = it.split()
				self.values[fields[0]] = int(fields[1])
		self.file = file # If file is not None, we can't add new values to Deduplicator.
	
	def get(self, key):
		if key in self.values:
			return self.values[key]
		if self.file:
			raise Exception(f"Deduplicator doesn't have value: {key}")
		if not key:
			return 0
		self.values[key] = max(self.values.values(), default=0) + 1
		return self.values[key]
