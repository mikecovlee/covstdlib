#include "covmem.h"
#include <iostream>
#include <string>
int main()
{
	auto a = cov::allocator < std::string >::allocate();
	a = "Hello";
	printf("%s %p\n", a.data().c_str(), &a.raw_data());
	cov::allocator < std::string >::deallocate(a);
	const auto b = cov::allocator < std::string >::allocate("Haha");
	printf("%s %p\n", b.data().c_str(), &b.raw_data());
	return 0;
}