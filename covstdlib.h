#pragma once

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
// Library Version: 1.16.08.01

#if __cplusplus < 201103L
#error Covariant C++ Library需要您的编译器支持C++11(C++0x)或者更高标准。请检查您否忘记了[-std=c++11]编译选项。
#else

#include <map>
#include <utility>
#include <string>
#include <deque>
#include <thread>
#include <chrono>
#include <sstream>
#include <ostream>
#include <memory>
#include <typeinfo>
#include <stdexcept>
#include <typeindex>

namespace cov {
	class any;
	template < typename T > class handler;
	template < typename T > class list;
	class timer;
	class argument_list;
	typedef unsigned long timer_t;
	template < typename T > const std::string& toString(const T & val)
	{
		static std::string str;
		std::stringstream ss;
		ss<<val;
		ss>>str;
		return str;
	}
	template < typename T > static T *duplicate(const T & obj)
	{
		return new T(obj);
	}
}

// 动态类型 Any
class cov::any final {
protected:
	// 资源持有者基类
	class baseHolder {
	public:
		baseHolder() = default;
		virtual ~ baseHolder() = default;
		virtual const std::type_info & type() const = 0;
		virtual baseHolder *duplicate() = 0;
		virtual bool compare(const baseHolder *) const = 0;
		virtual const std::string & toString() const = 0;
	};
	// 资源持有者派生类
	template < typename T > class holder final:public baseHolder {
	protected:
		T mDat;
	public:
		holder() = default;
		holder(const T & dat):mDat(dat) {}
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
	baseHolder * mDat;
public:
	any():mDat(nullptr) {}
	template < typename T > any(const T & dat):mDat(new holder < T > (dat)) {}
	any(const any & v):mDat(v.mDat->duplicate()) {}
	any(any&& v):mDat(v.mDat->duplicate()) {}
	~any()
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
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		return this->mDat->toString();
	}
	any & operator=(const any & var)
	{
		delete mDat;
		mDat = var.mDat->duplicate();
		return *this;
	}
	bool operator==(const any & var) const
	{
		return this->mDat->compare(var.mDat);
	}
	bool operator!=(const any & var)const
	{
		return !this->mDat->compare(var.mDat);
	}
	template < typename T > T & val()
	{
		if(typeid(T) != this->type())
			throw std::logic_error("请求的类型与对象类型不同");
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		return dynamic_cast < holder < T > *>(this->mDat)->data();
	}
	template < typename T > const T & val() const
	{
		if(typeid(T) != this->type())
			throw std::logic_error("请求的类型与对象类型不同");
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		return dynamic_cast < holder < T > *>(this->mDat)->data();
	}
	template < typename T > operator T&()
	{
		return this->val<T>();
	}
	template < typename T > operator const T&() const
	{
		return this->val<T>();
	}
	template < typename T > void assign(const T & dat)
	{
		if (typeid(T) != this->type() || this->mDat == nullptr) {
			delete mDat;
			mDat = new holder < T > (dat);
		}
		return dynamic_cast < holder < T > *>(mDat)->data(dat);
	}
	template < typename T > any & operator=(const T & dat)
	{
		assign(dat);
		return *this;
	}
};

std::ostream& operator<<(std::ostream& out,const cov::any& val)
{
	out<<val.toString();
	return out;
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
		if(empty())
			throw std::logic_error("使用了未初始化的对象");
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
		if(!it.usable())
			throw std::logic_error("使用了未初始化的对象");
		node *current = it.mDat;
		node *newdat = new node(current->backward(), new T(dat), current);
		current->backward()->forward(newdat);
		current->backward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace(base_iterator & it, Args && ... args)
	{
		if(!it.usable())
			throw std::logic_error("使用了未初始化的对象");
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
		if(!it.usable())
			throw std::logic_error("使用了未初始化的对象");
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
		if(!mFront.forward()->usable())
			throw std::logic_error("使用了未初始化的对象");
		return *mFront.forward()->data();
	}
	const T & front() const
	{
		return this->front();
	}
	T & back()
	{
		if(!mBack.backward()->usable())
			throw std::logic_error("使用了未初始化的对象");
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
		if(this->mDat == nullptr || !this->mDat->usable())
			throw std::logic_error("使用了未初始化的对象");
		return *this->mDat->data();
	}
	virtual const T & data() const
	{
		return this->data();
	}
	virtual void data(const T & dat)
	{
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
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
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		this->mDat = this->mDat->forward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
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
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		this->mDat = this->mDat->backward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("使用了未初始化的对象");
		this->mDat = this->mDat->forward();
	}
};

