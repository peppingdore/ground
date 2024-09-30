def calc(sz, min_block):
	total_sz = sz
	bits = 2
	while sz >= min_block:
		num = total_sz // sz
		# print("Total sz: {}, sz: {}, num: {}, bits: {}".format(total_sz, sz, num, 2 * num))
		bits += 2 * (num)
		sz //= 2
	return bits

def test(sz, min_block):
	import math
	header_len = math.ceil(calc(sz, min_block) / 8)
	print("Size: {}, Min block: {}, Header len: {}, blocks: {}, waste: {}".format(sz, min_block, header_len, sz / min_block, header_len / sz))

test(4096, 128)
test(4096, 64)
test(4096, 32)
test(8192, 128)
test(8192, 64)
test(8192, 32)
test(8192 * 2, 32)
test(8192 * 4, 32)
test(8192 * 8, 32)
test(8192 * 16, 32)
test(8192 * 32, 32)

test(8192 * 2, 64)
test(8192 * 4, 64)
test(8192 * 8, 64)
test(8192 * 16, 64)
test(8192 * 32, 64)

test(8192 * 32, 512)
test(32 * 1024 * 1024 * 1024, 4096)
test(32 * 1024 * 1024 * 1024, 8192)
# Terabyte
test(1024 * 1024 * 1024 * 1024, 8192)
