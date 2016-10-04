#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include "covstdlib.hpp"
namespace cov {
	namespace db {
		namespace type {
			typedef double NUM;
			typedef int INT;
			typedef float FLT;
			typedef std::string STR;
			typedef std::vector<int> ARR;
		}
		enum class access_type {
			preload,
			real_time,
			stepper
		};
		template<typename _Key,typename _Val>
		class db_element_base final {
			typedef std::map<_Key,_Val> data_type;
			data_type mDat;
		public:
			db_element_base()=default;
			db_element_base(const data_type& dat):mDat(dat) {}
			~db_element_base()=default;
			bool empty() const
			{
				return mDat.empty();
			}
			bool exist(const _Key& key) const
			{
				return mDat.find(key)!=mDat.end();
			}
			std::size_t size() const
			{
				return mDat.size();
			}
			void clear()
			{
				mDat.clear();
			}
			void insert(const _Key& key,const _Val& value)
			{
				if(exist(key))
					mDat.at(key)=value;
				else
					mDat.insert(std::make_pair(key,value));
			}
			void erase(typename data_type::const_iterator posit)
			{
				mDat.erase(posit);
			}
			std::size_t erase(const _Key& key)
			{
				return mDat.erase(key);
			}
			typename data_type::iterator begin()
			{
				return std::move(mDat.begin());
			}
			typename data_type::const_iterator begin() const
			{
				return std::move(mDat.cbegin());
			}
			typename data_type::const_iterator cbegin() const
			{
				return std::move(mDat.cbegin());
			}
			typename data_type::iterator end()
			{
				return std::move(mDat.end());
			}
			typename data_type::const_iterator end() const
			{
				return std::move(mDat.cend());
			}
			typename data_type::const_iterator cend() const
			{
				return std::move(mDat.cend());
			}
			_Val& at(const _Key& key)
			{
				if(mDat.find(key)==mDat.end())
					throw std::logic_error("E000C");
				return mDat.at(key);
			}
			const _Val& at(const _Key& key) const
			{
				if(mDat.find(key)==mDat.end())
					throw std::logic_error("E000C");
				return mDat.at(key);
			}
		};
		typedef db_element_base<std::string,cov::any> db_element;
		typedef db_element_base<std::string,db_element> database;
		const std::string db_version="1.16.09.01";
		void save_to_file(const std::string& file,const database& db)
		{
			std::ofstream outfs(file);
			outfs<<"#COV_DATABASE?"<<db_version<<std::endl;
			for(const auto& element:db) {
				outfs<<"#DATA?"<<element.first<<':';
				for(const auto& it:element.second) {
					outfs<<it.first;
					if(it.second.type()==typeid(type::INT)) {
						outfs<<"?INT="<<it.second.to_string()<<';';
						continue;
					}
					if(it.second.type()==typeid(type::FLT)) {
						outfs<<"?FLT="<<it.second.to_string()<<';';
						continue;
					}
					if(it.second.type()==typeid(type::STR)) {
						outfs<<"?STR="<<it.second.to_string()<<';';
						continue;
					}
					if(it.second.type()==typeid(type::ARR)) {
						outfs<<"?ARR={";
						type::ARR array=it.second.val<typename type::ARR>();
						for(auto it=array.begin(); it!=array.end()-1; ++it) {
							outfs<<*it<<',';
						}
						outfs<<array.back()<<"};";
						continue;
					}
					std::cerr<<it.second.type().name()<<std::endl;
					throw std::logic_error("Invaild Data Type.");
				}
				outfs<<std::endl;
			}
			outfs<<"#END_COV_DATABASE?Null";
			std::cout<<"Load file Finished."<<std::endl;
		}
		void parse_value(const std::string& val,const std::string& type,cov::any& value)
		{
			Switch(type) {
				Case("INT") {
					value=std::stoi(val);
				}
				EndCase;
				Case("FLT") {
					value=std::stof(val);
				}
				EndCase;
				Case("NUM") {
					value=std::stof(val);
				}
				EndCase;
				Case("STR") {
					value=val;
				}
				EndCase;
				Case("ARR") {
					std::string tmp;
					std::vector<int> array;
					for(const auto& it:val) {
						switch(it) {
						default:
							tmp+=it;
							break;
						case '{':
							break;
						case '}':
							array.push_back(std::stoi(tmp));
							break;
						case ',':
							array.push_back(std::stoi(tmp));
							tmp.clear();
							break;
						}
					}
					value=array;
				}
				EndCase;
			}
			EndSwitch;
		}
		void load_from_file(const std::string& file,database& db)
		{
			std::ifstream infs(file);
			if(!infs.is_open())
				throw std::logic_error("Invaild File Path.");
			std::vector<char> buffer;
			char c;
			while(infs>>c) {
				buffer.push_back(c);
			}
			int level_count=0;
			std::string temp,head,key,head_type("Null"),value_type;
			cov::any value;
			db_element data;
			for(const auto& ch:buffer) {
				switch(ch) {
				default:
					if(head_type=="DATA" || head_type=="Null")
						temp+=ch;
					break;
				case '#':
					if(head_type=="DATA") {
						db.insert(head,data);
						data.clear();
					}
					level_count=0;
					head_type="Null";
					break;
				case '?':
					switch(level_count) {
					case 0:
						head_type=temp;
						temp.clear();
						++level_count;
						break;
					case 1:
						key=temp;
						temp.clear();
						break;
					}
					break;
				case ':':
					head=temp;
					temp.clear();
					break;
				case '=':
					value_type=temp;
					temp.clear();
					break;
				case ';':
					parse_value(temp,value_type,value);
					data.insert(key,value);
					temp.clear();
					break;
				}
			}
		}
		std::string replace_variable(const std::string& str,const db_element& data)
		{
			std::string nstr,tmp;
			bool reading_var_name=false;
			for(const auto& it:str) {
				if(it=='%') {
					if(reading_var_name)
						nstr+=data.at(tmp).to_string();
					else
						reading_var_name=true;
					continue;
				}
				if(reading_var_name)
					tmp+=it;
				else
					nstr+=it;
			}
			return nstr;
		}
		void print_database(const database& db)
		{
			for(const auto& data:db) {
				std::cout<<"Database Head="<<data.first<<std::endl;
				for(const auto& element:data.second) {
					std::cout<<"Database Value Name="<<element.first;
					if(element.second.type()==typeid(type::ARR)) {
						std::cout<<" Array Elements:{ ";
						for(const auto& it:element.second.val<typename type::ARR>()) {
							std::cout<<it<<' ';
						}
						std::cout<<'}'<<std::endl;
					} else {
						if(element.second.type()==typeid(type::STR))
							std::cout<<" Value="<<replace_variable(element.second.val<std::string>(),db.at("VARIABLES"))<<std::endl;
						else
							std::cout<<" Value="<<element.second<<std::endl;
					}
				}
			}
			std::cout<<"Print Finished"<<std::endl;
		}
	}
}

