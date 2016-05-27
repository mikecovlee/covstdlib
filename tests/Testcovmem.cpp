#include "covmem.h"
#include <iostream>
#include <string>
#include <ctime>
int main()
{
	int i = 0;
	for (double s(clock()), e(clock()); (e - s) / (double)CLOCKS_PER_SEC <= 1; e = clock()) {
		cov::allocator < std::string >::allocate();
		++i;
	}
	printf("%d\n", i);
	auto a = cov::allocator < std::string >::allocate();
	a = "Hello";
	printf("%s %p\n", a.data().c_str(), &a.raw_data());
	cov::allocator < std::string >::deallocate(a);
	const auto b = cov::allocator < std::string >::allocate("Hello");
	printf("%s %p\n", b.data().c_str(), &b.raw_data());
	return 0;
}