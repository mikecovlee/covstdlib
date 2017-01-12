#include "./type_list_beta2.hpp"
#include <typeinfo>
#include <cstdio>
int main()
{
	using A=typename cov::make_tlist<int,float,char,bool>::result;
	printf("A:size=%d\t",cov::get_size<A>::result);
	cov::iterator<A>::show();
	printf("\n");
	
	using B=typename cov::erase<2,A>::result;
	printf("B:size=%d\t",cov::get_size<B>::result);
	cov::iterator<B>::show();
	printf("\n");
	
	using C=typename cov::insert<2,A,double>::result;
	printf("C:size=%d\t",cov::get_size<C>::result);
	cov::iterator<C>::show();
	printf("\n");
	
	using D=typename cov::push_back<C,void>::result;
	printf("D:size=%d\t",cov::get_size<D>::result);
	cov::iterator<D>::show();
	printf("\n");
	
	using E=typename cov::remove<typename cov::push_front<D,void>::result,void>::result;
	printf("E:size=%d\t",cov::get_size<E>::result);
	cov::iterator<E>::show();
	printf("\n");
	
	using F=typename cov::remove<typename cov::make_tlist<void,int,void,double,void>::result,void>::result;
	printf("F:size=%d\t",cov::get_size<F>::result);
	cov::iterator<F>::show();
	printf("\n");
	
	return 0;
}