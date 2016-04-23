#ifndef GUARD_COVARIANT_STANDRAD_LIBRARY
#define GUARD_COVARIANT_STANDARD_LIBRARY 1

// Covariant C++ Library

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Copyright (C) 2016 Mike Covariant Lee(李登淳)
// Library Version:1.16.41

// Macro Definitions:
// __HINT__ Default definition,prompt some errors
// __DEBUG__ Show all the details of the error

#if __cplusplus < 201103L
#error This library needs your compiler support C++11(C++0x) or higher standard.
#else
#define __HINT__
#ifdef __DEBUG__
#include <cassert>
#define auto_assert(exp) assert(exp);if(!(exp))throw #exp;
#elif defined(__HINT__)
#define auto_assert(exp) if(!(exp))throw #exp;
#endif

#include <map>
#include <string>
#include <cstdio>
#include <thread>
#include <chrono>
#include <typeinfo>

namespace cov {
// 常用的几个小函数
	inline int strlen(const char *);
	inline int copyFile(const char *, const char *);
// I/O控制器
	namespace ioctrl {
		inline void flush();
		inline void endl();
	}
	inline void print(const char *);
	inline void print(void (*)());
	template < typename Type > inline void print(const Type &);
	template < typename Type, typename ... Argt >
	inline void print(const Type &, const Argt & ...);

	class baseHolder;
	template < typename T > class holder;
	class genericType;
	template < typename T > class handler;
	template < typename T > class list;
	class timer;
	typedef genericType any;
	typedef unsigned long timer_t;
	template < typename T > const std::string& toString(const T &)
	{
		static std::string str(__func__);
		str+=" is not defined.";
		return str;
	}
	template < typename T > static T *duplicate(const T & obj)
	{
		return new T(obj);
	}
}

// 获取字符串长度函数
inline int cov::strlen(const char *str)
{
	if (*str == '\0')
		return 0;
	int siz = 0;
	while (*(str + (++siz)));
	return siz;
}

// 文件复制函数
inline int cov::copyFile(const char *source, const char *target)
{
	static const int buff_size = 256;

    int retval = 0;
    FILE *in = nullptr;
    FILE *out = nullptr;
    try {
        if ((in = fopen(source, "rb")) == nullptr) {
            throw 1;
        }
        if ((out = fopen(target, "wb")) == nullptr) {
            throw 2;
        }

        char data[buff_size] = {0};
        size_t n = 0;

        while ((n = fread(data, 1, buff_size, in)) > 0) {
            if (fwrite(data, 1, n, out) != n) {
                throw 3;
            }
        }
    } catch (int e) {
        retval = e;
    }

    if (in) {
        fclose(in);
    }
    if (out) {
        fclose(out);
    }
	return retval;
}

namespace cov {
	template <> void print < int >(const int &i)
	{
		printf("%d", i);
	}
	template <> void print < double >(const double &d)
	{
		printf("%f", d);
	}
	template <> void print < char >(const char &c)
	{
		putchar(c);
	}
	template <> void print < std::string > (const std::string & s)
	{
		printf("%s", s.c_str());
	}
}
inline void cov::print(const char *s)
{
	print < std::string > (s);
}
inline void cov::print(void (*func) ())
{
	func();
}
inline void cov::ioctrl::flush()
{
	fflush(stdout);
}
inline void cov::ioctrl::endl()
{
	printf("\n");
	fflush(stdout);
}
template < typename Type > void cov::print(const Type & obj)
{
	return;
}
template < typename Type, typename...Argt >
void cov::print(const Type & obj, const Argt & ... argv)
{
	print(obj);
	print(argv...);
}

