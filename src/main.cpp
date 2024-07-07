// #include "lucy/engine.h"
#include "lucyio/logger.h"
#include <lucy/engine.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "lucytl/linked_list.h"


// // TODO: dynamic viewport
// TODO: renderer restructuring
	// // TODO: semaphore unsignaling
	//! TODO: mesh loading
	// TODO: material management
	// TODO: shader management
// // TODO: memory handles
// TODO: render_pass restructuring
// // TODO: Implement Better Destruction -
// TODO: Implement Image Loading
// TODO: Implement DearImGui


int main(int count, char** args) {
	freopen("output.log", "w", stdout);

	std::vector<int> a = { 0 };
	// ltl::linked_list<int> list = {};
	// auto* a = list.push_back(10);
	// list.push_back(2);
	// list.push_back(3);
	// auto* b = list.push_back(4);
	// auto* c = list.push_back(7);
	
	// for (ltl::linked_list<int>::node* _node = list.end; _node != nullptr; _node = _node->prev) {
	// 	dloggln(_node->data);
	// }
	
	// list.erase(a);
	// list.erase(b);
	// list.erase(c);
	
	// dloggln("after");
	
	// for (ltl::linked_list<int>::node* _node = list.end; _node != nullptr; _node = _node->prev) {
	// 	dloggln(_node->data);
	// }
	
	lucy::engine::initialize();
	lucy::engine::mainloop();
	lucy::engine::destroy();
}