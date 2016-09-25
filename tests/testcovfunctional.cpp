#include "covfunctional.hpp"
#include <iostream>
#include <string>
int fff(const char* str)
{
	std::cout<<str<<std::endl;
	return 0;
}
struct fack_func {
	int a=10;
	int operator()(const char* str) const
	{
		std::cout<<typeid(*this).name()<<"|"<<str<<std::endl;
		return 0;
	}
	int func(const char* str)
	{
		std::cout<<a<<"|"<<str<<std::endl;
	}
	int ffunc(const char* str)
	{
		std::cout<<a*2<<"|"<<str<<std::endl;
	}
};
template<typename _Tp,typename _rT,typename..._ArgsT> cov::function<_rT(_ArgsT...)> getfunc(const _Tp& f)
{
	return std::move(cov::function<_rT(_ArgsT...)>(f));
}
int main(int args,char** argv)
{
	std::string str=",World!";
	fack_func ffunc;
	ffunc.a=22;
	auto f=[&](const char* s) {
		std::cout<<s<<str<<std::endl;
		return 0;
	};
	auto func=getfunc<decltype(f),int,const char*>(f);
	cov::function<int(fack_func&,const char*)> mfunc(&fack_func::func);
	func("Hello");
	func=fff;
	func("Hello");
	func=ffunc;
	func("Hello");
	mfunc(ffunc,"Hello");
	mfunc=&fack_func::ffunc;
	mfunc(ffunc,"Hello");
	return 0;
}