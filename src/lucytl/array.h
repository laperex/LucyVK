#pragma once

#include <cstddef>

namespace ltl {
	template <typename T, size_t _s_T>
	class array {
		T _array[_s_T] = {};

		template <size_t _il_T>
		void initializer_list(const T (&il)[_il_T]) {
			memcpy(_array, il, sizeof(il));
		}

	public:
		constexpr T* data() const {
			return begin();
		}

		constexpr size_t size() const {
			return _s_T;
		}

		constexpr operator T*() const {
			return data();
		}


		template <typename ..._il_T>
		array(const _il_T ...il) {
			initializer_list({ il... });
		}


		constexpr T& operator[](size_t index) const {
			return _array[index];
		}

		constexpr T* begin() {
			return _array;
		}

		constexpr T* end() {
			return _array + _s_T - 1;
		}
	};
}