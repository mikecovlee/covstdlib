#include "covstdlib.hpp"
#include <iostream>
#include <string>
int fff(const char* str)
{
	std::cout<<str<<std::endl;
	return 0;
}
int main(int args,char** argv)
{
	auto f=[](const char* str){
		std::cout<<str<<std::endl;
		//return 0;
	};
	cov::function<int(const char*)> func(f);
	return func("Hello");
}