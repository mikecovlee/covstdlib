#include "covstdlib.hpp"
#include <iostream>
#include <string>
int fff(const char* str)
{
	std::cout<<str<<std::endl;
	return 0;
}
struct fack_func {
	int a=10;
	int operator()(const char* str)
	{
		std::cout<<typeid(*this).name()<<"|"<<str<<std::endl;
		return 0;
	}
	int func(const char* str) const
	{
		std::cout<<a<<"|"<<str<<std::endl;
	}
	int ffunc(const char* str) const
	{
		std::cout<<a*2<<"|"<<str<<std::endl;
	}
};
int main(int args,char** argv)
{
	std::string str=",World!";
	fack_func ffunc;
	ffunc.a=22;
	auto f=[&](const char* s) {
		std::cout<<s<<str<<std::endl;
		return 0;
	};
	cov::function<int(const char*)> func(f);
	cov::function<int(fack_func::*)(const char*) const> mfunc(&fack_func::func);
	func("Hello");
	func=fff;
	func("Hello");
	func=ffunc;
	func("Hello");
	mfunc(&ffunc,"Hello");
	mfunc=&fack_func::ffunc;
	mfunc(&ffunc,"Hello");
	return 0;
}