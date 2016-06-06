#include "covstdlib.h"
#include <iostream>
#include <sstream>
using namespace cov;
template < typename T > const std::string& cov::toString(const T & obj)
{
    static std::string str;
    static std::stringstream ss;
    ss<<obj;
    ss>>str;
    return str;
}
int main()
{
    any txt;
    txt = 3.1415926;
    std::cout<<txt.toString() <<std::endl;
    txt = 123456;
    std::cout<<txt.toString() <<std::endl;
    std::cout<<clocks.measure(timer::timeUnit::milliSec,[&] {
        clocks.delay(clocks.timeUnit::milliSec, 100);
    }) <<std::endl;
    return 0;
}
