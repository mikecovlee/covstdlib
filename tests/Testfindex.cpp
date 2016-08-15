#include <iostream>
#include "covstdlib.h"
int main()
{
	return cov::make_function_index([&] {
		std::cout<<"Hello,world!"<<std::endl;
		return 0;
	})();
}