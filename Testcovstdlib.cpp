#include "covstdlib.h"
using namespace cov;
int main()
{
	any txt;
	txt = 3.1415926;
	print(txt, ioctrl::endl);
	cov::timer_t begin, end;
	begin = clocks.time(clocks.timeUnit::milliSec);
	clocks.delay(clocks.timeUnit::milliSec, 100);
	end = clocks.time(clocks.timeUnit::milliSec);
	print < int >(end - begin);
}