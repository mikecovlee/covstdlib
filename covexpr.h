#include "covstdlib.h"
#include <stdexcept>
#include <cstdlib>
namespace cov {
	namespace expr {
		double compute(const std::string & exp)
		{
			cov::list < double >nums;
			cov::list < char >operators;
			std::string tmp;
			for (int i = 0; i < exp.size();) {
				if (std::isspace(exp[i])) {
					++i;
					continue;
				}
				if (std::ispunct(exp[i])) {
					operators.push_back(exp[i]);
					++i;
					continue;
				}
				if (std::isdigit(exp[i]) || exp[i] == '.') {
					tmp.clear();
					for (; i < exp.size() && (isdigit(exp[i]) || exp[i] == '.'); ++i)
						tmp += exp[i];
					nums.push_back(atof(tmp.c_str()));
					continue;
				}
				if (exp[i] == '(') {
					std::string::size_type begin(exp.find('(') + 1), end(exp.rfind(')'));
					if (begin == std::string::npos || end == std::string::npos)
						throw std::logic_error("The lack of corresponding brackets.");
					nums.push_back(compute(exp.substr(begin, end - begin)));
					i = end + 1;
					continue;
				}
				throw std::logic_error("Operator does not recognize.");
			}
			float left = nums.front();
			nums.pop_front();
			float right = 0;
			char __operator = 0;
			for (auto & current:nums) {
				switch (operators.front()) {
				case '+': {
					if (right != 0) {
						switch (__operator) {
						case '+':
							left += right;
							break;
						case '-':
							left -= right;
							break;
						}
					}
					right = current;
					__operator = '+';
					break;
				}
				case '-': {
					if (right != 0) {
						switch (__operator) {
						case '+':
							left += right;
							break;
						case '-':
							left -= right;
							break;
						}
					}
					right = current;
					__operator = '-';
					break;
				}
				case '*': {
					if (right != 0)
						right *= current;
					else
						left *= current;
					break;
				}
				case '/': {
					if (right != 0)
						right /= current;
					else
						left /= current;
					break;
				}
				default:
					throw std::logic_error("Operator does not recognize.");
				}
				operators.pop_front();
			}
			switch (__operator) {
			case '+':
				return left + right;
			case '-':
				return left - right;
			}
			return left;
		}
	}
}