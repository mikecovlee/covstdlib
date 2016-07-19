#include "covstdlib.h"
#include <cstdio>
#include <string>
int main()
{
	auto t = cov::make_tuple(3.1415926, std::string("Hello"), 22);
	printf("%s", t.get < std::string > ().c_str());
}