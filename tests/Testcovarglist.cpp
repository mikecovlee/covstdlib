#include "covstdlib.h"
#include <iostream>
void func0(cov::argument_list args)
{
	args.check<int,double,char>(__func__);
	std::cout<<args[0].val<int>()<<std::endl<<args[1].val<double>()<<std::endl<<args[2].val<char>()<<std::endl;
}
void func1(cov::argument_list args)
{
	for(auto&it:args)
		std::cout<<it<<std::endl;
}
int main()
{
	func1({std::string("Hello,World!"),3.1415926});
	func0({22,3,'@'});
	return 0;
}