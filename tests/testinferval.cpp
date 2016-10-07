#include <iostream>
#include <string>
#include "covstdlib.hpp"
int cov::covstdlib::main(const cov::covstdlib::string_array& args)
{
	cov::any val;
	while(true) {
		std::cin>>val;
		std::cout<<val<<std::endl<<val.type().name()<<std::endl;
	}
	return 0;
}