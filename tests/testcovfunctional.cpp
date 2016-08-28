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
	std::string str=",World!";
	auto f=[&](const char* s) {
		std::cout<<s<<str<<std::endl;
		return 0;
	};
	cov::function<int(const char*)> func(f);
	func("Hello");
	func=fff;
	func("Hello");
	return 0;
}