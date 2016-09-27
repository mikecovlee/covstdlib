#include <iostream>
#include "covstdlib.hpp"
#if __covcpplib < 201610L
#error Covstdlib version is too old.
#endif
int main()
{
	std::cout<<
	cov::timer::measure(cov::timer::time_unit::milli_sec,cov::function<void()>([] {
		cov::timer::delay(cov::timer::time_unit::milli_sec,101);
	})) <<std::endl;
	cov::any a=3.1415926;
	std::cout<<a<<std::endl;
	auto t=cov::make_tuple(22,3.1415926,'@',std::string("Hello"));
	bool run=true;
	while(run) {
		std::string str;
		std::getline(std::cin,str);
		Switch(str) {
			Case("int") {
				std::cout<<t.get<int>()<<std::endl;
			}
			EndCase;
			Case("double") {
				std::cout<<t.get<double>()<<std::endl;
			}
			EndCase;
			Case("char") {
				std::cout<<t.get<char>()<<std::endl;
			}
			EndCase;
			Case("string") {
				std::cout<<t.get<std::string>()<<std::endl;
			}
			EndCase;
			Case("Exit") {
				run=false;
			}
			EndCase;
			Default {
				std::cout<<"Default Case"<<std::endl;
			} EndCase;
		}
		EndSwitch;
	}
	return 0;
}