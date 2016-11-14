#include <iostream>
#include <fstream>
#include <string>
#include <forward_list>
#include <unordered_map>
#include "covstdlib.hpp"
typedef std::pair<std::string,int> pair;
int start()
{
	std::ifstream infs("./abc.txt");
	if(!infs) return -1;
	long words_count=0;
	std::unordered_map<std::string,int> mdata;
	std::forward_list<pair> ldata;
	std::string word;
	while(infs>>word)
	{
		++mdata[word];
		++words_count;
	}
	for(auto& it:mdata)
		ldata.push_front(it);
	ldata.sort([](const pair& a,const pair& b)->bool{return a.second<b.second;});
	for(auto& it:ldata)
		std::cout<<it.first<<'\t'<<it.second<<std::endl;
	std::cout<<"There have "<<words_count<<" words in total."<<std::endl;
	return 0;
}
int main()
{
	std::cout<<
	cov::timer::measure(cov::timer::time_unit::milli_sec,[]{start();})
	<< std::endl;
	return 0;
}