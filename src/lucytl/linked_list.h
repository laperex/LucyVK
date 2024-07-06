#pragma once

#include <cstdint>


namespace ltl {
	template <typename T>
	class linked_list {
	public:
		struct node {
			node* prev;
			node* next;

			T data;
		};
		
		node* begin = nullptr;
		node* end = nullptr;
		
		uint32_t size = 0;
	
		void push_back(const T& data) {
			if (begin == nullptr) {
				begin = new node {
					.prev = end,
					.next = nullptr,
					.data = data
				};
			}
		}
	};
}