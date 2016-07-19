#include "covstdlib.h"
#include <iostream>
using namespace cov;
int main()
{
	any txt;
	txt = 3.1415926;
	std::cout<<txt<<std::endl;
	txt = 123456;
	std::cout<<txt<<std::endl;
	std::cout<<clocks.measure(timer::timeUnit::milliSec,[&] {
		clocks.delay(clocks.timeUnit::milliSec, 100);
	}) <<std::endl;
	return 0;
}
