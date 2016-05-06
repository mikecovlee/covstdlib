#include "covstdlib.h"
#include <stdexcept>
#include <cstdlib>
#include <cmath>
namespace cov {
	namespace expr {
		static std::map < std::string, any > database;
		any getvar(const std::string & name)
		{
			return database[name];
		}
		typedef cov::any(*_expr_func_) (cov::list < cov::any > &);
		cov::any call_func(cov::any &, const std::string &);
		double compute(const std::string & exp)
		{
			bool reverse = false;
			cov::list < double >nums;
			cov::list < char >operators;
			std::string tmp;
			for (int i = 0; i < exp.size();) {
				if (std::isspace(exp[i])) {
					++i;
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
				if (std::ispunct(exp[i])) {
					if (nums.empty()) {
						switch (exp[i]) {
						case '+':
							reverse = false;
							break;
						case '-':
							reverse = true;
							break;
						default:
							throw std::logic_error("Operator does not recognize.");
						}
						++i;
						continue;
					}
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
				if (std::isalpha(exp[i])) {
					tmp.clear();
					for (; i < exp.size() && (std::isalnum(exp[i]) || exp[i] == '_'); ++i)
						tmp += exp[i];
					any var = getvar(tmp);
					if (var.type() == typeid(_expr_func_)) {
						std::string::size_type begin(exp.find('(') + 1), end(exp.rfind(')'));
						if (begin == std::string::npos || end == std::string::npos)
							throw std::logic_error("The lack of corresponding brackets.");
						nums.push_back(call_func(var, exp.substr(begin, end - begin)).val < double >());
						i = end + 1;
						continue;
					}
					nums.push_back(var.val < double >());
					continue;
				}
				throw std::logic_error("Operator does not recognize.");
			}
			if (nums.empty())
				return -1;
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
				case '^': {
					if (right != 0)
						right = std::pow(right, current);
					else
						left = std::pow(left, current);
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
			if (reverse)
				return -left;
			else
				return left;
		}
		cov::any call_func(cov::any & f, const std::string & args)
		{
			_expr_func_ func = f.val < _expr_func_ > ();
			cov::list < cov::any > arguments;
			std::string tmp;
			for (int i = 0; i < args.size(); ++i) {
				if (args[i] != ',') {
					tmp += args[i];
				} else {
					arguments.push_back(compute(tmp));
					tmp.clear();
				}
			}
			arguments.push_back(compute(tmp));
			return func(arguments);
		}
	}
}

cov::any abs(cov::list < cov::any > &args)
{
	double val = args.front().val < double >();
	if (val < 0)
		return -val;
	else
		return val;
}

cov::any pow(cov::list < cov::any > &args)
{
	return std::pow(args.front().val < double >(), args.back().val < double >());
}

#include <iostream>
int main()
{
	cov::any var;
	var.assign < cov::expr::_expr_func_ > (abs);
	cov::expr::database["abs"] = var;
	var.assign < cov::expr::_expr_func_ > (pow);
	cov::expr::database["pow"] = var;
	cov::expr::database["x"] = 3.14;
	std::string line;
	std::getline(std::cin, line);
	try {
		std::cout << cov::expr::compute(line);
	} catch(const char *str) {
		printf("%s", str);
	} catch(std::logic_error e) {
		printf("%s", e.what());
	}
}