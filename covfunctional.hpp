#pragma once
/*
* Covariant C++ Library(2nd Generation) -- Functional
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
* Library Version: 2.16.09.02
*
*/
#include <stdexcept>
#include <utility>
namespace cov {

	template<typename> class function;
	template<typename> class function_base;
	template<typename> class function_index;
	template<typename> class executor_index;
	template<typename> struct function_parser;
	template<bool,typename> struct function_resolver;

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
#ifdef COV_COMMON_FUNCTION_INDEX
	template<typename T> class function_index {
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
		executor_index(_Tp obj):object(obj),function(&_Tp::operator()) {}
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
		executor_index(const _Tp obj):object(obj),function(&_Tp::operator()) {}
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
		static type make(_Tp f)
		{
			return executor_index<decltype(&_Tp::operator())>(f);
		}
		static type* make_ptr(_Tp f)
		{
			return new executor_index<decltype(&_Tp::operator())>(f);
		}
	};
	template<typename _Tp>struct function_resolver<false,_Tp> {
		typedef function_index<_Tp> type;
		static type make(_Tp f)
		{
			return function_index<_Tp>(f);
		}
		static type* make_ptr(_Tp f)
		{
			return new function_index<_Tp>(f);
		}
	};
	template<typename _Tp> struct function_parser {
		typedef typename function_resolver<is_functional<_Tp>::value,_Tp>::type type;
		static type make_func(_Tp f)
		{
			return function_resolver<is_functional<_Tp>::value,_Tp>::make(f);
		}
		static type* make_func_ptr(_Tp f)
		{
			return function_resolver<is_functional<_Tp>::value,_Tp>::make_ptr(f);
		}
	};

	template<typename _rT,typename...ArgsT>
	class function<_rT(ArgsT...)> final {
		function_base<_rT(*)(ArgsT...)>* mFunc=nullptr;
	public:
		function()=default;
		template<typename _Tp> explicit function(_Tp func)
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
		function(function&& func)
		{
			if(func.mFunc==nullptr)
				mFunc=func.mFunc;
			else
				mFunc=func.mFunc->copy();
		}
		~function()
		{
			delete mFunc;
		}
		bool callable() const
		{
			return mFunc!=nullptr;
		}
		_rT call(ArgsT&&...args) const
		{
			if(!callable())
				throw std::logic_error("E0005");
			return mFunc->call(std::forward<ArgsT>(args)...);
		}
		_rT operator()(ArgsT&&...args) const
		{
			if(!callable())
				throw std::logic_error("E0005");
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
				delete mFunc;
				if(func.mFunc==nullptr)
					mFunc=func.mFunc;
				else
					mFunc=func.mFunc->copy();
			}
			return *this;
		}
	};
}