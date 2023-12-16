#pragma once

#include <array>
#include <iostream>
#include <unordered_map>
#include <vector>

#ifndef dlogg
	#define dlogg(...) util::print(__VA_ARGS__)
#endif
#ifndef dloggln
	#define dloggln(...) util::println(__VA_ARGS__)
#endif

template <typename T>
std::ostream& operator<<(std::ostream& output, std::vector<T> const& values)
{
	output << '[';
	for (int i = 0; i < values.size(); i++)
	{
		output << ' ' << values[i] << ((i < values.size() - 1) ? ',' : ' ');
	}
	output << ']';
	
	return output;
}

template <typename Tp, size_t Nm>
std::ostream& operator<<(std::ostream& output, std::array<Tp, Nm> const& values)
{
	output << '[';
	for (int i = 0; i < values.size(); i++)
	{
		output << ' ' << values[i] << ((i < values.size() - 1) ? ',' : ' ');
	}
	output << ']';
	
	return output;
}

template <typename Key, typename Tp>
std::ostream& operator<<(std::ostream& output, std::unordered_map<Key, Tp> const& values)
{
	output << '{';
	int i = 0;
	for (const auto& pair: values) {
		output << ' ' << pair.first << " : " << pair.second << ((i < values.size() - 1) ? ',' : ' ');
		i += 1;
	}
	output << '}';
	
	return output;
}

namespace util {
	inline void print() {}

	template <typename T>
	inline void print(T a) {
		std::cout << a << ' ';
	}

	// template <typename T, typename Tp>
	// inline void print<std::vector<Tp>>(T a) {
		
	// }

	template <typename T, typename ...Ta>
	void print(T a, Ta ...as) {
		print(a);
		print(as...);
	}

	template <typename ...Ta>
	void println(Ta ...as) {
		print(as..., '\n');
	}
}