#pragma once
/*
* Covariant C++ Library(2nd Generation) -- Any
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
* Library Version: 2.16.09.03
*/

#ifndef __cplusplus
#error E0001
#else

#if __cplusplus < 201300L
#error E0002
#else

#include <string>
#include <utility>
#include <ostream>
#include <typeinfo>
#include <stdexcept>

namespace cov {
	class cov::any final {
	public:
		template < typename T > static std::string toString(const T &)
		{
			throw std::logic_error("E000D");
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
				return mDat;
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
}

std::ostream& operator<<(std::ostream& out,const cov::any& val)
{
	out<<val.toString();
	return out;
}

#endif /* #ifndef __cplusplus */
#endif /* #if __cplusplus < 201300L */