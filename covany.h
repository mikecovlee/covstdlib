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
#include <string>
#include <typeinfo>
namespace cov {
class baseHolder;
template < typename T > class holder;
class genericType;
typedef genericType any;
template < typename T > const std::string& toString(const T &)
{
	static std::string str(__func__);
	str+=" is not defined.";
	return str;
}
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
#endif