// 资源持有者基类
class cov::baseHolder {
public:
	baseHolder() = default;
	virtual ~ baseHolder() = default;
	virtual const std::type_info & type() const = 0;
	virtual baseHolder *duplicate() = 0;
	virtual bool compare(const baseHolder *) const = 0;
	virtual const std::string & toString() const = 0;
};
// 资源持有者派生类
template < typename T > class cov::holder:public baseHolder {
protected:
	T mDat;
public:
	holder() = default;
	holder(const T & dat):mDat(dat)
	{
	}
	virtual ~ holder() = default;
	virtual const std::type_info & type() const override
	{
		return typeid(T);
	}
	virtual baseHolder *duplicate() override
	{
		return new holder(mDat);
	}
	virtual bool compare(const baseHolder * obj)const override
	{
		if (obj->type() == this->type()) {
			const holder < T > *ptr = dynamic_cast < const holder < T > *>(obj);
			if (ptr == nullptr)
				return false;
			return mDat == ptr->data();
		} else
			return false;
	}
	virtual const std::string & toString() const override
	{
		return cov::toString(mDat);
	}
	T & data()
	{
		return mDat;
	}
	const T & data() const
	{
		return this->data();
	}
	void data(const T & dat)
	{
		mDat = dat;
	}
};
// 动态类型 Generic Type
class cov::genericType {
protected:
	baseHolder * mDat;
public:
	genericType():mDat(nullptr)
	{
	}
	template < typename T > genericType(const T & dat):mDat(new holder < T > (dat))
	{
	}
	genericType(const genericType & v):mDat(v.mDat->duplicate())
	{
	}
	~genericType()
	{
		delete mDat;
	}
	bool empty() const
	{
		return mDat != nullptr;
	}
	const std::type_info & type() const
	{
		if (this->mDat != nullptr)
			return this->mDat->type();
		else
			return typeid(void);
	}
	const std::string & toString() const
	{
		auto_assert(this->mDat != nullptr);
		return this->mDat->toString();
	}
	genericType & operator=(const genericType & var)
	{
		delete mDat;
		mDat = var.mDat->duplicate();
		return *this;
	}
	bool operator==(const genericType & var) const
	{
		return this->mDat->compare(var.mDat);
	}
	bool operator!=(const genericType & var)const
	{
		return !this->mDat->compare(var.mDat);
	}
	template < typename T > T & val()
	{
		auto_assert(typeid(T) == this->type() && this->mDat != nullptr);
		return dynamic_cast < holder < T > *>(this->mDat)->data();
	}
	template < typename T > const T & val() const
	{
		auto_assert(typeid(T) == this->type() && this->mDat != nullptr);
		return dynamic_cast < holder < T > *>(this->mDat)->data();
	}
	template < typename T > void assign(const T & dat)
	{
		if (typeid(T) != this->type() || this->mDat == nullptr) {
			delete mDat;
			mDat = new holder < T > (dat);
		}
		return dynamic_cast < holder < T > *>(mDat)->data(dat);
	}
	template < typename T > genericType & operator=(const T & dat)
	{
		assign(dat);
		return *this;
	}
};
namespace cov {
	template<> void print<genericType>(const genericType& var)
	{
		printf("%s",var.toString().c_str());
	}
}