// 时钟
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
	timer_t time(timeUnit unit)
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
	void delay(timeUnit unit, timer_t time)
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
	template<typename T,typename...Elements>
	timer_t measure(timeUnit unit,T func,Elements&&...args)
	{
		timer_t begin(0),end(0);
		begin=this->time(unit);
		func(args...);
		end=this->time(unit);
		return end-begin;
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
			if(typeid(T) != current_switcher_type)
				throw std::logic_error("请求的类型与对象类型不同");
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

// Covariant Tuple

namespace cov {
// 判断类型是否相同
	template < typename _fT, typename _sT > struct isTypeEqual {
		static constexpr bool value = false;
	};
	template < typename _Tp > struct isTypeEqual <_Tp, _Tp > {
		static constexpr bool value = true;
	};
// Tuple节点
	template < typename _fT, typename _sT > struct tuple_node {
		_fT current;
		_sT forward;
	};
	struct final_tuple_node {};
// Tuple包装器
	template < typename ... Args > struct tuple_wrapper;
	template < typename _Tp > struct tuple_wrapper <_Tp > {
		typedef tuple_node<_Tp,final_tuple_node> type;
	};
	template < typename _Tp, typename ... Args > struct tuple_wrapper<_Tp, Args ... > {
		typedef tuple_node < _Tp, typename tuple_wrapper < Args ... >::type > type;
	};
// 基于类型的Tuple迭代器
	template<typename _Arg,typename _Tp,typename _Node>
	struct type_iterator {
		typedef _Arg type;
		typedef tuple_node<_Tp,_Node> node_type;
		static type& get(node_type &var)
		{
			static_assert(isTypeEqual<_Arg,_Tp>::value,"类型匹配失败");
		}
		static const type & get(const node_type&var)
		{
			static_assert(isTypeEqual<_Arg,_Tp>::value,"类型匹配失败");
		}
		static void set(const type&,node_type&)
		{
			static_assert(isTypeEqual<_Arg,_Tp>::value,"类型匹配失败");
		}
	};
	template<typename _Tp,typename _Node>
	struct type_iterator<_Tp,_Tp,_Node> {
		typedef _Tp type;
		typedef tuple_node<_Tp,_Node> node_type;
		static _Tp& get(node_type &var)
		{
			return var.current;
		}
		static const _Tp& get(const node_type &var)
		{
			return var.current;
		}
		static void set(const type& _dat,node_type& _t)
		{
			_t.current=_dat;
		}
	};
	template<typename _Tp,typename _fT,typename _sT>
	struct type_iterator<_Tp,_Tp,tuple_node<_fT,_sT>> {
		typedef _Tp type;
		typedef tuple_node<_Tp,tuple_node<_fT,_sT>> node_type;
		static type& get(node_type &var)
		{
			return var.current;
		}
		static const type& get(const node_type &var)
		{
			return var.current;
		}
		static void set(const type& _dat,node_type& _t)
		{
			_t.current=_dat;
		}
	};
	template<typename _Arg,typename _Tp,typename _fT,typename _sT>
	struct type_iterator<_Arg,_Tp,tuple_node<_fT,_sT>> {
		typedef _Arg type;
		typedef tuple_node<_Tp,tuple_node<_fT,_sT>> node_type;
		static type& get(node_type &var)
		{
			return type_iterator<_Arg,_fT,_sT>::get(var.forward);
		}
		static const type& get(const node_type &var)
		{
			return type_iterator<_Arg,_fT,_sT>::get(var.forward);
		}
		static void set(const type & _dat, node_type & _t)
		{
			type_iterator<_Arg,_fT,_sT>::set(_dat, _t.forward);
		}
	};
// 基于下标的Tuple迭代器
	template < int N, typename _Tp > struct tuple_element;
	template < typename _cT, typename _fT > struct tuple_element <0, tuple_node < _cT, _fT >> {
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
	template < typename _cT, typename _fT, typename _sT >
	struct tuple_element <0, tuple_node < _cT, tuple_node < _fT, _sT >>> {
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
// Tuple填充函数
	void fill_tuple(const final_tuple_node & _t)
	{
	}
	template <typename T, typename...Elements >
	void fill_tuple(typename tuple_wrapper<T, Elements...>::type & _t, const T& dat,const Elements&... args)
	{
		_t.current = dat;
		fill_tuple(_t.forward, args...);
	}
// Tuple元素Get函数
	template<typename T,typename _fT,typename _sT>
	T& get(tuple_node<_fT,_sT>&var)
	{
		return type_iterator<T,_fT,_sT>::get(var);
	}
	template < int N, typename _fT, typename _sT >
	typename tuple_element < N, tuple_node < _fT, _sT >>::type &get(tuple_node < _fT, _sT > &_t)
	{
		return tuple_element < N, tuple_node < _fT, _sT >>::get(_t);
	}
	template<typename T,typename _fT,typename _sT>
	const T& get(const tuple_node<_fT,_sT>&var)
	{
		return type_iterator<T,_fT,_sT>::get(var);
	}
	template < int N, typename _fT, typename _sT >
	const typename tuple_element < N, tuple_node < _fT, _sT >>::type &get(const tuple_node < _fT, _sT > &_t)
	{
		return tuple_element < N, tuple_node < _fT, _sT >>::get(_t);
	}
// Tuple类
	template<typename...Elements>
	class tuple {
	protected:
		typename tuple_wrapper<Elements...>::type mTuple;
	public:
		typedef typename tuple_wrapper<Elements...>::type tuple_type;
		tuple()=default;
		tuple(const tuple&)=default;
		tuple(tuple&&)=default;
		~tuple()=default;
		tuple(const Elements&...args)
		{
			cov::fill_tuple(mTuple,args...);
		}
		template<typename T>T& get()
		{
			return cov::get<T>(this->mTuple);
		}
		template<typename T> const T& get() const
		{
			return cov::get<T>(this->mTuple);
		}
		template<int N> typename tuple_element < N, tuple_type>::type& get()
		{
			return cov::get<N>(this->mTuple);
		}
		template<int N> const typename tuple_element < N, tuple_type>::type& get() const
		{
			return cov::get<N>(this->mTuple);
		}
	};
// Tuple类制作函数
	template<typename...Elements>tuple<Elements...> make_tuple(const Elements&...args)
	{
		return std::move(tuple<Elements...>(args...));
	}
}

// Covariant Memory

namespace cov {
	namespace allocator_policy {
		static bool full_manual=false; //全手动
		static bool delay_construction=true; //延时构造
		static bool allocate_on_demand=true; //写时分配
		static bool memory_alignment=true; //内存对齐
		static std::size_t max_recycle_count=1000; //回收栈可容纳的对象数量
	}
// 资源持有者类
	template<typename T>
	class sourceHolder {
	public:
		static std::allocator<T> sourceManager;
	protected:
		static const T* hintAddr;
		std::size_t mSize=0;
		bool mAvailable=false;
		bool mUsable=false;
		T* mSource=nullptr;
		void mAllocate(std::size_t n=1)
		{
			mSize=n;
			if(allocator_policy::memory_alignment&&hintAddr!=nullptr) {
				mSource=sourceManager.allocate(mSize,hintAddr);
				hintAddr=mSource;
			} else
				mSource=sourceManager.allocate(mSize);
		}
	public:
		void allocate(std::size_t n=1)
		{
			destroy();
			if(!mAvailable) {
				mAllocate();
				mAvailable=true;
			}
		}
		void deallocate()
		{
			destroy();
			if(mAvailable) {
				sourceManager.deallocate(mSource,mSize);
				mSize=0;
				mAvailable=false;
			}
		}
		template<typename...Args>void construct(Args&&...args)
		{
			allocate();
			sourceManager.construct(mSource,args...);
			mUsable=true;
		}
		void destroy()
		{
			if(mAvailable&&mUsable) {
				sourceManager.destroy(mSource);
				mUsable=false;
			}
		}
		inline bool usable() const
		{
			return mAvailable&&mUsable;
		}
		T* source()
		{
			return mSource;
		}
		const T* source() const
		{
			return mSource;
		}
		void data(const T& dat)
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
			*mSource=dat;
		}
		T& data()
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
			return *mSource;
		}
		const T& data() const
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
			return *mSource;
		}
	public:
		sourceHolder()=default;
		sourceHolder(const sourceHolder&)=delete;
		template<typename...Args>
		explicit sourceHolder(Args&&...args)
		{
			construct(args...);
		}
		~sourceHolder()=default;
		sourceHolder& operator=(const sourceHolder&)=delete;
		bool operator==(const sourceHolder& holder) const
		{
			return mSource==holder.mSource;
		}
		bool operator!=(const sourceHolder& holder) const
		{
			return mSource!=holder.mSource;
		}
		T& operator*()
		{
			if(!usable())
				throw std::logic_error("使用了未初始化的对象");
			return *mSource;
		}
		const T& operator*() const
		{
			if(!usable())
				throw std::logic_error("使用了未初始化的对象");
			return *mSource;
		}
		T* operator->()
		{
			if(!usable())
				throw std::logic_error("使用了未初始化的对象");
			return mSource;
		}
		const T* operator->() const
		{
			if(!usable())
				throw std::logic_error("使用了未初始化的对象");
			return mSource;
		}
		operator bool() const
		{
			return mAvailable;
		}
	};
	template<typename> class allocator;
// 持有者类
	template<typename T>
	class holder {
		friend class allocator<T>;
		std::shared_ptr<sourceHolder<T>> mDat;
		static void deleter(sourceHolder<T>*);
	public:
		holder():mDat(new sourceHolder<T>(),deleter)
		{
			if(!allocator_policy::allocate_on_demand)
				mDat->allocate();
			if(!allocator_policy::delay_construction||allocator_policy::full_manual)
				mDat->construct();
		}
		holder(holder&&)=default;
		explicit holder(holder&)=default;
		explicit holder(sourceHolder<T>* ptr):mDat(ptr) {}
		template<typename...Args>explicit holder(Args&&...args):mDat(new sourceHolder<T>(args...),deleter) {}
		virtual ~holder()=default;
		T& data()
		{
			return mDat->data();
		}
		const T& data() const
		{
			return mDat->data();
		}
		sourceHolder<T>& raw_data()
		{
			return *mDat;
		}
		const sourceHolder<T>& raw_data() const
		{
			return *mDat;
		}
		const T& operator=(const T& data)
		{
			if(!mDat->usable())
				mDat->construct(data);
			else
				mDat->data(data);
			return data;
		}
		holder& operator=(const holder&)=default;
		holder& operator=(holder&&)=default;
		bool operator==(const holder& obj) const
		{
			return this->dat()==obj.dat();
		}
		bool operator!=(const holder& obj) const
		{
			return this->dat()!=obj.dat();
		}
	};
// 分配器类
	template<typename T>
	class allocator {
		friend void holder<T>::deleter(sourceHolder<T>*);
		class collector {
			friend class allocator;
			friend void holder<T>::deleter(sourceHolder<T>*);
			std::deque<sourceHolder<T>*>recycleStack;
			~collector();
		};
		static collector mCollector;
	public:
		static inline holder<T> allocate()
		{
			if(allocator_policy::full_manual)
				return std::move(holder<T>());
			if(!mCollector.recycleStack.empty()) {
				holder<T> ret(mCollector.recycleStack.front());
				mCollector.recycleStack.pop_front();
				return std::move(ret);
			}
			return std::move(holder<T>());
		}
		template<typename...Args>static inline holder<T> allocate(Args&&...args)
		{
			if(allocator_policy::full_manual)
				return std::move(holder<T>(args...));
			if(!mCollector.recycleStack.empty()) {
				holder<T> ret(mCollector.recycleStack.front());
				mCollector.recycleStack.pop_front();
				ret.raw_data().construct(args...);
				return std::move(ret);
			}
			return std::move(holder<T>(args...));
		}
		static inline void recycle()
		{
			while(mCollector.recycleStack.size()>allocator_policy::max_recycle_count) {
				mCollector.recycleStack.back()->deallocate();
				delete mCollector.recycleStack.back();
				mCollector.recycleStack.pop_back();
			}
		}
		static inline void deallocate(holder<T>& source)
		{
			source.mDat=nullptr;
		}
	};
	template<typename T>allocator<T>::collector::~collector()
	{
		for(auto&it:recycleStack) {
			it->deallocate();
			delete it;
		}
	}
	template<typename T>void holder<T>::deleter(sourceHolder<T>* ptr)
	{
		if(allocator_policy::full_manual) {
			if(ptr!=nullptr) {
				ptr->deallocate();
				delete ptr;
			}
		} else {
			if(ptr!=nullptr) {
				ptr->destroy();
				allocator<T>::mCollector.recycleStack.push_front(ptr);
			}
			allocator<T>::recycle();
		}
	}
	template<typename T> std::allocator<T> sourceHolder<T>::sourceManager;
	template<typename T> const T* sourceHolder<T>::hintAddr=nullptr;
	template<typename T> typename allocator<T>::collector allocator<T>::mCollector;
}

