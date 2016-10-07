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
* Library Version: 2.16.14
*
* Function List:
* Covariant Functional(New)
* cov::any(New)
* cov::tuple
* cov::timer
* cov::switcher(New)
* cov::argument_list
*
* Marco List:
* Library Version: __covcpplib
*/
#ifndef __cplusplus
#error E0001
#endif

#if __cplusplus < 201300L
#error E0002
#endif

#define __covcpplib 201614L

#include <map>
#include <deque>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include <utility>
#include <ostream>
#include <typeinfo>
#include <exception>
#include <typeindex>

namespace cov {
	class warning final:public std::exception {
		std::string mWhat="Covstdlib Warning";
	public:
		warning()=default;
		warning(const std::string& str) noexcept:mWhat("Covstdlib Warning:"+str) {}
		warning(const warning&)=default;
		warning(warning&&)=default;
		virtual ~warning()=default;
		virtual const char* what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};
	class error final:public std::exception {
		std::string mWhat="Covstdlib Error";
	public:
		error()=default;
		error(const std::string& str) noexcept:mWhat("Covstdlib Error:"+str) {}
		error(const error&)=default;
		error(error&&)=default;
		virtual ~error()=default;
		error& operator=(const error&)=default;
		error& operator=(error&&)=default;
		virtual const char* what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};
	class object {
	public:
		static bool show_warning;
		object()=default;
		object(object&&) noexcept=default;
		object(const object&)=default;
		virtual ~object()=default;
		virtual std::type_index object_type() const noexcept final
		{
			return typeid(*this);
		}
		virtual std::string to_string() const noexcept
		{
			return typeid(*this).name();
		}
		virtual object* clone() noexcept
		{
			return nullptr;
		}
		virtual bool equals(const object* ptr) const noexcept
		{
			return this==ptr;
		}
	};
	class covstdlib final:public object {
	public:
		typedef std::deque<std::string> string_array;
		covstdlib()=default;
		covstdlib(const covstdlib&)=delete;
		covstdlib(covstdlib&&) noexcept=delete;
		~covstdlib()=default;
		static int main(const string_array&);
	};
	bool object::show_warning=true;
	template<typename> class function;
	template<typename> class function_base;
	template<typename> class function_container;
	template<typename> class function_index;
	template<typename> class executor_index;
	template<typename> struct function_parser;
	template<bool,typename> struct function_resolver;
	class any;
	class timer;
	class switcher;
	class argument_list;
}

int main(int args,char** argv)
{
	static cov::covstdlib::string_array arr;
	for(int i=0; i<args; ++i)
		arr.push_back(argv[i]);
	try {
		return cov::covstdlib::main(arr);
	} catch(cov::warning w) {
		std::printf("%s\nAborted",w.what());
		return -1;
	} catch(cov::error e) {
		std::printf("%s\nAborted",e.what());
		return -1;
	}
	return -1;
}

