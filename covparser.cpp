#include "covany.h"
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string>
#include <cmath>
#include <list>
#include <map>
namespace Parser {
class Variable {
public:
	typedef cov::any Var;
	typedef float Number;
	typedef bool Boolean;
	typedef std::string String;
	typedef cov::any(*Function) (std::vector < cov::any > &);
private:
	static std::map < String, Var > mData;
public:
	static bool haveVar(const String &);
	static void inferVar(const String &, const String &);
	static void delVar(const String &);
	static Var callFunc(const String &, std::vector < Var > &);
	template < typename T > static bool checkType(const String &);
	template < typename T > static void addVar(const String &, T);
	template < typename T > static T getVar(const String &);
};

class Expression {
private:
	static const long Precision = 10000;
	static void cutPrecision(int, Variable::Number &);
public:
	static Variable::Number compute(const Variable::String &);
};

std::map < Variable::String, Variable::Var > Variable::mData;

bool Variable::haveVar(const String & name)
{
	return mData.find(name) != mData.end();
}

void Variable::delVar(const String & name)
{
	if (haveVar(name))
		mData.erase(mData.find(name));
}

template < typename T > bool Variable::checkType(const String & name)
{
	if (!haveVar(name))
		return false;
	return mData[name].type() == typeid(T);
}

template <> void Variable::addVar < Variable::Number > (const String & name, Number var)
{
	mData[name] = var;
}

template <> void Variable::addVar < Variable::Boolean > (const String & name, Boolean var)
{
	mData[name] = var;
}

template <> void Variable::addVar < Variable::String > (const String & name, String str)
{
	mData[name] = str;
}

template <> void Variable::addVar < Variable::Function > (const String & name, Function func)
{
	mData[name] = func;
}

template <> Variable::Number Variable::getVar < Variable::Number > (const String & name)
{
	if (!checkType < Number > (name))
		throw std::logic_error("Variable Type Error.");
	return mData[name].val < Number > ();
}

template <> Variable::Boolean Variable::getVar < Variable::Boolean > (const String & name)
{
	if (!checkType < Boolean > (name))
		throw std::logic_error("Variable Type Error.");
	return mData[name].val < Boolean > ();
}

template <> Variable::String Variable::getVar < Variable::String > (const String & name)
{
	if (!checkType < String > (name))
		throw std::logic_error("Variable Type Error.");
	return mData[name].val < String > ();
}

template <> Variable::Function Variable::getVar < Variable::Function > (const String & name)
{
	if (!checkType < Function > (name))
		throw std::logic_error("Variable Type Error.");
	return mData[name].val < Function > ();
}

Variable::Var Variable::callFunc(const String & name, std::vector < Var > &arglist)
{
	return getVar < Function > (name) (arglist);
}

void Variable::inferVar(const String & name, const String & val)
{
	if (val == "true" || val == "True" || val == "TRUE") {
		mData[name] = true;
		return;
	}
	if (val == "false" || val == "False" || val == "FALSE") {
		mData[name] = false;
		return;
	}
	try {
		mData[name] = Expression::compute(val);
	} catch(...) {
		mData[name] = val;
	}
}

void Expression::cutPrecision(int precision, Variable::Number & num)
{
	num *= precision;
	num = (long)num;
	num /= precision;
}

Variable::Number Expression::compute(const Variable::String & exp)
{
	bool reverse = false;
	std::list < Variable::Number > nums;
	std::list < char >operators;
	std::string tmp;
	for (int i = 0; i < exp.size();) {
		if (std::isspace(exp[i])) {
			++i;
			continue;
		}
		if (exp[i] == '(') {
			int level(1), pos(++i);
			for (; pos < exp.size() && level > 0; ++pos) {
				if (exp[pos] == '(')
					++level;
				if (exp[pos] == ')')
					--level;
			}
			if (level > 0)
				throw std::logic_error("The lack of corresponding brackets.");
			nums.push_back(compute(exp.substr(i, pos - i - 1)));
			i = pos;
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
			if (Variable::checkType < Variable::Function > (tmp)) {
				int level(1), pos(++i);
				for (; pos < exp.size() && level > 0; ++pos) {
					if (exp[pos] == '(')
						++level;
					if (exp[pos] == ')')
						--level;
				}
				if (level > 0)
					throw std::logic_error("The lack of corresponding brackets.");
				std::string arglist = exp.substr(i, pos - i - 1);
				std::string temp;
				std::vector < cov::any > args;
				for (int i = 0; i < arglist.size(); ++i) {
					if (arglist[i] != ',') {
						temp += arglist[i];
					} else {
						args.push_back(compute(temp));
						temp.clear();
					}
				}
				args.push_back(compute(temp));
				nums.push_back(Variable::callFunc(tmp, args).val < Variable::Number > ());
				i = pos;
				continue;
			}
			nums.push_back(Variable::getVar < Variable::Number > (tmp));
			continue;
		}
		throw std::logic_error("Operator does not recognize.");
	}
	if (nums.empty())
		return -1;
	Variable::Number left = nums.front();
	Variable::Number right = 0;
	char signal = 0;
	nums.pop_front();
	for (auto & current:nums) {
		switch (operators.front()) {
		case '+': {
			if (right != 0) {
				switch (signal) {
				case '+':
					left += right;
					break;
				case '-':
					left -= right;
					break;
				}
			}
			right = current;
			signal = '+';
			break;
		}
		case '-': {
			if (right != 0) {
				switch (signal) {
				case '+':
					left += right;
					break;
				case '-':
					left -= right;
					break;
				}
			}
			right = current;
			signal = '-';
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
	Variable::Number result = 0;
	switch (signal) {
	case '+':
		result = left + right;
		break;
	case '-':
		result = left - right;
		break;
	default:
		result = left;
		break;
	}
	cutPrecision(Precision, result);
	if (reverse)
		result = -result;
	return result;
}
}

cov::any abs(std::vector < cov::any > &args)
{
	return std::abs(args.front().val < Parser::Variable::Number > ());
}

cov::any sin(std::vector < cov::any > &args)
{
	return std::sin(args.front().val < Parser::Variable::Number > ());
}

cov::any cos(std::vector < cov::any > &args)
{
	return std::cos(args.front().val < Parser::Variable::Number > ());
}

cov::any tan(std::vector < cov::any > &args)
{
	return std::tan(args.front().val < Parser::Variable::Number > ());
}

cov::any sqrt(std::vector < cov::any > &args)
{
	return std::sqrt(args.front().val < Parser::Variable::Number > ());
}

cov::any pow(std::vector < cov::any > &args)
{
	return std::pow(args.front().val < Parser::Variable::Number > (),args.back().val < Parser::Variable::Number > ());
}

#include <iostream>

int main()
{
	Parser::Variable::addVar < Parser::Variable::Function > ("abs", abs);
	Parser::Variable::addVar < Parser::Variable::Function > ("sin", sin);
	Parser::Variable::addVar < Parser::Variable::Function > ("cos", cos);
	Parser::Variable::addVar < Parser::Variable::Function > ("tan", tan);
	Parser::Variable::addVar < Parser::Variable::Function > ("sqrt", sqrt);
	Parser::Variable::addVar < Parser::Variable::Function > ("pow", pow);
	Parser::Variable::addVar < Parser::Variable::Number > ("pi", 3.141);
	std::string line;
	while (true) {
		std::getline(std::cin, line);
		if (line.find('=') != std::string::npos)
			Parser::Variable::inferVar(line.substr(0, line.find('=')), line.substr(line.find('=') + 1));
		else
			std::cout << Parser::Expression::compute(line) << std::endl;
	}
	return 0;
}