#include "covstdlib.hpp"
#include <iostream>
int test_count(0);
class test {
	int c=++test_count;
public:
	test()
	{
		std::cerr<<"Test Construct."<<c<<std::endl;
	}
	test(int a)
	{
		std::cerr<<"Test Construct with Args."<<c<<std::endl;
	}
	test(const test&)
	{
		std::cerr<<"Test Copy Construct."<<c<<std::endl;
	}
	~test()
	{
		std::cerr<<"Test Destroy."<<c<<std::endl;
	}
	void ttt()
	{
		std::cerr<<"Call once"<<c<<std::endl;
	}
};
void tt(const cov::shared_ptr<test> p)
{
	for(int i=0; i<10; ++i) {
		cov::shared_ptr<test> ssp=p;
		ssp->ttt();
	}
}
void start()
{
	cov::shared_ptr<test> sp(10);
	for(int i=0; i<10; ++i) {
		std::thread th(tt,sp);
		th.detach();
	}
}
int main()
{
	std::cerr<<"Start"<<std::endl;
	start();
	std::cerr<<"End"<<std::endl;
	while(true);
	return 0;
}