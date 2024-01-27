#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <typeinfo>

namespace ltl {
	class registry {
		std::map<size_t, size_t> type_index_map = {};
		size_t offset = 0;
		uint8_t* storage = nullptr;

	public:
		~registry() {
			free(storage);
		}

		template<typename T, typename ... _T>
		void add(_T ... _t) {
			add(T { _t ... });
		}

		template<typename T>
		void add(const T && _t) {
			// assert(storage == nullptr);
			assert(type_index_map.find(typeid(T).hash_code()) == type_index_map.end());

			type_index_map[typeid(T).hash_code()] = offset;
			offset += sizeof(T);

			if (storage == nullptr) {
				// std::cout << "registry: storage allocated - " << offset << '\n';
				storage = (uint8_t*)malloc(offset);
			} else {
				// std::cout << "registry: storage re-allocation - " << offset << '\n';
				storage = (uint8_t*)realloc(storage, offset);
			}

			new (storage + offset - sizeof(T)) T(_t);
		}

		void initialize() {
			storage = (uint8_t*)malloc(offset);
		}

		template<typename T>
		T& get() {
			return *get_ref<T>();
		}

		template<typename T>
		T* get_ref() {
			assert(type_index_map.find(typeid(T).hash_code()) != type_index_map.end());

			return (T*)(storage + type_index_map[typeid(T).hash_code()]);
		}
	};
}