// 内存管理 Memory Handler
template < typename T > class cov::handler {
protected:
	// Static引用计数数据库
	static std::map < T *, unsigned long >mRefDb;
	// 源数据指针
	T *mData;
	// 从数据库中删除数据
	void remove();
	// 处理数据回收
	void destory();
public:
	// 默认构造函数
	handler();
	// 自定义构造函数，以源数据为参数，不允许隐式调用
	handler(const T &);
	// 复制构造函数
	handler(const handler &);
	// 析构函数
	~handler();
	// 内存分配函数，原数据安全，会自动回收
	void alloc();
	// 内存释放函数，如引用计数大于1只会解除引用而不是删除数据
	void free();
	// 数据拷贝函数，手动将当前数据放置在独立的内存空间中
	// 拷贝后用户对内存空间的操作权限为读写
	// 如不进行拷贝用户将只能对共享内存空间进行读操作
	void clone();
	// 取值函数，可对指定内存空间进行写操作。如没有进行手动拷贝将自动进行拷贝
	T & modify();
	// 赋值函数，会为当前值开辟一个新空间。如不想开辟新空间请使用modify函数
	void assign(const T &);
	// 复制函数
	handler & operator=(const handler &);
	// 比较函数
	bool operator==(const handler &);
	bool operator!=(const handler &);
	// 布尔操作符函数，可获取当前数据可用状态
	operator  bool() const
	{
		return mData;
	}
	// 查询数据是否为NULL
	bool empty() const
	{
		return !mData;
	}
	// 以只读方式获取源数据指针
	const T *raw() const
	{
		return mData;
	}
	// 以只读方式获取源数据引用
	const T & data() const
	{
		auto_assert(!empty());
		return *mData;
	}
	// 获取当前数据的引用计数
	unsigned long refcount() const
	{
		if (!empty())
			return mRefDb[mData];
		else
			return 0;
	}
};
template < typename T > std::map < T *, unsigned long >cov::handler < T >::mRefDb;
template < typename T > void cov::handler < T >::remove()
{
	for (auto it = mRefDb.begin(); it != mRefDb.end(); ++it) {
		if (it->first == mData) {
			delete it->first;
			mRefDb.erase(it);
			break;
		}
	}
}
template < typename T > void cov::handler < T >::destory()
{
	if (!empty()) {
		if (--mRefDb[mData] == 0)
			remove();
	}
}
template < typename T > cov::handler < T >::handler():mData(0)
{
}
template < typename T > cov::handler < T >::handler(const T & obj):mData(duplicate(obj))
{
	if (mRefDb.find(mData) != mRefDb.end()) {
		++mRefDb[mData];
	} else {
		mRefDb[mData] = 1;
	}
}
template < typename T > cov::handler < T >::handler(const handler & obj):mData(obj.mData)
{
	++mRefDb[mData];
}
template < typename T > cov::handler < T >::~handler()
{
	destory();
}
template < typename T > void cov::handler < T >::alloc()
{
	destory();
	mData = new T;
	mRefDb[mData] = 1;
}
template < typename T > void cov::handler < T >::free()
{
	destory();
	mData = 0;
}
template < typename T > void cov::handler < T >::clone()
{
	T *newdt = cov::duplicate(*mData);
	destory();
	mData = newdt;
	mRefDb[mData] = 1;
}
template < typename T > T & cov::handler < T >::modify()
{
	if (mRefDb[mData] > 1)
		clone();
	return *mData;
}
template < typename T > void cov::handler < T >::assign(const T & obj)
{
	destory();
	mData = cov::duplicate(obj);
	mRefDb[mData] = 1;
}
template < typename T > cov::handler < T > &cov::handler < T >::operator=(const cov::handler < T >&obj)
{
	destory();
	mData = obj.mData;
	++mRefDb[mData];
	return *this;
}
template < typename T > bool cov::handler < T >::operator==(const cov::handler < T > &obj)
{
	return *mData == obj.data();
}
template < typename T > bool cov::handler < T >::operator!=(const cov::handler < T > &obj)
{
	return *mData != obj.data();
}

