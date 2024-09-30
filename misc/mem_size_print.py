def sizeof_fmt(num, suffix="B"):
    for unit in ("", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"):
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    return f"{num:.1f}Yi{suffix}"

CUTOFF = 4096

def calc_size_binary_buddy(size, depth):
	if size < CUTOFF:
		return (depth - 1, { }, 0)
	a = calc_size_binary_buddy(size // 2, depth + 1)
	return (max(a[0], depth), { size: True } | a[1], pow(2, depth) + a[2])

def calc_size_xx_buddy(size, depth):
	if size < CUTOFF:
		return (depth - 1, { }, 1)
	a = calc_size_xx_buddy(size // 3 * 2, depth + 1)
	b = calc_size_xx_buddy(size - (size // 3 * 2), depth + 1)
	return (max(a[0], b[0], depth), { size: True } | a[1] | b[1], a[2] + b[2])

def calc_size_quad_buddy(size, depth):
	if size < CUTOFF:
		return (depth - 1, { }, 0)
	a = calc_size_quad_buddy(size // 4, depth + 1)
	return (max(a[0], depth), { size: True } | a[1], pow(4, depth) + a[2])

for sz in [4 * 1024, 8 * 1024, 16 * 1024, 32 * 1024, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024, 2 * 1024 * 1024,
		   4 * 1024 * 1024, 8 * 1024 * 1024, 16 * 1024 * 1024, 32 * 1024 * 1024, 64 * 1024 * 1024, 128 * 1024 * 1024, 256 * 1024 * 1024,
		   512 * 1024 * 1024, 1024 * 1024 * 1024, 2 * 1024 * 1024 * 1024, 32 * 1024 * 1024 * 1024
		   ]:
	a = calc_size_binary_buddy(sz, 1)
	b = calc_size_xx_buddy(sz, 1)
	c = calc_size_quad_buddy(sz, 1)

	print("Size: ", sizeof_fmt(sz))
	print()
	print("Binary buddy")
	# for size in a[1]:
	# 	print(size)
	print("   Sizes: ", len(a[1]))
	print("   Max depth: ", a[0])
	print()
	sizes = list(a[1].keys())
	sizes.sort()
	print("Binary bottom sizes: ", sizes[:5])
	print("Path bits: ", sizeof_fmt(a[2]))
	print("XX buddy")
	# for size in b[1]:
	# 	print(size)
	print("   Sizes: ", len(b[1]))
	print("   Max depth: ", b[0])
	print()
	sizes = list(b[1].keys())
	sizes.sort()
	print("XX bottom sizes: ", sizes[:5])
	print("Path bits: ", sizeof_fmt(b[2]))
	print()
	print("Quad buddy")
	# for size in c[1]:
	# 	print(size)
	print("   Sizes: ", len(c[1]))
	print("   Max depth: ", c[0])
	print()
	sizes = list(c[1].keys())
	sizes.sort()
	print("Quad bottom sizes: ", sizes[:5])
	print("Path bits: ", sizeof_fmt(c[2]))
	print()