// Covariant Functional

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
		template < typename T,decltype(&T::operator()) X >struct matcher;
		template < typename T > static constexpr bool match(T*)
		{
			return false;
		}
		template < typename T > static constexpr bool match(matcher < T,&T::operator() > *)
		{
			return true;
		}
	public:
		static constexpr bool value = match < _Tp > (nullptr);
	};
	template<typename _rT,typename..._ArgsT>
	class function_base<_rT(*)(_ArgsT...)> {
	public:
		function_base()=default;
		function_base(const function_base&)=default;
		function_base(function_base&&)=default;
		virtual ~function_base()=default;
		virtual function_base* copy() const=0;
		virtual _rT call(_ArgsT&&...) const=0;
	};
	template<typename _Tp> class function_container {
		_Tp function;
	public:
		function_container(_Tp func):function(func) {}
		template<typename..._ArgsT>
		decltype(std::declval<_Tp>()(std::declval<_ArgsT>()...))
		call(_ArgsT&&...args)
		{
			return function(std::forward<_ArgsT>(args)...);
		}
	};
	template<typename _rT,typename...Args>
	class function_index<_rT(*)(Args...)>:public function_base<_rT(*)(Args...)> {
	public:
		typedef _rT(*type)(Args...);
		typedef _rT(*common_type)(Args...);
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		virtual _rT call(Args&&...args) const override
		{
			return function(std::forward<Args>(args)...);
		}
		virtual function_base<common_type>* copy() const override
		{
			return new function_index(function);
		}
	};
	template<typename _Tp,typename _rT,typename...Args>
	class function_index<_rT(_Tp::*)(Args...)>:public function_base<_rT(*)(_Tp&,Args...)> {
	public:
		typedef _rT(_Tp::*type)(Args...);
		typedef _rT(*common_type)(_Tp&,Args...);
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		virtual ~function_index()=default;
		virtual _rT call(_Tp& obj,Args&&...args) const override
		{
			return (obj.*function)(std::forward<Args>(args)...);
		}
		virtual function_base<common_type>* copy() const override
		{
			return new function_index(function);
		}
	};
	template<typename _Tp,typename _rT,typename...Args>
	class function_index<_rT(_Tp::*)(Args...) const>:public function_base<_rT(*)(const _Tp&,Args...)> {
	public:
		typedef _rT(_Tp::*type)(Args...) const;
		typedef _rT(*common_type)(const _Tp&,Args...);
	private:
		type function;
	public:
		function_index(type func):function(func) {}
		virtual ~function_index()=default;
		virtual _rT call(const _Tp& obj,Args&&...args) const override
		{
			return (obj.*function)(std::forward<Args>(args)...);
		}
		virtual function_base<common_type>* copy() const override
		{
			return new function_index(function);
		}
	};
	template<typename _Tp,typename _rT,typename..._ArgsT>
	class executor_index<_rT(_Tp::*)(_ArgsT...)>:public function_base<_rT(*)(_ArgsT...)> {
	public:
		typedef _rT(_Tp::*type)(_ArgsT...);
		typedef _rT(*common_type)(_ArgsT...);
	private:
		mutable _Tp object;
		type function;
	public:
		executor_index(const _Tp& obj):object(obj),function(&_Tp::operator()) {}
		virtual ~executor_index()=default;
		virtual _rT call(_ArgsT&&...args) const override
		{
			return (object.*function)(std::forward<_ArgsT>(args)...);
		}
		virtual function_base<common_type>* copy() const override
		{
			return new executor_index(object);
		}
	};
	template<typename _Tp,typename _rT,typename..._ArgsT>
	class executor_index<_rT(_Tp::*)(_ArgsT...) const>:public function_base<_rT(*)(_ArgsT...)> {
	public:
		typedef _rT(_Tp::*type)(_ArgsT...) const;
		typedef _rT(*common_type)(_ArgsT...);
	private:
		const _Tp object;
		type function;
	public:
		executor_index(const _Tp& obj):object(obj),function(&_Tp::operator()) {}
		virtual ~executor_index()=default;
		virtual _rT call(_ArgsT&&...args) const override
		{
			return (object.*function)(std::forward<_ArgsT>(args)...);
		}
		virtual function_base<common_type>* copy() const override
		{
			return new executor_index(object);
		}
	};
	template<typename _Tp>struct function_resolver<true,_Tp> {
		typedef executor_index<decltype(&_Tp::operator())> type;
		static type make(const _Tp& f)
		{
			return executor_index<decltype(&_Tp::operator())>(f);
		}
		static type* make_ptr(const _Tp& f)
		{
			return new executor_index<decltype(&_Tp::operator())>(f);
		}
	};
	template<typename _Tp>struct function_resolver<false,_Tp> {
		typedef function_index<_Tp> type;
		static type make(const _Tp& f)
		{
			return function_index<_Tp>(f);
		}
		static type* make_ptr(const _Tp& f)
		{
			return new function_index<_Tp>(f);
		}
	};
	template<typename _Tp> struct function_parser {
		typedef typename function_resolver<is_functional<_Tp>::value,_Tp>::type type;
		static type make_func(const _Tp& f)
		{
			return function_resolver<is_functional<_Tp>::value,_Tp>::make(f);
		}
		static type* make_func_ptr(const _Tp& f)
		{
			return function_resolver<is_functional<_Tp>::value,_Tp>::make_ptr(f);
		}
	};

	template<typename _rT,typename...ArgsT>
	class function<_rT(ArgsT...)> final {
		function_base<_rT(*)(ArgsT...)>* mFunc=nullptr;
	public:
		bool callable() const
		{
			return mFunc!=nullptr;
		}
		void swap(function& func) noexcept
		{
			function_base<_rT(*)(ArgsT...)>* tmp=mFunc;
			mFunc=func.mFunc;
			func.mFunc=tmp;
		}
		void swap(function&& func) noexcept
		{
			function_base<_rT(*)(ArgsT...)>* tmp=mFunc;
			mFunc=func.mFunc;
			func.mFunc=tmp;
		}
		function()=default;
		template<typename _Tp> function(const _Tp& func)
		{
			static_assert(is_same_type<_rT(*)(ArgsT...),typename function_parser<_Tp>::type::common_type>::value,"E000B");
			mFunc=function_parser<_Tp>::make_func_ptr(func);
		}
		function(const function& func)
		{
			if(func.mFunc==nullptr)
				mFunc=func.mFunc;
			else
				mFunc=func.mFunc->copy();
		}
		function(function&& func) noexcept
		{
			swap(std::forward<function>(func));
		}
		~function()
		{
			delete mFunc;
		}
		_rT call(ArgsT&&...args) const
		{
			if(!callable())
				throw cov::error("E0005");
			return mFunc->call(std::forward<ArgsT>(args)...);
		}
		_rT operator()(ArgsT&&...args) const
		{
			if(!callable())
				throw cov::error("E0005");
			return mFunc->call(std::forward<ArgsT>(args)...);
		}
		template<typename _Tp> function& operator=(_Tp func)
		{
			static_assert(is_same_type<_rT(*)(ArgsT...),typename function_parser<_Tp>::type::common_type>::value,"E000B");
			delete mFunc;
			mFunc=function_parser<_Tp>::make_func_ptr(func);
			return *this;
		}
		function& operator=(const function& func)
		{
			if(this!=&func) {
				delete mFunc;
				if(func.mFunc==nullptr)
					mFunc=func.mFunc;
				else
					mFunc=func.mFunc->copy();
			}
			return *this;
		}
		function& operator=(function&& func)
		{
			if(this!=&func) {
				swap(std::forward<function>(func));
			}
			return *this;
		}
	};
	template<typename _Tp> function_container<_Tp>
	make_function_container(_Tp func)
	{
		return function_container<_Tp>(func);
	}
}