// 双向链表
template < typename T > class cov::list {
protected:
	class node;
	class base_iterator;
	node mFront;
	node mBack;
	unsigned long mSize = 0;
	void copy(const list & lst)
	{
		for (iterator it = lst.begin(); it != lst.end(); ++it)
			push_back(*it);
	}
public:
	class iterator;
	class reverse_iterator;
	list():mFront(nullptr, &mBack), mBack(&mFront, nullptr), mSize(0)
	{
	}
	list(const list & obj)
	{
		copy(obj);
	}
	list & operator=(const list & obj)
	{
		clear();
		copy(obj);
		return *this;
	}
	~list()
	{
		clear();
	}
	unsigned long size() const
	{
		return mSize;
	}
	bool empty() const
	{
		return !mFront.forward()->usable();
	}
	void clear()
	{
		for (iterator it = this->begin(); it != this->end();)
			it = this->erase(it);
	}
	void push_front(const T & dat)
	{
		node *newdat = new node(&mFront, new T(dat), mFront.forward());
		mFront.forward()->backward(newdat);
		mFront.forward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace_front(Args && ... args)
	{
		node *newdat = new node(&mFront, new T(args...), mFront.forward());
		mFront.forward()->backward(newdat);
		mFront.forward(newdat);
		++mSize;
	}
	void push_back(const T & dat)
	{
		node *newdat = new node(mBack.backward(), new T(dat), &mBack);
		mBack.backward()->forward(newdat);
		mBack.backward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace_back(Args && ... args)
	{
		node *newdat = new node(mBack.backward(), new T(args...), &mBack);
		mBack.backward()->forward(newdat);
		mBack.backward(newdat);
		++mSize;
	}
	void insert(base_iterator & it, const T & dat)
	{
		auto_assert(it.usable());
		node *current = it.mDat;
		node *newdat = new node(current->backward(), new T(dat), current);
		current->backward()->forward(newdat);
		current->backward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace(base_iterator & it, Args && ... args)
	{
		auto_assert(it.usable());
		node *current = it.mDat;
		node *newdat = new node(current->backward(), new T(args...), current);
		current->backward()->forward(newdat);
		current->backward(newdat);
		++mSize;
	}
	void pop_front()
	{
		if (mSize == 0)
			return;
		node *current = mFront.forward();
		current->forward()->backward(&mFront);
		mFront.forward(current->forward());
		delete current->data();
		delete current;
		--mSize;
	}
	void pop_back()
	{
		if (mSize == 0)
			return;
		node *current = mBack.backward();
		current->backward()->forward(&mBack);
		mBack.backward(current->backward());
		delete current->data();
		delete current;
		--mSize;
	}
	iterator erase(base_iterator & it)
	{
		auto_assert(it.usable());
		node *current = it.mDat;
		node *previous = current->backward();
		node *next = current->forward();
		if (previous != nullptr)
			previous->forward(next);
		if (next != nullptr)
			next->backward(previous);
		delete current->data();
		delete current;
		--mSize;
		return iterator(next);
	}
	iterator erase(base_iterator & start, base_iterator & finish)
	{
		for (iterator it = start; it != finish; it = erase(it));
		return erase(finish);
	}
	void remove(const T & dat)
	{
		for (iterator it = this->begin(); it != this->end();) {
			if (*it == dat)
				it = this->erase(it);
			else
				++it;
		}
	}
	template < typename __func > void remove_if(__func && condition)
	{
		for (iterator it = this->begin(); it != this->end();) {
			if (condition(*it))
				it = this->erase(it);
			else
				++it;
		}
	}
	void resize(unsigned long size)
	{
		while (mSize > size)
			pop_back();
		while (mSize < size)
			emplace_back();
	}
	T & front()
	{
		auto_assert(mFront.forward()->usable());
		return *mFront.forward()->data();
	}
	const T & front() const
	{
		return this->front();
	}
	T & back()
	{
		auto_assert(mBack.backward()->usable());
		return *mBack.backward()->data();
	}
	const T & back() const
	{
		return this->back();
	}
	iterator begin()
	{
		return iterator(mFront.forward());
	}
	iterator end()
	{
		return iterator(&mBack);
	}
	reverse_iterator rbegin()
	{
		return reverse_iterator(mBack.backward());
	}
	reverse_iterator rend()
	{
		return reverse_iterator(&mFront);
	}
};
//链表节点
template < typename T > class cov::list < T >::node {
protected:
	node * mFront;
	node *mBack;
	T *mDat;
public:
	node():mFront(nullptr), mBack(nullptr), mDat(nullptr)
	{
	}
	node(T * dat):mFront(nullptr), mBack(nullptr), mDat(dat)
	{
	}
	node(node * front, node * back):mFront(back), mBack(front), mDat(nullptr)
	{
	}
	node(node * front, T * dat, node * back):mFront(back), mBack(front), mDat(dat)
	{
	}
	node(const node & obj):mFront(obj.forward()), mBack(obj.backward()), mDat(obj.data())
	{
	}
	~node() = default;
	node *forward()
	{
		return mFront;
	}
	const node *forward() const
	{
		return mFront;
	}
	void forward(node * ptr)
	{
		mFront = ptr;
	}
	node *backward()
	{
		return mBack;
	}
	const node *backward() const
	{
		return mBack;
	}
	void backward(node * ptr)
	{
		mBack = ptr;
	}
	T *data()
	{
		return mDat;
	}
	const T *data() const
	{
		return mDat;
	}
	void data(T * dat)
	{
		mDat = dat;
	}
	bool usable() const
	{
		return mDat != nullptr;
	}
};
// 迭代器基类
template < typename T > class cov::list < T >::base_iterator {
protected:
	friend class list;
	node *mDat;
public:
	base_iterator() = delete;
	base_iterator(node * ptr):mDat(ptr)
	{
	}
	base_iterator(const base_iterator & it):mDat(it.mDat)
	{
	}
	virtual ~ base_iterator() = default;
	virtual T & data()
	{
		auto_assert(this->mDat != nullptr);
		auto_assert(this->mDat->usable());
		return *this->mDat->data();
	}
	virtual const T & data() const
	{
		return this->data();
	}
	virtual void data(const T & dat)
	{
		auto_assert(this->mDat != nullptr);
		if (this->mDat->usable()) {
			delete this->mDat->data();
			this->mDat->data(nullptr);
		}
		this->mDat->data(new T(dat));
	}
	virtual void forward() = 0;
	virtual void backward() = 0;
	virtual bool compare(const base_iterator & it) const
	{
		if (this->usable() && it.usable())
			return this->data() == it.data();
		else
			return this->mDat == it.mDat;
	}
	virtual bool usable() const
	{
		if (this->mDat != nullptr)
			return this->mDat->usable();
		else
			return false;
	}
	T & operator*()
	{
		return this->data();
	}
	const T & operator*() const
	{
		return this->data();
	}
	T *operator->()
	{
		return &this->data();
	}
	void operator++()
	{
		this->forward();
	}
	void operator--()
	{
		this->backward();
	}
	bool operator==(const base_iterator & it) const
	{
		return this->compare(it);
	}
	bool operator!=(const base_iterator & it)const
	{
		return !this->compare(it);
	}
};
// 正向迭代器
template < typename T > class cov::list < T >::iterator:public base_iterator {
	friend class list;
public:
	using base_iterator::base_iterator;
	iterator() = delete;
	virtual void forward() override
	{
		auto_assert(this->mDat != nullptr);
		this->mDat = this->mDat->forward();
	}
	virtual void backward() override
	{
		auto_assert(this->mDat != nullptr);
		this->mDat = this->mDat->backward();
	}
};
//反向迭代器
template < typename T > class cov::list < T >::reverse_iterator:public base_iterator {
	friend class list;
public:
	using base_iterator::base_iterator;
	reverse_iterator() = delete;
	virtual void forward() override
	{
		auto_assert(this->mDat != nullptr);
		this->mDat = this->mDat->backward();
	}
	virtual void backward() override
	{
		auto_assert(this->mDat != nullptr);
		this->mDat = this->mDat->forward();
	}
};

// 时钟 timer
class cov::timer {
	std::chrono::time_point < std::chrono::high_resolution_clock > m_timer;
public:
	enum timeUnit {
		nanoSec, microSec, milliSec, second, minute
	};
	timer():m_timer(std::chrono::high_resolution_clock::now())
	{
	}
	void reset()
	{
		m_timer = std::chrono::high_resolution_clock::now();
	}
	timer_t time(const int unit)
	{
		switch (unit) {
		case timeUnit::nanoSec:
			return std::chrono::duration_cast < std::chrono::nanoseconds >
			       (std::chrono::high_resolution_clock::now() - m_timer).count();
		case timeUnit::microSec:
			return std::chrono::duration_cast < std::chrono::microseconds >
			       (std::chrono::high_resolution_clock::now() - m_timer).count();
		case timeUnit::milliSec:
			return std::chrono::duration_cast < std::chrono::milliseconds >
			       (std::chrono::high_resolution_clock::now() - m_timer).count();
		case timeUnit::second:
			return std::chrono::duration_cast < std::chrono::seconds >
			       (std::chrono::high_resolution_clock::now() - m_timer).count();
		case timeUnit::minute:
			return std::chrono::duration_cast < std::chrono::minutes >
			       (std::chrono::high_resolution_clock::now() - m_timer).count();
		}
		return 0;
	}
	void delay(const int unit, timer_t time)
	{
		switch (unit) {
		case timeUnit::nanoSec:
			std::this_thread::sleep_for(std::chrono::nanoseconds(time));
			break;
		case timeUnit::microSec:
			std::this_thread::sleep_for(std::chrono::microseconds(time));
			break;
		case timeUnit::milliSec:
			std::this_thread::sleep_for(std::chrono::milliseconds(time));
			break;
		case timeUnit::second:
			std::this_thread::sleep_for(std::chrono::seconds(time));
			break;
		case timeUnit::minute:
			std::this_thread::sleep_for(std::chrono::minutes(time));
			break;
		}
	}
};
namespace cov {
	static timer clocks;
}

// Covariant Logic

namespace cov {
	namespace logic {
// LambdaBlock虚抽象基类
		class baseFunction {
		public:
			baseFunction() = default;
			virtual ~ baseFunction()
			{
			}
			virtual void perform() = 0;
		};
// Switcher虚抽象基类
		class baseSwitcher {
		public:
			baseSwitcher() = default;
			virtual ~ baseSwitcher()
			{
			}
			virtual void addDefault(baseFunction *) = 0;
			virtual const std::type_info & typeinf() = 0;
			virtual void perform() = 0;
		};
// LambdaBlock类，通过继承baseFunction来抽象包装
		template < typename T > class lambdaBlock:public baseFunction {
		protected:
			T mLambda;
		public:
			lambdaBlock() = delete;
			lambdaBlock(T lamb):mLambda(lamb)
			{
			}
			virtual void perform()
			{
				mLambda();
			}
		};
// Switcher类，通过继承baseSwitcher来抽象包装
		template < typename T > class switcher:public baseSwitcher {
		protected:
			const T & mCondition;
			baseFunction *mDefault = nullptr;
			std::map < const T, baseFunction * >mcases;
		public:
			switcher() = delete;
			switcher(const T & condition):mCondition(condition)
			{
			}
			virtual ~ switcher()
			{
				for (auto & it:mcases)
					delete it.second;
			}
			void addDefault(baseFunction * lambda) override
			{
				if (mDefault != nullptr)
					delete mDefault;
				mDefault = lambda;
			}
			const std::type_info & typeinf() override
			{
				return typeid(T);
			}
			void addMethod(const T & lable, baseFunction * lambda)
			{
				if (mcases.find(lable) != mcases.end())
					delete mcases[lable];
				mcases[lable] = lambda;
			}
			void perform() override
			{
				if (mcases.find(mCondition) == mcases.end()) {
					if (mDefault != nullptr)
						mDefault->perform();
				} else
					mcases[mCondition]->perform();
			}
		};
// Switcher栈类
		class switches {
		protected:
			cov::list < baseSwitcher * >mSwitches;
		public:
			void clear()
			{
				mSwitches.clear();
			}
			baseSwitcher *current()
			{
				return mSwitches.back();
			}
			void pop()
			{
				if (mSwitches.back() != nullptr)
					delete mSwitches.back();
				mSwitches.pop_back();
			}
			void push(baseSwitcher * s)
			{
				mSwitches.push_back(s);
			}
		};
// Switcher栈
		static switches switcher_stack;
		template < typename T > void checkType()
		{
			const std::type_info & current_switcher_type = switcher_stack.current()->typeinf();
			auto_assert(typeid(T) == current_switcher_type);
		}
// Lambda抽象对象获取函数
		template < typename T > lambdaBlock < T > *GetLambda(T lamb)
		{
			return new lambdaBlock < T > (lamb);
		}
// Switcher抽象对象获取函数
		template < typename T > baseSwitcher * GetSwitcher(const T & condition)
		{
			return new switcher < T > (condition);
		}
// Switcher Add Method抽象包装函数
		template < typename T > void addMethod(baseSwitcher * ptr, const T & obj,baseFunction * lambda)
		{
			checkType < T > ();
			switcher < T > *s = dynamic_cast < switcher < T > *>(ptr);
			if (s != nullptr)
				s->addMethod(obj, lambda);
		}
// Switcher抽象启动函数
		template < typename T > void runSwitcher(const T & switches)
		{
			switches();
		}
// const char*的特化处理
		baseSwitcher *GetSwitcher(const char *str)
		{
			return new switcher < std::string > (str);
		}
		void addMethod(baseSwitcher * ptr, const char *str, baseFunction * lambda)
		{
			checkType < std::string > ();
			switcher < std::string > *s = dynamic_cast < switcher < std::string > *>(ptr);
			if (s != nullptr)
				s->addMethod(std::string(str), lambda);
		}
	}
}

#define SWITCH(obj) cov::logic::runSwitcher([&]{cov::logic::switcher_stack.push(cov::logic::GetSwitcher(obj));
#define SWITCH_T(typ,obj) cov::logic::runSwitcher([&]{cov::logic::switcher_stack.push(cov::logic::GetSwitcher<typ>(obj));
#define ENDSWITCH cov::logic::switcher_stack.current()->perform();cov::logic::switcher_stack.pop();});
#define CASE(obj) cov::logic::addMethod(cov::logic::switcher_stack.current(),obj,cov::logic::GetLambda([&]{
#define CASE_T(typ,obj) cov::logic::checkType<typ>();cov::logic::addMethod<typ>(cov::logic::switcher_stack.current(),obj,cov::logic::GetLambda([&]{
#define DEFAULT cov::logic::switcher_stack.current()->addDefault(cov::logic::GetLambda([&]{
#define BREAK }));

namespace cov {
// Tuple List尾部节点
	struct final_tuple_node {
	};
// Tuple List节点
	template < typename _fT, typename _sT > struct tuple_node {
		typedef _fT type;
		typedef _sT forward_type;
		_fT current;
		_sT forward;
	};
// Tuple List元素
	template < int N, typename _Tp > struct tuple_element;
// Tuple List元素 模板元递归终点
	template < typename _cT, typename _fT > struct tuple_element <1, tuple_node < _cT, _fT >> {
		typedef _cT type;
		typedef tuple_node < _cT, _fT > node_type;
		static type & get(node_type & _t)
		{
			return _t.current;
		}
		static const type & get(const node_type & _t)
		{
			return _t.current;
		}
		static void set(const type & _dat, node_type & _t)
		{
			_t.current = _dat;
		}
	};
// Tuple List元素 模板元递归终点
	template < typename _cT, typename _fT, typename _sT >
	struct tuple_element <1, tuple_node < _cT, tuple_node < _fT, _sT >>> {
		typedef _cT type;
		typedef tuple_node < _cT, tuple_node < _fT, _sT >> node_type;
		static type & get(node_type & _t)
		{
			return _t.current;
		}
		static const type & get(const node_type & _t)
		{
			return _t.current;
		}
		static void set(const type & _dat, node_type & _t)
		{
			_t.current = _dat;
		}
	};
// Tuple List元素
	template < int N, typename _cT, typename _fT, typename _sT >
	struct tuple_element <N, tuple_node < _cT, tuple_node < _fT, _sT >>> {
		typedef typename tuple_element < N - 1, tuple_node < _fT, _sT >>::type type;
		typedef tuple_node < _cT, tuple_node < _fT, _sT >> node_type;
		static type & get(node_type & _t)
		{
			return tuple_element < N - 1, tuple_node < _fT, _sT >>::get(_t.forward);
		}
		static const type & get(const node_type & _t)
		{
			return tuple_element < N - 1, tuple_node < _fT, _sT >>::get(_t.forward);
		}
		static void set(const type & _dat, node_type & _t)
		{
			tuple_element < N - 1, tuple_node < _fT, _sT >>::set(_dat, _t.forward);
		}
	};
// Tuple List包装器
	template < typename...Elements > struct tuple_wrapper;
// Tuple List包装器 模板元递归终点
	template < typename T > struct tuple_wrapper <T > {
		typedef tuple_node < T, final_tuple_node > type;
	};
// Tuple List包装器
	template < typename T, typename...Elements > struct tuple_wrapper <T, Elements ... > {
		typedef tuple_node < T, typename tuple_wrapper < Elements ... >::type > type;
	};
	template <typename...Elements> using tuple=typename tuple_wrapper<Elements...>::type;
// Tuple List大小获取函数
	template < typename T > int tuple_size(const T & _t, int count = 0)
	{
		return tuple_size(_t.forward, ++count);
	}
	template <> int tuple_size < final_tuple_node > (const final_tuple_node & _t, int count)
	{
		return count;
	}
// Tuple List填充函数
	void fill_tuple(final_tuple_node & _t)
	{
	}
	template < typename T, typename...Elements >
	void fill_tuple(tuple<T,Elements...> & _t, const T & dat,Elements ... args)
	{
		_t.current = dat;
		fill_tuple(_t.forward, args...);
	}
// Tuple List制作函数
	template < typename...Argt > tuple<Argt...> make_tuple(Argt...args)
	{
		tuple<Argt...> ret;
		fill_tuple(ret, args...);
		return ret;
	}
// Tuple List元素获取函数
	template < int N, typename _fT, typename _sT >
	typename tuple_element < N, tuple_node < _fT, _sT >>::type &get(tuple_node < _fT, _sT > &_t)
	{
		return tuple_element < N, tuple_node < _fT, _sT >>::get(_t);
	}
// for each函数中转
	template < typename T > void for_each_process(T &);
// for each函数
	void for_each(const final_tuple_node & _t)
	{
	}
	template < typename _fT, typename _sT > void for_each(const tuple_node < _fT, _sT > &_t)
	{
		for_each_process(_t.current);
		for_each(_t.forward);
	}
}

#endif
#endif