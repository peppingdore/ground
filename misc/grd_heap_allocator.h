#pragma once

#include "grd_allocator.h"

struct BuddyHeader {
	// Must be divisible by 2, so that leaves last bit usable
	//   for indicating whether buddy is is divided or not.
	//
	// If level is 0, then flags & ~1 is size of the block.
	u64          flags;
};

u64 size(BuddyHeader* header) {
	return header->flags & ~1ull;
}

bool is_divided(BuddyHeader* header) {
	return (header->flags & 1ull) == 1;
}

struct BuddyAllocTrieNode {
	u64                 num = 0;
	BuddyAllocTrieNode* sibling = NULL;
	BuddyAllocTrieNode* child   = NULL;
};

struct BuddyGlobalHeader {
	GrdAllocator          parent;
	u64                size = 0;
	u64                metadata_size = 0;
	BuddyAllocTrieNode trie_first;
	BuddyHeader        first_header;
	char               metadata_start;
};

constexpr s64 BUDDY_SIZE_CLASSES_COUNT = 55;

constexpr s64 BUDDY_SIZE_CLASSES[] = {
	16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256,
	288, 320, 352, 384, 416, 448, 480, 512, 576, 640, 704, 768, 832, 896, 960,
	1024, 1152, 1280, 1408, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328,
	3648, 4096, 4480, 4992, 5504, 6016, 6528, 7040, 7680, 8224, 8832, 9472,
};
static_assert(grd_static_array_count(BUDDY_SIZE_CLASSES) == 55);

constexpr s64 BUDDY_MIN_BLOCK_SIZE = 4096 * 8;

s64 map_size_to_class(u64 size) {
	for (auto i: grd_range(BUDDY_SIZE_CLASSES_COUNT)) {
		if (size <= BUDDY_SIZE_CLASSES[i]) {
			return i;
		}
	}
	return -1;
}

void* buddy_alloc_raw(BuddyHeader* header, u64 size, s64 order) {
	if (is_divided(header)) {
		
	}
}

void* buddy_alloc(BuddyHeader* header, u64 size, s64 order) {
	auto class_idx = map_size_to_class(size);
	if (class_idx != -1) {
		
	}
	return buddy_alloc_raw(header, size, order);
}

BuddyGlobalHeader* init(GrdAllocator parent, u64 size) {
	if (size < 512 * 1024 * 1024) {
		size = 512 * 1024 * 1024;
	}
	auto header = (BuddyGlobalHeader*) GrdMalloc(parent, size);
	*header = {};
	header->parent = parent;
	header->size = size;
	header->metadata_size = 4 * 1024;
}
