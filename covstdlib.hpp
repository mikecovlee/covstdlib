#pragma once
/*
* Covariant C++ Library(2nd Generation)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Copyright (C) 2016 Michael Lee(李登淳)
* Email: China-LDC@outlook.com
* Github: https://github.com/mikecovlee
* Website: http://ldc.atd3.cn
*
* Library Version: 2.16.08.02
*
* Function List:
* Covariant Templates Toolbox
* Covariant Functions Toolbox
* cov::any
* cov::list
* cov::tuple
* cov::timer
* cov::switcher
* cov::argument_list
*
* Marco List:
* COV_COMMON_FUNCTION_INDEX
*/
#ifndef __cplusplus
#error E0001
#else

#if __cplusplus < 201300L
#error E0002
#else

#include <map>
#include <deque>
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <sstream>
#include <ostream>
#include <typeinfo>
#include <stdexcept>
#include <typeindex>

namespace cov {
	class any;
	class timer;
	class argument_list;
	template < typename > class list;
}

// Covariant Templates Toolbox

namespace cov {
	template < typename T, typename X > struct is_same_type {
		static constexpr bool value=false;
	};
	template < typename T > struct is_same_type<T,T> {
		static constexpr bool value=true;
	};
	template < typename T > struct is_constant {
		static constexpr bool value=false;
	};
	template < typename T > struct is_constant<T const> {
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
}

// Covariant Functions Toolbox

namespace cov {
template<typename _Tp>class function_base;
	template<typename _rT,typename..._ArgsT>
	class function_base<_rT(*)(_ArgsT...)>
	{
		public:
		function_base()=default;
		function_base(const function_base&)=default;
		function_base(function_base&&)=default;
		virtual ~function_base()=default;
		virtual _rT call(_ArgsT&&...)=0;
	};
#ifndef COV_COMMON_FUNCTION_INDEX
	template<typename T>
	class function_index;
#else
	template<typename T>
	class function_index {
	public:
		static constexpr bool is_member_function=false;
		static constexpr bool is_function_object=is_functional<T>::value;
		typedef T type;
		typedef void return_type;
		typedef void(*common_type)();
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		template<typename...Args>
		decltype(std::declval<type>()(std::declval<Args>()...))
		call(Args&&...args)
		{
			static_assert(is_function_object,"E0003");
			return function(args...);
		}
		template<typename...Args>
		decltype(std::declval<type>()(std::declval<Args>()...))
		operator()(Args&&...args)
		{
			static_assert(is_function_object,"E0003");
			return function(args...);
		}
	};
#endif /* #ifndef COV_COMMON_FUNCTION_INDEX */
	template<typename _rT,typename...Args>
	class function_index<_rT(*)(Args...)>:public function_base<_rT(*)(Args...)> {
	public:
		static constexpr bool is_member_function=false;
		static constexpr bool is_function_object=false;
		typedef _rT(*type)(Args...);
		typedef _rT return_type;
		typedef _rT(*common_type)(Args...);
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		virtual _rT call(Args&&...args) override
		{
			return function(args...);
		}
		_rT operator()(Args&&...args)
		{
			return function(args...);
		}
	};
	template<typename T,typename _rT,typename...Args>
	class function_index<_rT(T::*)(Args...)>:public function_base<_rT(*)(Args...)> {
	public:
		static constexpr bool is_member_function=true;
		static constexpr bool is_function_object=false;
		typedef _rT(T::*type)(Args...);
		typedef _rT return_type;
		typedef _rT(*common_type)(Args...);
	private:
		T* object=nullptr;
		type function;
	public:
		function_index(type func):function(func) {}
		function_index(T* ptr,type func):object(ptr),function(func) {}
		virtual _rT call(Args&&...args) override
		{
			if(object==nullptr)
				throw std::logic_error("E0004");
			return (object->*function)(args...);
		}
		_rT _call(T* this_ptr,Args&&...args)
		{
			if(this_ptr==nullptr)
				throw std::logic_error("E0004");
			return (this_ptr->*function)(args...);
		}
		_rT operator()(Args&&...args)
		{
			if(object==nullptr)
				throw std::logic_error("E0004");
			return (object->*function)(args...);
		}
	};
	template<typename T,typename _rT,typename...Args>
	class function_index<_rT(T::*)(Args...) const>:public function_base<_rT(*)(Args...)> {
	public:
		static constexpr bool is_member_function=true;
		static constexpr bool is_function_object=false;
		typedef _rT(T::*type)(Args...) const;
		typedef _rT return_type;
		typedef _rT(*common_type)(Args...);
	private:
		const T* object=nullptr;
		type function;
	public:
		function_index(type func):function(func) {}
		function_index(const T* ptr,type func):object(ptr),function(func) {}
		virtual _rT call(Args&&...args) override
		{
			if(object==nullptr)
				throw std::logic_error("E0004");
			return (object->*function)(args...);
		}
		_rT _call(const T* this_ptr,Args&&...args)
		{
			if(this_ptr==nullptr)
				throw std::logic_error("E0004");
			return (this_ptr->*function)(args...);
		}
		_rT operator()(Args&&...args)
		{
			if(object==nullptr)
				throw std::logic_error("E0004");
			return (object->*function)(args...);
		}
	};
	template<typename T,typename...Args> struct is_received;
	template<typename T,typename...Args> struct is_received<function_index<T>,Args...> {
		static constexpr bool value=is_same_type<typename function_index<T>::common_type,typename function_index<T>::return_type(*)(Args...)>::value;
	};
	template<bool,typename>struct intelligent_infer_function;
	template<typename T>struct intelligent_infer_function<true,T> {
		typedef decltype(&T::operator()) real_type;
		static function_index<real_type> get(T func)
		{
			return function_index<real_type>(&func,&T::operator());
		}
		static function_index<real_type>* get_ptr(T func)
		{
			return new function_index<real_type>(&func,&T::operator());
		}
	};
	template<typename T>struct intelligent_infer_function<false,T> {
		typedef T real_type;
		static function_index<real_type> get(T func)
		{
			return function_index<real_type>(func);
		}
		static function_index<real_type>* get_ptr(T func)
		{
			return new function_index<real_type>(func);
		}
	};
	template<typename T>
	function_index<typename intelligent_infer_function<is_functional<T>::value,T>::real_type>
	make_function_index(T func)
	{
		return intelligent_infer_function<is_functional<T>::value,T>::get(func);
	}
	template<typename T>
	function_index<typename intelligent_infer_function<is_functional<T>::value,T>::real_type>*
	make_function_index_ptr(T func)
	{
		return intelligent_infer_function<is_functional<T>::value,T>::get_ptr(func);
	}
	template<typename _Tp> class function;
	template<typename _rT,typename...ArgsT>
	class function<_rT(ArgsT...)> final
	{
		function_base<_rT(*)(ArgsT...)>* __f_=nullptr;
		public:
		function()=delete;
		template<typename _Tp>function(_Tp __f)
		{
			static_assert(is_same_type<_rT(*)(ArgsT...),typename function_index<typename intelligent_infer_function<is_functional<_Tp>::value,_Tp>::real_type>::common_type>::value,"E000B");
			__f_=make_function_index_ptr(__f);
		}
		~function(){delete __f_;}
		_rT operator()(ArgsT&&...args)
		{
			return __f_->call(std::forward<ArgsT>(args)...);
		}
	};
}

// Covariant Any

class cov::any final {
public:
	template < typename T > static std::string toString(const T & val)
	{
		std::string str;
		std::stringstream ss;
		ss << val;
		ss >> str;
		return std::move(str);
	}
private:
	class baseHolder {
	public:
		baseHolder() = default;
		virtual ~ baseHolder() = default;
		virtual const std::type_info & type() const = 0;
		virtual baseHolder *duplicate() = 0;
		virtual bool compare(const baseHolder *) const = 0;
		virtual std::string toString() const = 0;
	};
	template < typename T > class holder final:public baseHolder {
	protected:
		T mDat;
	public:
		holder() = default;
		holder(const T& dat):mDat(dat) {}
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
		virtual std::string toString() const override
		{
			return std::move(any::toString(mDat));
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
	std::string toString() const
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		return std::move(this->mDat->toString());
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
			throw std::logic_error("E0006");
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		return dynamic_cast < holder < T > *>(this->mDat)->data();
	}
	template < typename T > const T & val() const
	{
		if(typeid(T) != this->type())
			throw std::logic_error("E0006");
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
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

// Covariant List

template < typename T > class cov::list final {
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
			throw std::logic_error("E0005");
		node *current = it.mDat;
		node *newdat = new node(current->backward(), new T(dat), current);
		current->backward()->forward(newdat);
		current->backward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace(base_iterator & it, Args && ... args)
	{
		if(!it.usable())
			throw std::logic_error("E0005");
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
			throw std::logic_error("E0005");
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
			throw std::logic_error("E0005");
		return *mFront.forward()->data();
	}
	const T & front() const
	{
		return this->front();
	}
	T & back()
	{
		if(!mBack.backward()->usable())
			throw std::logic_error("E0005");
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
			throw std::logic_error("E0005");
		return *this->mDat->data();
	}
	virtual const T & data() const
	{
		return this->data();
	}
	virtual void data(const T & dat)
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
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
template < typename T > class cov::list < T >::iterator:public base_iterator {
	friend class list;
public:
	using base_iterator::base_iterator;
	iterator() = delete;
	virtual void forward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		this->mDat = this->mDat->forward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		this->mDat = this->mDat->backward();
	}
};
template < typename T > class cov::list < T >::reverse_iterator:public base_iterator {
	friend class list;
public:
	using base_iterator::base_iterator;
	reverse_iterator() = delete;
	virtual void forward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		this->mDat = this->mDat->backward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw std::logic_error("E0005");
		this->mDat = this->mDat->forward();
	}
};

// Covariant Tuple

namespace cov {
	template < typename _fT, typename _sT > struct tuple_node {
		_fT current;
		_sT forward;
	};
	struct final_tuple_node {};
	template < typename ... Args > struct tuple_wrapper;
	template < typename _Tp > struct tuple_wrapper <_Tp > {
		typedef tuple_node<_Tp,final_tuple_node> type;
	};
	template < typename _Tp, typename ... Args > struct tuple_wrapper<_Tp, Args ... > {
		typedef tuple_node < _Tp, typename tuple_wrapper < Args ... >::type > type;
	};
	template<typename _Arg,typename _Tp,typename _Node>
	struct type_iterator {
		typedef _Arg type;
		typedef tuple_node<_Tp,_Node> node_type;
		static type& get(node_type &var)
		{
			static_assert(is_same_type<_Arg,_Tp>::value,"E000B");
		}
		static const type & get(const node_type&var)
		{
			static_assert(is_same_type<_Arg,_Tp>::value,"E000B");
		}
		static void set(const type&,node_type&)
		{
			static_assert(is_same_type<_Arg,_Tp>::value,"E000B");
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
	void fill_tuple(const final_tuple_node & _t)
	{
	}
	template <typename T, typename...Elements >
	void fill_tuple(typename tuple_wrapper<T, Elements...>::type & _t, const T& dat,const Elements&... args)
	{
		_t.current = dat;
		fill_tuple(_t.forward, args...);
	}
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
	template<typename...Elements>
	class tuple final {
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
	template<typename...Elements>tuple<Elements...> make_tuple(const Elements&...args)
	{
		return std::move(tuple<Elements...>(args...));
	}
}

// Covariant Timer

class cov::timer final {
	static std::chrono::time_point < std::chrono::high_resolution_clock > m_timer;
public:
	typedef unsigned long timer_t;
	enum class time_unit {
		nano_sec, micro_sec, milli_sec, second, minute
	};
	static void reset()
	{
		m_timer = std::chrono::high_resolution_clock::now();
	}
	static timer_t time(time_unit unit)
	{
		switch (unit) {
		case time_unit::nano_sec:
			return std::chrono::duration_cast < std::chrono::nanoseconds >(std::chrono::high_resolution_clock::now() - m_timer).count();
		case time_unit::micro_sec:
			return std::chrono::duration_cast < std::chrono::microseconds >(std::chrono::high_resolution_clock::now() - m_timer).count();
		case time_unit::milli_sec:
			return std::chrono::duration_cast < std::chrono::milliseconds >(std::chrono::high_resolution_clock::now() - m_timer).count();
		case time_unit::second:
			return std::chrono::duration_cast < std::chrono::seconds >(std::chrono::high_resolution_clock::now() - m_timer).count();
		case time_unit::minute:
			return std::chrono::duration_cast < std::chrono::minutes >(std::chrono::high_resolution_clock::now() - m_timer).count();
		}
		return 0;
	}
	static void delay(time_unit unit, timer_t time)
	{
		switch (unit) {
		case time_unit::nano_sec:
			std::this_thread::sleep_for(std::chrono::nanoseconds(time));
			break;
		case time_unit::micro_sec:
			std::this_thread::sleep_for(std::chrono::microseconds(time));
			break;
		case time_unit::milli_sec:
			std::this_thread::sleep_for(std::chrono::milliseconds(time));
			break;
		case time_unit::second:
			std::this_thread::sleep_for(std::chrono::seconds(time));
			break;
		case time_unit::minute:
			std::this_thread::sleep_for(std::chrono::minutes(time));
			break;
		}
	}
	template<typename T,typename...Elements>
	static timer_t measure(time_unit unit,T func,Elements&&...args)
	{
		timer_t begin(0),end(0);
		static_assert(is_received<function_index<typename intelligent_infer_function<is_functional<T>::value,T>::real_type>,Elements...>::value,"E0007");
		begin=time(unit);
		func(args...);
		end=time(unit);
		return end-begin;
	}
};
std::chrono::time_point < std::chrono::high_resolution_clock > cov::timer::m_timer(std::chrono::high_resolution_clock::now());

// Covariant Switcher

namespace cov {
	namespace logic {
		class baseFunction {
		public:
			baseFunction() = default;
			virtual ~ baseFunction()
			{
			}
			virtual void perform() = 0;
		};
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
		template < typename T > class lambdaBlock final:public baseFunction {
		protected:
			T mLambda;
		public:
			lambdaBlock() = delete;
			lambdaBlock(T lamb):mLambda(lamb)
			{
				static_assert(is_functional<T>::value,"E0003");
				static_assert(is_received<function_index<typename intelligent_infer_function<is_functional<T>::value,T>::real_type>>::value,"E000A");
			}
			virtual void perform()
			{
				mLambda();
			}
		};
		template < typename T > class switcher final:public baseSwitcher {
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
		class switches final {
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
		static switches switcher_stack;
		template < typename T > void checkType()
		{
			const std::type_info & current_switcher_type = switcher_stack.current()->typeinf();
			if(typeid(T) != current_switcher_type)
				throw std::logic_error("E0006");
		}
		template < typename T > lambdaBlock < T > *GetLambda(T lamb)
		{
			return new lambdaBlock < T > (lamb);
		}
		template < typename T > baseSwitcher * GetSwitcher(const T & condition)
		{
			return new switcher < T > (condition);
		}
		template < typename T > void addMethod(baseSwitcher * ptr, const T & obj,baseFunction * lambda)
		{
			checkType < T > ();
			switcher < T > *s = dynamic_cast < switcher < T > *>(ptr);
			if (s != nullptr)
				s->addMethod(obj, lambda);
		}
		template < typename T > void runSwitcher(const T & switches)
		{
			static_assert(is_functional<T>::value,"E0003");
			switches();
		}
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
	template<typename...ArgTypes>argument_list(ArgTypes&&...args):mArgs(
	{
		args...
	})
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
	template<typename...ArgTypes>void check() const
	{
		if(count_types<ArgTypes...>()==this->mTypes.size()) {
			int result=check_types<ArgTypes...>(1,this->mTypes.begin());
			if(result!=-1)
				throw std::invalid_argument("E0008.At "+cov::any::toString(result)+".Expected "+get_type<ArgTypes...>(result,1));
		} else
			throw std::logic_error("E0009.Expected "+cov::any::toString(count_types<ArgTypes...>()));
	}
};

#define Switch(obj) cov::logic::runSwitcher([&]{cov::logic::switcher_stack.push(cov::logic::GetSwitcher(obj));
#define Switch_(typ,obj) cov::logic::runSwitcher([&]{cov::logic::switcher_stack.push(cov::logic::GetSwitcher<typ>(obj));
#define EndSwitch cov::logic::switcher_stack.current()->perform();cov::logic::switcher_stack.pop();});
#define Case(obj) cov::logic::addMethod(cov::logic::switcher_stack.current(),obj,cov::logic::GetLambda([&]{
#define Case_(typ,obj) cov::logic::checkType<typ>();cov::logic::addMethod<typ>(cov::logic::switcher_stack.current(),obj,cov::logic::GetLambda([&]{
#define Default cov::logic::switcher_stack.current()->addDefault(cov::logic::GetLambda([&]{
#define EndCase }));

#endif /* #ifndef __cplusplus */
#endif /* #if __cplusplus < 201300L */