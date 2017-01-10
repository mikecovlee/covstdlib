#include "./type_list.hpp"
#include <typeinfo>
#include <cstdio>
int main()
{
    using A=typename cov::make_tlist<void,void,float,void>::result;
    printf("%d\n",cov::get_size<A>::result);
    using B=typename cov::erase<3,A>::result;
    printf("%d\n",cov::get_size<B>::result);
    printf("%s\n",typeid(B).name());
    return 0;
}