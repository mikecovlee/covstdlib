#include <iostream>
#include <cstdio>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <list>
typedef double number_t;
typedef std::array<number_t,2> posit_t;
static constexpr number_t prec=0.000000001;
static constexpr number_t block_size=0.01;
inline number_t distance(const posit_t& a,const posit_t& b)
{
	return std::sqrt(std::pow(a.at(0)-b.at(0),2)+std::pow(a.at(1)-b.at(1),2));
}
void compute_pi()
{
	int thread_count(0);
	int finish_count(0);
	number_t final_result(0);
	std::mutex locker;
	std::list<std::thread*> threads;
	auto func0=[&](number_t range_s,number_t range_e) {
		locker.lock();
		++thread_count;
		int this_thread_id=thread_count;
		std::cerr<<"Thread joined:"<<this_thread_id<<std::endl;
		locker.unlock();
		number_t result(0);
		for(number_t start(range_s); start<=range_e-prec; start+=prec)
			result+=std::sqrt(1-std::pow(start+0.5*prec,2))*prec;
		locker.lock();
		final_result+=result;
		std::cerr<<"Thread finished:"<<this_thread_id<<" Total finished:"<<finish_count+1<<" Total Thread:"<<thread_count<<std::endl;
		++finish_count;
		locker.unlock();
	};
	for(number_t start=-1.0; start<1.0; start+=block_size) {
		std::thread* th0=new std::thread(func0,start,start+block_size);
		threads.push_back(th0);
	}
	for(auto&it:threads)
		it->detach();
	while(finish_count<thread_count);
	for(auto&it:threads)
		delete it;
	std::cout.precision(30);
	std::cout<<"Pi="<<final_result*2<<std::endl;
}
int main()
{
	compute_pi();
	return 0;
}