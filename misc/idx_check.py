def get_block_bits_idx(depth, idx):
	layer_start = 0
	for i in range(depth):
		layer_start += 1 << i
	return layer_start + idx

print(get_block_bits_idx(0, 0))
print(get_block_bits_idx(1, 0))
print(get_block_bits_idx(2, 0))
print(get_block_bits_idx(3, 0))

