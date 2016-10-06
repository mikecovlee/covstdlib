#include <iostream>
#include "covstdlib.hpp"
#if __covcpplib < 201613L
#error Covstdlib version is too old.
#endif
int main()
{
	std::cout<<
	cov::timer::measure(cov::timer::time_unit::milli_sec,[] {
		cov::timer::delay(cov::timer::time_unit::milli_sec,101);
	}) <<std::endl;
	cov::any val=3.1415926;
	std::cout<<val<<std::endl;
	auto t=cov::make_tuple(22,3.1415926,'@',std::string("Hello"));
	bool run=true;
	while(run) {
		std::string str;
		std::getline(std::cin,str);
		Switch(str) {
			Case("int") {
				val=t.get<int>();
				std::cout<<t.get<int>()<<std::endl;
			}
			EndCase;
			Case("double") {
				val=t.get<double>();
				std::cout<<t.get<double>()<<std::endl;
			}
			EndCase;
			Case("char") {
				val=t.get<char>();
				std::cout<<t.get<char>()<<std::endl;
			}
			EndCase;
			Case("string") {
				val=t.get<std::string>();
				std::cout<<t.get<std::string>()<<std::endl;
			}
			EndCase;
			Case("Show") {
				Switch(val.type()) {
					Case(typeid(int)) {
						std::cout<<"INT"<<std::endl;
					}
					EndCase;
					Case(typeid(double)) {
						std::cout<<"DOUBLE"<<std::endl;
					}
					EndCase;
					Case(typeid(char)) {
						std::cout<<"CHAR"<<std::endl;
					}
					EndCase;
					Case(typeid(std::string)) {
						std::cout<<"STRING"<<std::endl;
					}
					EndCase;
					Default {
						std::cout<<val.type().name()<<std::endl;
					} EndCase;
				}
				EndSwitch;
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