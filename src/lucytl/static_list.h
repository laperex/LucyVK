#pragma once

#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace ltl {
	template <typename T>
	class static_list {
		T* _begin = nullptr;
		uint32_t _capacity = 0;
		
		template <size_t _il_T>
		void initializer_list(const T (&il)[_il_T]) {
			reserve(_il_T);
			memcpy(_begin, il, sizeof(il));
		}

	public:
		static_list() {
			_capacity = 0;
			_begin = nullptr;
		}

		template <typename ..._il_T>
		static_list(const _il_T ...il) {
			initializer_list({ il... });
		}

		static_list(static_list<T>& _list) {
			_begin = _list._begin;
			_capacity = _list._capacity;

			_list._begin = nullptr;
		}

		// inline static_list<T>& operator=(static_list<T>& _list) {
		// 	_begin = _list._begin;
		// 	_capacity = _list._capacity;

		// 	_list._begin = nullptr;

		// 	return *this;
		// }

		[[nodiscard]] inline constexpr T& operator[](const size_t index) {
			if (_capacity <= index) {
				throw std::runtime_error("Error: Index Out of Bounds");
			}

			return _begin[index];
		}

		void reserve(uint32_t size) {
			if (_capacity != 0) {
				throw std::runtime_error("Error: Repeated Memory Reservation");
			}

			_begin = new T[size];
			_capacity = size;
		}


		T* data() {
			return _begin;
		}

		uint32_t size() {
			return _capacity;
		}
		
		T* begin() {
			return _begin;
		}
		
		T* end() {
			return _begin + _capacity - 1;
		}

		~static_list() {
			delete [] _begin;
		}
	};
}