#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <stdexcept>

namespace ltl {
	template <typename T>
	class list {
		T* _begin = nullptr;
		T* _end = nullptr;
		size_t _capacity = 0;

		template <size_t _s_T>
		void initializer_list(const T (&il)[_s_T]) {
			_capacity = _s_T;
			_begin = (T*)malloc(sizeof(il));
			_end = &_begin[_capacity];
			memcpy(_begin, il, sizeof(il));
		}

	public:
		list() {
			_begin = nullptr;
			_end = nullptr;
			_capacity = 0;
		}
		
		list(const list<T>& _list) {
			_begin = _list._begin;
			_end = _list._end;
			_capacity = _list._capacity;
		}
	
		template <typename ..._il_T>
		list(const _il_T ...il) {
			initializer_list({ il... });
		}
		
		inline list<T>& operator=(const list<T>& _list) {
			_begin = _list._begin;
			_end = _list._end;
			_capacity = _list._capacity;

			return *this;
		}
	
		inline constexpr void resize(size_t size) {
			if (size > _capacity) {
				_begin = (_begin == nullptr) ? (T*)malloc(size * sizeof(T)): (T*)realloc(_begin, size * sizeof(T));
				if (_capacity == 0) {
					_end = _begin;
				}
				_capacity = size;
			}
		}

		// inline constexpr void resize() {
		// 	_begin = (T*)realloc(_begin, size() * sizeof(T));
		// 	_capacity = size();
		// }


		[[nodiscard]] inline constexpr T& operator[](const size_t index) {
			return _begin[index];
		}


		[[nodiscard]] inline constexpr const size_t size() {
			return (_begin != nullptr) ? _end - _begin: 0;
		}
		
		[[nodiscard]] inline constexpr const size_t data() {
			return _begin;
		}

		[[nodiscard]] inline constexpr const size_t capacity() {
			return _capacity;
		}


		void push_back(const T& element) {
			if (size() >= _capacity) {
				_begin = (T*)realloc(_begin, (++_capacity) * sizeof(T));
				_end = (_begin + _capacity - 1);
			}

			*_end++ = element;
			
			// std::vector<>
		}
		
		T pop_back() {
			return *--_end;
		}


		T* begin() {
			return _begin;
		}

		T* end() {
			return _end;
		}


		~list() {
			free(_begin);
		}
	};
}