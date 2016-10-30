#include "covstdlib.hpp"
#include <iostream>
class interger:public cov::object
{
    int mDat=0;
    public:
    interger()=default;
    interger(int i):mDat(i){}
    virtual object* clone() noexcept override
    {
        return new interger(*this);
    }
    operator int() const
    {
        return mDat;
    }
};
int main()
{
    const cov::tuple<cov::object*,float,bool,int> t(new interger(10),3.14,false,12);
    cov::any a=dynamic_cast<const interger*>(t.get<cov::object*>())->operator int();
    std::cout<<a.type().name()<<std::endl<<a<<std::endl;
    delete t.get<0>();
    return 0;
}