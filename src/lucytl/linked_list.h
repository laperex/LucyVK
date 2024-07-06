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

		void erase(node* _node) {
			if (_node != end) {
				_node->next->prev = _node->prev;
			} else {
				end = _node->prev;
			}

			if (_node != begin) {
				_node->prev->next = _node->next;
			} else {
				begin = _node->next;
			}
			
			size--;

			delete _node;
		}

		node* push_back(const T& data) {
			node* new_node = new node {
				.prev = end,
				.next = nullptr,
				.data = data
			};
			
			if (end != nullptr) {
				end->next = new_node;
			} else {
				begin = new_node;
			}
			end = new_node;
			
			size++;
			
			return end;
		}
	};
}