// Covariant Argument List

class cov::argument_list final {
private:
	std::deque<std::type_index> mTypes;
	std::deque<cov::any> mArgs;
	template<typename _Tp>void unpack_types()
	{
		this->mTypes.push_back(typeid(_Tp));
	}
	template<typename _Tp,typename _fT,typename...ArgTypes>void unpack_types()
	{
		this->mTypes.push_back(typeid(_Tp));
		unpack_types<_fT,ArgTypes...>();
	}
	template<typename _Tp>int count_types(int index=1) const
	{
		return index;
	}
	template<typename _Tp,typename _fT,typename...ArgTypes>int count_types(int index=1) const
	{
		return count_types<_fT,ArgTypes...>(++index);
	}
	template<typename _Tp>std::string get_type(int expect,int index) const
	{
		if(expect==index)
			return typeid(_Tp).name();
	}
	template<typename _Tp,typename _fT,typename...ArgTypes>std::string get_type(int expect,int index) const
	{
		if(expect==index)
			return typeid(_Tp).name();
		else
			return get_type<_fT,ArgTypes...>(expect,++index);
	}
	template<typename _Tp>int check_types(int index,std::deque<std::type_index>::const_iterator it) const
	{
		if(it!=this->mTypes.end()&&*it==typeid(_Tp))
			return -1;
		else
			return index;
	}
	template<typename _Tp,typename _fT,typename...ArgTypes>int check_types(int index,std::deque<std::type_index>::const_iterator it) const
	{
		if(it!=this->mTypes.end()&&*it==typeid(_Tp))
			return check_types<_fT,ArgTypes...>(++index,++it);
		else
			return index;
	}
public:
	typedef std::deque<cov::any>::iterator iterator;
	typedef std::deque<cov::any>::const_iterator const_iterator;
	argument_list()=delete;
	template<typename...ArgTypes>argument_list(ArgTypes&&...args):mArgs({args...})
	{
		unpack_types<ArgTypes...>();
	}
	argument_list(const argument_list&)=default;
	argument_list(argument_list&&)=default;
	~argument_list()=default;
	argument_list& operator=(const argument_list& arglist)
	{
		if(&arglist!=this&&arglist.mTypes==this->mTypes)
			this->mArgs=arglist.mArgs;
		return *this;
	}
	argument_list& operator=(argument_list&& arglist)
	{
		if(&arglist!=this&&arglist.mTypes==this->mTypes)
			this->mArgs=arglist.mArgs;
		return *this;
	}
	bool operator==(const argument_list& arglist)
	{
		if(&arglist==this)
			return true;
		return arglist.mTypes==this->mTypes;
	}
	bool operator!=(const argument_list& arglist)
	{
		if(&arglist==this)
			return false;
		return arglist.mTypes!=this->mTypes;
	}
	cov::any& operator[](std::size_t posit)
	{
		return this->mArgs.at(posit);
	}
	const cov::any& operator[](std::size_t posit) const
	{
		return this->mArgs.at(posit);
	}
	cov::any& at(std::size_t posit)
	{
		return this->mArgs.at(posit);
	}
	const cov::any& at(std::size_t posit) const
	{
		return this->mArgs.at(posit);
	}
	iterator begin()
	{
		return this->mArgs.begin();
	}
	const_iterator begin() const
	{
		return this->mArgs.begin();
	}
	iterator end()
	{
		return this->mArgs.end();
	}
	const_iterator end() const
	{
		return this->mArgs.end();
	}
	std::size_t size() const
	{
		return this->mArgs.size();
	}
	template<typename...ArgTypes>void check(std::string func) const
	{
		if(count_types<ArgTypes...>()==this->mTypes.size()) {
			int result=check_types<ArgTypes...>(1,this->mTypes.begin());
			if(result!=-1)
				throw std::invalid_argument("在函数\""+func+"\"中:参数类型错误:在位置"+cov::toString(result)+".预期的参数类型为"+get_type<ArgTypes...>(result,1));
		} else
			throw std::logic_error("在函数\""+func+"\"中:参数数量错误.预期的参数数量为"+cov::toString(count_types<ArgTypes...>()));
	}
};

