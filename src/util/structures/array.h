#pragma once

#include <cstddef>
#include <initializer_list>

namespace util {
	template <typename T, size_t _s_T>
	class array {
		T _array[_s_T];
		
		array(std::initializer_list<T> il): _array(il) {}

		T& operator[](size_t index) {
			return _array[index];
		}
		
		size_t size() {
			return _s_T;
		}
		
		T* begin() {
			return _array;
		}

		T* end() {
			return _array + _s_T - 1;
		}
	};
}