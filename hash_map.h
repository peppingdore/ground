#pragma once

#include "hash.h"
#include "allocator.h"
#include "reflect.h"

constexpr Hash64 HASH_MAP_HASH_EMPTY = 0;
constexpr Hash64 HASH_MAP_SLOT_HASH_FIRST_OCCUPIED = 1;

template <typename K, typename V>
struct HashMapEntry {
	K      key;
	V      value;
	Hash64 normalized_hash;

	bool is_occupied() {
		return normalized_hash != HASH_MAP_HASH_EMPTY;
	}
};

template <typename K, typename V>
struct HashMap {
	using Entry = HashMapEntry<K, V>;

	Allocator    allocator   = c_allocator;
	Entry*       data        = NULL;
	s64          capacity    = 0;
	s64          count       = 0;
	f32          load_factor = 0.75;
	CodeLocation loc = caller_loc();

	void free() {
		if (data) {
			Free(allocator, data, loc);
			data  = NULL;
		}
		*this = {};
	}

	Generator<Entry*> iterate() {
		for (auto i: range(capacity)) {
			auto* e = &data[i];
			if (e->is_occupied()) {
				co_yield e;
			}
		}
	}
};

template <typename K, typename V>
s64 hash_map_get_home(HashMap<K, V>* map, HashMapEntry<K, V>* e) {
	assert(map->capacity > 0);
	return e->normalized_hash % map->capacity;
}

Hash64 hash_map_normalize_hash(Hash64 hash) {
	if (hash < HASH_MAP_SLOT_HASH_FIRST_OCCUPIED) {
		return HASH_MAP_SLOT_HASH_FIRST_OCCUPIED;
	}
	return hash;
}

template <typename K>
Hash64 hash_key(K key) {
	auto h = hash64(key);
	return hash_map_normalize_hash(h);
}

s64 hash_map_distance_from_home(s64 home, s64 slot, s64 capacity) {
	if (slot < home) {
		return slot + (capacity - home);
	}
	return slot - home;
}

s64 hash_map_next_index_wraparound(s64 idx, s64 capacity) {
	return (idx + 1) % capacity;
}

template <typename K, typename V>
void hash_map_clear_entries_hashes(HashMapEntry<K, V>* entries, s64 capacity) {
	for (auto i: range(capacity)) {
		entries[i].normalized_hash = HASH_MAP_HASH_EMPTY;
	}
}

template <typename K, typename V>
HashMapEntry<K, V>* put_entry(HashMap<K, V>* map, K key) {
	if (!map->data) {
		if (map->capacity <= 4) {
			map->capacity = 16;
		}
		map->data = Alloc<HashMapEntry<K, V>>(map->allocator, map->capacity, map->loc);
		hash_map_clear_entries_hashes(map->data, map->capacity);
		map->count = 0;
	}
	
	if (map->count >= map->capacity * map->load_factor) {
		auto old_data = map->data;
		auto old_capacity = map->capacity;
		
		map->capacity *= 2;
		map->data = Alloc<HashMapEntry<K, V>>(map->allocator, map->capacity, map->loc);
		hash_map_clear_entries_hashes(map->data, map->capacity);

		for (auto i: range(old_capacity)) {
			auto e = old_data[i];
			if (e.is_occupied()) {
				put_entry(map, e.key)->value = e.value;
			}
		}

		Free(map->allocator, old_data, map->loc);
	}

	Hash64 hash = hash_key(key);
	s64 idx = hash % map->capacity;
	s64 home = idx;
	HashMapEntry<K, V>* e = NULL;
	s64 cap = map->capacity;

	while (true) {
		e = &map->data[idx];

		if (e->is_occupied()) {
			if (e->normalized_hash == hash && e->key == key) {
				break;
			}
			
			s64 a_dist = hash_map_distance_from_home(hash_map_get_home(map, e), idx, cap);
			s64 b_dist = hash_map_distance_from_home(home, idx, cap);
			if (a_dist < b_dist) {
				map->count += 1;
				// Free the entry by shifting it's value to the right.
				s64 sh_idx = hash_map_next_index_wraparound(idx, map->capacity);
				while (true) {
					auto sh_e = &map->data[sh_idx];
					if (!sh_e->is_occupied()) {
						*sh_e = *e;
						break;
					}

					s64 c_dist = hash_map_distance_from_home(hash_map_get_home(map, sh_e), sh_idx, cap);
					s64 d_dist = hash_map_distance_from_home(hash_map_get_home(map, e), sh_idx, cap);
					if (c_dist < d_dist) {
						auto tmp = *sh_e;
						*sh_e = *e;
						*e = tmp;
					}
					sh_idx = hash_map_next_index_wraparound(sh_idx, map->capacity);
				}
				break;
			}
		} else {
			map->count += 1;
			break;
		}
		idx = hash_map_next_index_wraparound(idx, map->capacity);
	}
	e->key = key;
	e->normalized_hash = hash;
	return e;
}

