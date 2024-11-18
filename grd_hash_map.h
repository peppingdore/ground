#pragma once

#include "grd_hash.h"
#include "grd_allocator.h"
#include "grd_reflect.h"

constexpr GrdHash64 HASH_MAP_HASH_EMPTY = 0;
constexpr GrdHash64 HASH_MAP_SLOT_HASH_FIRST_OCCUPIED = 1;

template <typename K, typename V>
struct GrdHashMapEntry {
	K         key;
	V         value;
	GrdHash64 normalized_hash;

	bool is_occupied() {
		return normalized_hash != HASH_MAP_HASH_EMPTY;
	}
};

template <typename K, typename V>
struct GrdHashMap {
	using Entry = GrdHashMapEntry<K, V>;

	GrdAllocator allocator   = c_allocator;
	Entry*       data        = NULL;
	s64          capacity    = 0;
	s64          count       = 0;
	f32          load_factor = 0.75;
	GrdCodeLoc loc = grd_caller_loc();

	void free() {
		if (data) {
			GrdFree(allocator, data, loc);
			data  = NULL;
		}
		*this = {};
	}

	GrdGenerator<Entry*> iterate() {
		for (auto i: grd_range(capacity)) {
			auto* e = &data[i];
			if (e->is_occupied()) {
				co_yield e;
			}
		}
	}
};

template <typename K, typename V>
s64 grd_hash_map_get_home(GrdHashMap<K, V>* map, GrdHashMapEntry<K, V>* e) {
	assert(map->capacity > 0);
	return e->normalized_hash % map->capacity;
}

GrdHash64 grd_hash_map_normalize_hash(GrdHash64 hash) {
	if (hash < HASH_MAP_SLOT_HASH_FIRST_OCCUPIED) {
		return HASH_MAP_SLOT_HASH_FIRST_OCCUPIED;
	}
	return hash;
}

template <typename K>
GrdHash64 grd_hash_key(K key) {
	auto h = grd_hash64(key);
	return grd_hash_map_normalize_hash(h);
}

s64 grd_hash_map_distance_from_home(s64 home, s64 slot, s64 capacity) {
	if (slot < home) {
		return slot + (capacity - home);
	}
	return slot - home;
}

s64 grd_hash_map_next_index_wraparound(s64 idx, s64 capacity) {
	return (idx + 1) % capacity;
}

template <typename K, typename V>
void grd_hash_map_clear_entries_hashes(GrdHashMapEntry<K, V>* entries, s64 capacity) {
	for (auto i: grd_range(capacity)) {
		entries[i].normalized_hash = HASH_MAP_HASH_EMPTY;
	}
}

template <typename K, typename V>
GrdHashMapEntry<K, V>* grd_put_entry(GrdHashMap<K, V>* map, K key) {
	if (!map->data) {
		if (map->capacity <= 4) {
			map->capacity = 16;
		}
		map->data = GrdAlloc<GrdHashMapEntry<K, V>>(map->allocator, map->capacity, map->loc);
		grd_hash_map_clear_entries_hashes(map->data, map->capacity);
		map->count = 0;
	}
	
	if (map->count >= map->capacity * map->load_factor) {
		auto old_data = map->data;
		auto old_capacity = map->capacity;
		
		map->capacity *= 2;
		map->data = GrdAlloc<GrdHashMapEntry<K, V>>(map->allocator, map->capacity, map->loc);
		grd_hash_map_clear_entries_hashes(map->data, map->capacity);

		for (auto i: grd_range(old_capacity)) {
			auto e = old_data[i];
			if (e.is_occupied()) {
				grd_put_entry(map, e.key)->value = e.value;
			}
		}

		GrdFree(map->allocator, old_data, map->loc);
	}

	GrdHash64 hash = grd_hash_key(key);
	s64 idx = hash % map->capacity;
	s64 home = idx;
	GrdHashMapEntry<K, V>* e = NULL;
	s64 cap = map->capacity;

	while (true) {
		e = &map->data[idx];

		if (e->is_occupied()) {
			if (e->normalized_hash == hash && e->key == key) {
				break;
			}
			
			s64 a_dist = grd_hash_map_distance_from_home(grd_hash_map_get_home(map, e), idx, cap);
			s64 b_dist = grd_hash_map_distance_from_home(home, idx, cap);
			if (a_dist < b_dist) {
				map->count += 1;
				// GrdFree the entry by shifting it's value to the right.
				s64 sh_idx = grd_hash_map_next_index_wraparound(idx, map->capacity);
				while (true) {
					auto sh_e = &map->data[sh_idx];
					if (!sh_e->is_occupied()) {
						*sh_e = *e;
						break;
					}

					s64 c_dist = grd_hash_map_distance_from_home(grd_hash_map_get_home(map, sh_e), sh_idx, cap);
					s64 d_dist = grd_hash_map_distance_from_home(grd_hash_map_get_home(map, e), sh_idx, cap);
					if (c_dist < d_dist) {
						auto tmp = *sh_e;
						*sh_e = *e;
						*e = tmp;
					}
					sh_idx = grd_hash_map_next_index_wraparound(sh_idx, map->capacity);
				}
				break;
			}
		} else {
			map->count += 1;
			break;
		}
		idx = grd_hash_map_next_index_wraparound(idx, map->capacity);
	}
	e->key = key;
	e->normalized_hash = hash;
	return e;
}

