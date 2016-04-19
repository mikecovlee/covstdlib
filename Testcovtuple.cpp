#include "covstdlib.h"

template < typename T > void cov::for_each_process(T & dat)
{
	cov::print(dat, cov::ioctrl::endl);
}

int main()
{
	using namespace cov;
	auto t = make_tuple(222, 3.1415926, '@', "Hello,Tuple!");
	for_each(t);
	cov::print(cov::tuple_size(t));
}