template <typename K, typename V>
HashMapEntry<K, V>* get_entry(HashMap<K, V>* map, std::type_identity_t<K> key) {
	if (!map->data) {
		return NULL;
	}
	Hash64 hash = hash_key(key);
	s64 idx = hash % map->capacity;
	s64 home = idx;

	while (true) {
		auto e = &map->data[idx];
		if (e->is_occupied()) {
			if (e->normalized_hash == hash && e->key == key) {
				return e;
			}
			
			s64 a_dist = hash_map_distance_from_home(hash_map_get_home(map, e), idx, map->capacity);
			s64 b_dist = hash_map_distance_from_home(home, idx, map->capacity);
			if (a_dist < b_dist) {
				// If the key was in this table,
				//   it would have been in this slot in the worst case.
				// But it's not in here, so we can early out.
				return NULL;
			}
		} else {
			return NULL;
		}
		idx = hash_map_next_index_wraparound(idx, map->capacity);
	}
}

template <typename K, typename V>
HashMapEntry<K, V>* remove(HashMap<K, V>* map, std::type_identity_t<K> key) {
	if (!map->data) {
		return NULL;
	}
	Hash64 hash = hash_key(key);
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
			s64 shift_idx = hash_map_next_index_wraparound(idx, map->capacity);
			while (true) {
				auto shift_e = &map->data[shift_idx];
				if (!shift_e->is_occupied()) {
					break;
				}
				if (hash_map_get_home(map, shift_e) == shift_idx) {
					break;
				}
				assert(hash_map_distance_from_home(hash_map_get_home(map, shift_e), shift_idx, map->capacity) > hash_map_distance_from_home(hash_map_get_home(map, shift_e), prev_idx, map->capacity));

				map->data[prev_idx] = *shift_e;
				prev_idx = shift_idx;
				shift_e->normalized_hash = HASH_MAP_HASH_EMPTY;
				shift_idx = hash_map_next_index_wraparound(shift_idx, map->capacity);
			}

			map->count -= 1;
			return e;
		}
		idx = hash_map_next_index_wraparound(idx, map->capacity);
	}
}

template <typename K, typename V>
V* put(HashMap<K, V>* map, std::type_identity_t<K> key) {
	return &put_entry(map, key)->value;
}

template <typename K, typename V>
V* put(HashMap<K, V>* map, std::type_identity_t<K> key, std::type_identity_t<V> value) {
	V* result = put(map, key);
	*result = value;
	return result;
}

template <typename K, typename V>
V* get(HashMap<K, V>* map, std::type_identity_t<K> key) {
	auto* e = get_entry(map, key);
	if (!e) {
		return NULL;
	}
	return &e->value;
}

template <typename K, typename V>
s64 len(HashMap<K, V> map) {
	return map.count;
}

struct HashMapType: MapType {
	s32 key_offset   = 0;
	s32 value_offset = 0;
	s32 hash_offset  = 0;
	u32 entry_size   = 0;

	struct Item: MapType::Item {
		Hash64* hash;
	};
};

template <typename K, typename V>
HashMapType* reflect_type(HashMap<K, V>* x, HashMapType* type) {
	type->key   = reflect_type_of<K>();
	type->value = reflect_type_of<V>();
	type->name  = heap_sprintf("HashMap<%s, %s>", type->key->name, type->value->name);
	type->subkind = "hash_map";

	using Map = HashMap<K, V>;
	using Entry = Map::Entry;

	type->key_offset   = offsetof(Entry, key);
	type->value_offset = offsetof(Entry, value);
	type->hash_offset  = offsetof(Entry, normalized_hash);
	type->entry_size   = sizeof(Entry);

	type->get_count = [](void* map) {
		auto casted = (HashMap<int, int>*) map;
		return len(*casted);
	};

	type->get_capacity = [](void* map) {
		auto casted = (HashMap<int, int>*) map;
		return casted->capacity;
	};

	type->iterate = [](void* map) -> Generator<MapType::Item*> {
		auto casted_map = (Map*) map;

		HashMapType::Item item;

		for (auto i: range(casted_map->capacity)) {
			auto* entry = (Entry*) ptr_add(casted_map->data, i * sizeof(Entry));
			if (entry->is_occupied()) {
				item.key   = &entry->key;
				item.value = &entry->value;
				item.hash  = &entry->normalized_hash;
				co_yield &item;
			}
		}
	};
	return type;
}