// Covariant Any

namespace std {
	template<typename T> std::string to_string(const T&)
	{
		throw cov::error("E000D");
	}
	template<> std::string to_string<std::string>(const std::string& str)
	{
		return str;
	}
}

namespace cov {
	class any final {
		class baseHolder {
		public:
			baseHolder() = default;
			virtual ~ baseHolder() = default;
			virtual const std::type_info & type() const = 0;
			virtual baseHolder *duplicate() = 0;
			virtual bool compare(const baseHolder *) const = 0;
			virtual std::string to_string() const = 0;
		};
		template < typename T > class holder:public baseHolder {
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
					return ptr!=nullptr?mDat == ptr->data():false;
				}
				return false;
			}
			virtual std::string to_string() const override
			{
				return std::move(std::to_string(mDat));
			}
			T & data()
			{
				return mDat;
			}
			const T & data() const
			{
				return mDat;
			}
			void data(const T & dat)
			{
				mDat = dat;
			}
		};
		baseHolder * mDat=nullptr;
	public:
		void swap(any& obj) noexcept
		{
			baseHolder* tmp=this->mDat;
			this->mDat=obj.mDat;
			obj.mDat=tmp;
		}
		void swap(any&& obj)
		{
			baseHolder* tmp=this->mDat;
			this->mDat=obj.mDat;
			obj.mDat=tmp;
		}
		bool usable() const noexcept
		{
			return mDat != nullptr;
		}
		any()=default;
		template < typename T > any(const T & dat):mDat(new holder < T > (dat)) {}
		any(const any & v):mDat(v.usable()?v.mDat->duplicate():nullptr) {}
		any(any&& v) noexcept
		{
			swap(std::forward<any>(v));
		}
		~any()
		{
			delete mDat;
		}
		const std::type_info & type() const
		{
			return this->mDat != nullptr?this->mDat->type():typeid(void);
		}
		std::string to_string() const
		{
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return std::move(this->mDat->to_string());
		}
		any & operator=(const any & var)
		{
			if(&var!=this) {
				delete mDat;
				mDat = var.usable()?var.mDat->duplicate():nullptr;
			}
			return *this;
		}
		any & operator=(any&& var) noexcept
		{
			if(&var!=this)
				swap(std::forward<any>(var));
			return *this;
		}
		bool operator==(const any & var) const
		{
			return usable()?this->mDat->compare(var.mDat):!var.usable();
		}
		bool operator!=(const any & var)const
		{
			return usable()?!this->mDat->compare(var.mDat):var.usable();
		}
		template < typename T > T & val()
		{
			if(typeid(T) != this->type())
				throw cov::error("E0006");
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return dynamic_cast < holder < T > *>(this->mDat)->data();
		}
		template < typename T > const T & val() const
		{
			if(typeid(T) != this->type())
				throw cov::error("E0006");
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return dynamic_cast < const holder < T > *>(this->mDat)->data();
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
			delete mDat;
			mDat = new holder < T > (dat);
		}
		template < typename T > any & operator=(const T & dat)
		{
			assign(dat);
			return *this;
		}
	};
	template<int N> class any::holder<char[N]>:public any::holder<std::string> {
	public:
		using holder<std::string>::holder;
	};
	template<> class any::holder<std::type_info>:public any::holder<std::type_index> {
	public:
		using holder<std::type_index>::holder;
	};
}

