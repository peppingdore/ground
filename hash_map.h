#pragma once

#include "hash.h"
#include "array.h"
#include "code_location.h"
#include "reflection.h"
#include "optional.h"

constexpr s64    DEFAULT_HASH_MAP_CAPACITY = 16;
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

	Array<Entry> entries;
	f32          load_factor = 0.75;
	CodeLocation loc = caller_loc();

	void make_sure_initted() {
		if (entries.data) {
			return;
		}
		entries.make_sure_initted();
		clear_entries_hashes();
	}

	s64 get_home(Entry* e) {
		assert(entries.data);
		return e->normalized_hash % entries.capacity;
	}

	s64 count() {
		return entries.count;
	}

	s64 capacity() {
		return entries.capacity;
	}

	Hash64 normalize_hash(Hash64 hash) {
		if (hash < HASH_MAP_SLOT_HASH_FIRST_OCCUPIED) {
			return HASH_MAP_SLOT_HASH_FIRST_OCCUPIED;
		}
		return hash;
	}

	Hash64 hash_key(K key) {
		auto h = hash64(key);
		return normalize_hash(h);
	}

	s64 distance_from_home(s64 home, s64 slot) {
		if (slot < home) {
			return slot + (entries.capacity - home);
		}
		return slot - home;
	}

	s64 next_index_wraparound(s64 idx) {
		return (idx + 1) % entries.capacity;
	}

	void shift_to_right(Entry shifting, s64 shifting_idx) {
		s64 idx = shifting_idx;

		while (true) {
			Entry* current = &entries.data[idx];

			if (!current->is_occupied()) {
				*current = shifting;
				break;
			}
			
			if (distance_from_home(get_home(current), idx) < distance_from_home(get_home(&shifting), idx)) {
				swap(current, &shifting);
			}

			idx = next_index_wraparound(idx);
		}
	}


	void clear_entries_hashes() {
		for (auto i: range(entries.capacity)) {
			entries.data[i].normalized_hash = HASH_MAP_HASH_EMPTY;
		}
	}

	bool should_grow() {
		return entries.count >= entries.capacity * load_factor;
	}

	void grow() {
		auto old_entries = entries;
		entries = {};
		entries.allocator = old_entries.allocator;
		entries.capacity  = old_entries.capacity * 2;
		
		make_sure_initted();

		for (auto i: range(old_entries.capacity)) {
			auto e = old_entries.data[i];

			if (e.is_occupied()) {
				put_entry(e.key)->value = e.value;

				auto found = get_entry(e.key);
				assert(found);
			}
		}

		for (auto i: range(old_entries.capacity)) {
			auto e = old_entries.data[i];

			if (e.is_occupied()) {
				auto found = get_entry(e.key);
				assert(found);
			}
		}

		old_entries.free();
	}

	void clear() {
		entries.count = 0;
		clear_entries_hashes();
	}

	V* put(K key) {
		return &put_entry(key)->value;
	}

	V* put(K key, V value) {
		V* result = put(key);
		*result = value;
		return result;
	}

	V* get(K key) {
		auto* e = get_entry(key);
		if (!e) {
			return NULL;
		}
		return &e->value;
	}

	Optional<V> get_value(K key) {
		auto res = get(key);
		if (res) {
			return *res;
		}
		return {};
	}

	V* get_with_default(K key, V default_value = {}) {
		auto v = get(key);
		if (!v) {
			return put(key, default_value);
		}
		return v;
	}

	Entry* put_entry(K key) {
		make_sure_initted();
		if (should_grow()) {
			grow();
		}

		Hash64 hash = hash_key(key);
		s64 idx = hash % entries.capacity;
		s64 home = idx;
		Entry* e = NULL;

		while (true) {
			e = &entries.data[idx];

			if (e->is_occupied()) {
				if (e->normalized_hash == hash && e->key == key) {
					break;
				}

				if (distance_from_home(get_home(e), idx) < distance_from_home(home, idx)) {
					entries.count += 1;
					shift_to_right(*e, idx);
					break;
				}
			} else {
				entries.count += 1;
				break;
			}

			idx = next_index_wraparound(idx);
		}
		e->key = key;
		e->normalized_hash = hash;
		return e;
	}

	Entry* get_entry(K key) {
		if (!entries.data) {
			return NULL;
		}
		
		Hash64 hash = hash_key(key);
		s64    idx = hash % entries.capacity;
		s64    home = idx;

		while (true) {
			Entry* e = &entries.data[idx];

			if (e->is_occupied()) {
				if (e->normalized_hash == hash && e->key == key) {
					return e;
				}

				if (distance_from_home(get_home(e), idx) < distance_from_home(home, idx)) {
					// If key would have been in this table, it would be in this slot in the worst case.
					//   But it's not in here, so we can early out.

					return NULL;
				}
			} else {
				return NULL;
			}

			idx = next_index_wraparound(idx);
		}
	}

	void shift_left(s64 into) {
		s64 prev_idx = into;
		s64 idx      = next_index_wraparound(into);

		while (true) {
			Entry* current = &entries.data[idx];

			if (!current->is_occupied()) {
				break;
			}

			if (get_home(current) == idx) {
				break;
			}

			assert(distance_from_home(get_home(current), idx) > distance_from_home(get_home(current), prev_idx));
			entries.data[prev_idx] = *current;
			prev_idx = idx;
			current->normalized_hash = HASH_MAP_HASH_EMPTY;

			idx = next_index_wraparound(idx);
		}
	}

	Entry* remove(K key) {
		if (!entries.data) {
			return NULL;
		}
		Hash64 hash = hash_key(key);
		s64    idx = hash % entries.capacity;
		s64    home = idx;

		while (true) {
			Entry* e = &entries.data[idx];

			if (!e->is_occupied()) {
				return NULL;
			}

			if (e->normalized_hash == hash && e->key == key) {
				e->normalized_hash = HASH_MAP_HASH_EMPTY;
				shift_left(idx);
				entries.count -= 1;
				return e;
			}

			idx = next_index_wraparound(idx);
		}

		return NULL;
	}

	HashMap copy(Allocator allocator) {
		auto copied = *this;
		copied.allocator = allocator;
		if (entries.data) {
			copied.entries.ensure_capacity(entries.capacity);
			memcpy(copied.entries.data, entries.data, entries.capacity * sizeof(Entry));
			copied.entries.count = entries.count;			
		}
		return copied;
	}

	void free() {
		entries.free();
	}

	Generator<Entry*> iterate() {
		for (auto i: range(entries.capacity)) {
			auto entry = entries.data[i];
			if (entry->hash >= HASH_MAP_SLOT_HASH_FIRST_OCCUPIED) {
				co_yield entry;
			}
		}
	}
};

