#pragma once
#include <cstdio>
#include <typeinfo>
namespace cov {
	template<typename fT,typename sT>struct tlist_node {
		using first=fT;
		using second=sT;
	};
	template<typename...>struct make_tlist;
	template<typename fT,typename sT>struct make_tlist<fT,sT> {
		using result=tlist_node<fT,sT>;
	};
	template<typename T,typename...Args>struct make_tlist<T,Args...> {
		using result=tlist_node<T,typename make_tlist<Args...>::result>;
	};
	template<typename T,int size=0>struct get_size {
		static constexpr int result=size+1;
	};
	template<typename fT,typename sT,int size>struct get_size<tlist_node<fT,sT>,size> {
		static constexpr int result=get_size<sT,size+1>::result;
	};
	template<typename T>struct iterator {
		static void show()
		{
			printf("%s\n",typeid(T).name());
		}
	};
	template<typename fT,typename sT>struct iterator<tlist_node<fT,sT>> {
		static void show()
		{
			printf("%s\t",typeid(fT).name());
			iterator<sT>::show();
		}
	};
	template<int posit,typename T>struct get_type {
		//using result=void;
	};
	template<int posit,typename fT,typename sT>struct get_type<posit,tlist_node<fT,sT>> {
		using result=typename get_type<posit-1,sT>::result;
	};
	template<typename T>struct get_type<0,T> {
		using result=T;
	};
	template<typename fT,typename sT>struct get_type<0,tlist_node<fT,sT>> {
		using result=fT;
	};
	template<int posit,typename T,typename Arg>struct change_type {
		//using result=void;
	};
	template<int posit,typename fT,typename sT,typename Arg>struct change_type<posit,tlist_node<fT,sT>,Arg> {
		using result=tlist_node<fT,typename change_type<posit-1,sT,Arg>::result>;
	};
	template<typename T,typename Arg>struct change_type<0,T,Arg> {
		using result=Arg;
	};
	template<typename fT,typename sT,typename Arg>struct change_type<0,tlist_node<fT,sT>,Arg> {
		using result=tlist_node<Arg,sT>;
	};
    template<typename T,typename Arg>struct push_front{
        using result=tlist_node<Arg,T>;
    };
    template<typename T>struct pop_front{
        //using result=void;
    };
    template<typename fT,typename sT>struct pop_front<tlist_node<fT,sT>>{
        using result=sT;
    };
	template<typename T,typename Arg>struct push_back{
		//using result=void;	
	};
	template<typename fT,typename sT,typename Arg>struct push_back<tlist_node<fT,sT>,Arg>{
		using result=tlist_node<fT,tlist_node<sT,Arg>>;
	};
	template<typename fT,typename sT,typename tT,typename Arg>struct push_back<tlist_node<fT,tlist_node<sT,tT>>,Arg>{
		using result=tlist_node<fT,typename push_back<tlist_node<sT,tT>,Arg>::result>;
	};
	template<typename T>struct pop_back{
		//using result=void;
	};
	template<typename fT,typename sT>struct pop_back<tlist_node<fT,sT>>{
		using result=fT;
	};
	template<typename fT,typename sT,typename tT>struct pop_back<tlist_node<fT,tlist_node<sT,tT>>>{
		using result=tlist_node<fT,typename pop_back<tlist_node<sT,tT>>::result>;
	};
	template<int posit,typename T>struct erase{
		//using result=void;
	};
	template<int posit,typename fT,typename sT>struct erase<posit,tlist_node<fT,sT>>{
		using result=tlist_node<fT,typename erase<posit-1,sT>::result>;
	};
	template<typename fT,typename sT>struct erase<0,tlist_node<fT,sT>>{
		using result=sT;
	};
	template<int posit,typename T,typename Arg>
}