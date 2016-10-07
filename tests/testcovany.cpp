#include "covstdlib.hpp"
#include <iostream>
#include <cmath>
int cov::covstdlib::main(const cov::covstdlib::string_array& args)
{
	cov::any var;
	var=var.usable();
	if(!var)
		std::cout<<"Ok"<<std::endl;
	if(var!=12345)
		std::cout<<"Ok"<<std::endl;
	var=3.1415926;
	var=std::pow(static_cast<double>(var),static_cast<double>(var));
	std::cout<<var<<std::endl;
	var="TestStr";
	std::cout<<var<<std::endl;
	const cov::any var1=var;
	if(var==var1)
		std::cout<<"Ok"<<std::endl;
	std::cout<<var1<<std::endl;
	var=0;
	return var;
}