template <typename K, typename V>
inline HashMap<K, V> make_hash_map(Allocator allocator = c_allocator, s64 capacity = DEFAULT_HASH_MAP_CAPACITY, CodeLocation loc = caller_loc()) {
	HashMap<K, V> map;
	map.entries.allocator = allocator;
	map.loc = loc;
	map.entries.capacity = capacity;
	return map;
}

template <typename K, typename V>
inline void make_hash_map(HashMap<K, V>* map, Allocator allocator = c_allocator, s64 capacity = DEFAULT_HASH_MAP_CAPACITY) {
	*map = make_hash_map<K, V>(allocator, capacity);
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
HashMapType* reflect_type(HashMap<K, V>* map, HashMapType* type) {
	type->key   = reflect.type_of<K>();
	type->value = reflect.type_of<V>();
	type->name  = heap_sprintf("HashMap<%s, %s>", type->key->name, type->value->name);
	type->subkind = "hash_map";

	using Map = HashMap<K, V>;
	using Entry = Map::Entry;

	type->key_offset   = offsetof(Entry, key);
	type->value_offset = offsetof(Entry, value);
	type->hash_offset  = offsetof(Entry, hash);
	type->entry_size   = sizeof(Entry);

	type->get_count = [](void* map) {
		auto casted = (HashMap<int, int>*) map;
		return casted->count();
	};

	type->get_capacity = [](void* map) {
		auto casted = (HashMap<int, int>*) map;
		return casted->capacity();
	};

	type->iterate = [](void* map) -> Generator<MapType::Item*> {
		auto casted_map = (Map*) map;

		HashMapType::Item item;

		for (auto i: range(casted_map->capacity())) {
			auto* entry = (Entry*) ptr_add(casted_map->entries.data, i * sizeof(Entry));
			if (entry->hash >= HASH_MAP_SLOT_HASH_FIRST_OCCUPIED) {
				item.key   = &entry->key;
				item.value = &entry->value;
				item.hash  = &entry->hash;
				co_yield &item;
			}
		}
	};

	return type;
}