template <typename K, typename V>
GrdHashMapEntry<K, V>* grd_get_entry(GrdHashMap<K, V>* map, std::type_identity_t<K> key) {
	if (!map->data) {
		return NULL;
	}
	GrdHash64 hash = grd_hash_key(key);
	s64 idx = hash % map->capacity;
	s64 home = idx;

	while (true) {
		auto e = &map->data[idx];
		if (e->is_occupied()) {
			if (e->normalized_hash == hash && e->key == key) {
				return e;
			}
			
			s64 a_dist = grd_hash_map_distance_from_home(grd_hash_map_get_home(map, e), idx, map->capacity);
			s64 b_dist = grd_hash_map_distance_from_home(home, idx, map->capacity);
			if (a_dist < b_dist) {
				// If the key was in this table,
				//   it would have been in this slot in the worst case.
				// But it's not in here, so we can early out.
				return NULL;
			}
		} else {
			return NULL;
		}
		idx = grd_hash_map_next_index_wraparound(idx, map->capacity);
	}
}

template <typename K, typename V>
GrdHashMapEntry<K, V>* grd_remove(GrdHashMap<K, V>* map, std::type_identity_t<K> key) {
	if (!map->data) {
		return NULL;
	}
	GrdHash64 hash = grd_hash_key(key);
	s64 idx = hash % map->capacity;
	s64 home = idx;

	while (true) {
		auto e = &map->data[idx];
		if (!e->is_occupied()) {
			return NULL;
		}
		if (e->normalized_hash == hash && e->key == key) {
			e->normalized_hash = HASH_MAP_HASH_EMPTY;

			// Backward shift.
			s64 prev_idx = idx;
			s64 shift_idx = grd_hash_map_next_index_wraparound(idx, map->capacity);
			while (true) {
				auto shift_e = &map->data[shift_idx];
				if (!shift_e->is_occupied()) {
					break;
				}
				if (grd_hash_map_get_home(map, shift_e) == shift_idx) {
					break;
				}
				assert(
					grd_hash_map_distance_from_home(grd_hash_map_get_home(map, shift_e), shift_idx, map->capacity) >
					grd_hash_map_distance_from_home(grd_hash_map_get_home(map, shift_e), prev_idx, map->capacity));

				map->data[prev_idx] = *shift_e;
				prev_idx = shift_idx;
				shift_e->normalized_hash = HASH_MAP_HASH_EMPTY;
				shift_idx = grd_hash_map_next_index_wraparound(shift_idx, map->capacity);
			}

			map->count -= 1;
			return e;
		}
		idx = grd_hash_map_next_index_wraparound(idx, map->capacity);
	}
}

template <typename K, typename V>
V* grd_put(GrdHashMap<K, V>* map, std::type_identity_t<K> key) {
	return &grd_put_entry(map, key)->value;
}

template <typename K, typename V>
V* grd_put(GrdHashMap<K, V>* map, std::type_identity_t<K> key, std::type_identity_t<V> value) {
	V* result = grd_put(map, key);
	*result = value;
	return result;
}

template <typename K, typename V>
V* grd_get(GrdHashMap<K, V>* map, std::type_identity_t<K> key) {
	auto* e = grd_get_entry(map, key);
	if (!e) {
		return NULL;
	}
	return &e->value;
}

template <typename K, typename V>
s64 grd_len(GrdHashMap<K, V> map) {
	return map.count;
}

struct GrdHashMapType: GrdMapType {
	s32 key_offset   = 0;
	s32 value_offset = 0;
	s32 hash_offset  = 0;
	u32 entry_size   = 0;

	struct Item: GrdMapType::Item {
		GrdHash64* hash;
	};
};

template <typename K, typename V>
GrdHashMapType* grd_reflect_create_type(GrdHashMap<K, V>* x) {
	return grd_reflect_add_type_named<GrdHashMap<K, V>, GrdHashMapType>("");
}

template <typename K, typename V>
void grd_reflect_type(GrdHashMap<K, V>* x, GrdHashMapType* type) {
	type->key   = grd_reflect_type_of<K>();
	type->value = grd_reflect_type_of<V>();
	type->name  = grd_heap_sprintf("GrdHashMap<%s, %s>", type->key->name, type->value->name);
	type->subkind = "hash_map";

	using Map = GrdHashMap<K, V>;
	using Entry = Map::Entry;

	type->key_offset   = offsetof(Entry, key);
	type->value_offset = offsetof(Entry, value);
	type->hash_offset  = offsetof(Entry, normalized_hash);
	type->entry_size   = sizeof(Entry);

	type->get_count = [](void* map) {
		auto casted = (GrdHashMap<int, int>*) map;
		return grd_len(*casted);
	};

	type->get_capacity = [](void* map) {
		auto casted = (GrdHashMap<int, int>*) map;
		return casted->capacity;
	};

	type->iterate = [](void* map) -> GrdGenerator<GrdMapType::Item*> {
		auto casted_map = (Map*) map;

		GrdHashMapType::Item item;

		for (auto i: grd_range(casted_map->capacity)) {
			auto* entry = (Entry*) grd_ptr_add(casted_map->data, i * sizeof(Entry));
			if (entry->is_occupied()) {
				item.key   = &entry->key;
				item.value = &entry->value;
				item.hash  = &entry->normalized_hash;
				co_yield &item;
			}
		}
	};
}