std::ostream& operator<<(std::ostream& out,const cov::any& val)
{
	out<<val.to_string();
	return out;
}

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
	static timer_t measure(time_unit unit,const cov::function<void()>& func)
	{
		timer_t begin(0),end(0);
		begin=time(unit);
		func();
		end=time(unit);
		return end-begin;
	}
};
std::chrono::time_point < std::chrono::high_resolution_clock > cov::timer::m_timer(std::chrono::high_resolution_clock::now());

// Covariant Switcher

namespace cov {
	class switcher final {
	public:
		typedef cov::function<void()> case_type;
	private:
		std::deque<cov::tuple<cov::any,cov::function<void()>>> mCases;
		cov::function<void()> mDefault;
		const cov::any mCondition;
	public:
		switcher()=delete;
		switcher(const cov::any& val):mCondition(val) {}
		switcher(switcher&&) noexcept=delete;
		switcher(const switcher&)=delete;
		~switcher()=default;
		void add_case(const cov::any& head,const case_type& body)
		{
			if(object::show_warning&&head.type()!=mCondition.type())
				throw cov::warning("W0001");
			bool exsist=false;
			for(auto& it:mCases) {
				if(it.get<0>()==head) {
					exsist=true;
					if(object::show_warning)
						throw cov::warning("W0002");
					else
						it.get<1>()=body;
					break;
				}
			}
			if(!exsist)
				mCases.push_back({head,body});
		}
		void add_default(const case_type& body)
		{
			if(mDefault.callable()&&object::show_warning)
				throw cov::warning("W0002");
			mDefault=body;
		}
		void perform()
		{
			for(auto& it:mCases) {
				if(it.get<0>()==mCondition&&it.get<1>().callable()) {
					it.get<1>().call();
					return;
				}
			}
			if(mDefault.callable())
				mDefault.call();
		}
	};
	class switcher_stack final {
		std::deque<switcher*> mStack;
	public:
		switcher_stack()=default;
		switcher_stack(switcher_stack&&) noexcept=delete;
		switcher_stack(const switcher_stack&)=delete;
		~switcher_stack()
		{
			for(auto&it:mStack)
				delete it;
		}
		switcher& top()
		{
			return *mStack.front();
		}
		void push(const cov::any& val)
		{
			mStack.push_front(new switcher(val));
		}
		void pop()
		{
			delete mStack.front();
			mStack.pop_front();
		}
	};
	switcher_stack cov_switchers;
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
				throw std::invalid_argument("E0008.At "+std::to_string(result)+".Expected "+get_type<ArgTypes...>(result,1));
		} else
			throw cov::error("E0009.Expected "+std::to_string(count_types<ArgTypes...>()));
	}
};

#define Switch(obj) cov::cov_switchers.push(obj);
#define EndSwitch cov::cov_switchers.top().perform();cov::cov_switchers.pop();
#define Case(obj) cov::cov_switchers.top().add_case(obj,[&]{
#define Default cov::cov_switchers.top().add_default([&]{
#define EndCase });