// Covariant Function Index

namespace cov {
	template<typename T>struct is_constant {
		static constexpr bool value=false;
	};
	template<typename T>struct is_constant<T const> {
		static constexpr bool value=true;
	};
	template < typename _Tp > class is_functional {
		template < typename T, decltype(&T::operator()) X >struct matcher;
		template < typename T, typename X > static constexpr bool match(X *)
		{
			return false;
		}
		template < typename T, typename X > static constexpr bool match(matcher < T, &T::operator() > *)
		{
			return true;
		}
	public:
		static constexpr bool value = match < _Tp, decltype(nullptr) > (nullptr);
	};
	template<typename T>
	class function_index {
	public:
		static constexpr bool is_member_function=false;
		static constexpr bool is_function_object=is_functional<T>::value;
		typedef T type;
		typedef void return_type;
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		template<typename...Args>
		decltype(std::declval<type>()(std::declval<Args>()...))
		call(Args&&...args)
		{
			static_assert(is_function_object,"Target is not a function object.");
			return function(args...);
		}
	};
	template<typename _rT,typename...Args>
	class function_index<_rT(*)(Args...)> {
	public:
		static constexpr bool is_member_function=false;
		static constexpr bool is_function_object=false;
		typedef _rT(*type)(Args...);
		typedef _rT return_type;
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		_rT call(Args&&...args)
		{
			return function(args...);
		}
	};
	template<typename T,typename _rT,typename...Args>
	class function_index<_rT(T::*)(Args...)> {
	public:
		static constexpr bool is_member_function=true;
		static constexpr bool is_function_object=false;
		typedef _rT(T::*type)(Args...);
		typedef _rT return_type;
	private:
		T* object=nullptr;
		type function;
	public:
		function_index(type func):function(func) {}
		function_index(T* ptr,type func):object(ptr),function(func) {}
		_rT call(Args&&...args)
		{
			if(object==nullptr)
				throw std::logic_error("Call member function with null pointer.");
			return (object->*function)(args...);
		}
		_rT call(T* this_ptr,Args&&...args)
		{
			if(this_ptr==nullptr)
				throw std::logic_error("Call member function with null pointer.");
			return (this_ptr->*function)(args...);
		}
	};
	template<typename T,typename _rT,typename...Args>
	class function_index<_rT(T::*)(Args...) const> {
	public:
		static constexpr bool is_member_function=true;
		static constexpr bool is_function_object=false;
		typedef _rT(T::*type)(Args...) const;
		typedef _rT return_type;
	private:
		const T* object=nullptr;
		type function;
	public:
		function_index(type func):function(func) {}
		function_index(const T* ptr,type func):object(ptr),function(func) {}
		_rT call(Args&&...args)
		{
			if(object==nullptr)
				throw std::logic_error("Call member function with null pointer.");
			return (object->*function)(args...);
		}
		_rT call(const T* this_ptr,Args&&...args)
		{
			if(this_ptr==nullptr)
				throw std::logic_error("Call member function with null pointer.");
			return (this_ptr->*function)(args...);
		}
	};
	template<bool,typename>struct intelligent_infer_function;
	template<typename T>struct intelligent_infer_function<true,T> {
		typedef decltype(&T::operator()) real_type;
		static function_index<real_type> get(T func)
		{
			return function_index<real_type>(&func,&T::operator());
		}
	};
	template<typename T>struct intelligent_infer_function<false,T> {
		typedef T real_type;
		static function_index<real_type> get(T func)
		{
			return function_index<real_type>(func);
		}
	};
	template<typename T>
	function_index<typename intelligent_infer_function<is_functional<T>::value,T>::real_type>
	make_function_index(T func)
	{
		return intelligent_infer_function<is_functional<T>::value,T>::get(func);
	}
}

#endif