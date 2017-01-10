#include "./type_list.hpp"
#include <typeinfo>
#include <cstdio>
int main()
{
	using A=typename cov::make_tlist<int,float,char,bool>::result;
	printf("%d\n",cov::get_size<A>::result);
	cov::iterator<A>::show();
	using B=typename cov::erase<1,A>::result;
	printf("%d\n",cov::get_size<B>::result);
	cov::iterator<B>::show();
	using C=typename cov::insert<2,A,double>::result;
	printf("%d\n",cov::get_size<C>::result);
	cov::iterator<C>::show();
	return 0;
}