/*int main()
{
	std::string cmd;
	std::string key;
	cov::db::db_element db;
	cov::db::database data;
	cov::db::load_form_file("./testdb.sav",data);
	print_database(data);
	cov::db::save_to_file("./testdb0.sav",data);
	cov::db::load_form_file("./testdb0.sav",data);
	print_database(data);
	while(true) {
		std::cin>>cmd;
		Switch(cmd) {
			Case("INT") {
				int in;
				std::cin>>key>>in;
				db.insert(key,in);
			}
			EndCase;
			Case("FLT") {
				double in;
				std::cin>>key>>in;
				db.insert(key,in);
			}
			EndCase;
			Case("STR") {
				std::string in;
				std::cin>>key>>in;
				db.insert(key,in);
			}
			EndCase;
			Case("DEL") {
				std::cin>>key;
				std::cout<<"Erase "<<db.erase(key)<<" Elements."<<std::endl;
			}
			EndCase;
			Case("SAVE") {
				std::string in;
				std::cin>>key>>in;
				data.insert(key,db);
				cov::db::save_to_file(in,data);
			}
			EndCase;
		}
		EndSwitch;
		for(const auto& it:db) {
			std::cout<<"KEY="<<it.first<<" VAL="<<it.second<<std::endl;
		}
	}
}*/