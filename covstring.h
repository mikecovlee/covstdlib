#include <stdexcept>
namespace cov {
	class string {
	public:
		typedef unsigned long size_type;
		static inline size_type strlen(const char *);
		void assign(const char *);
		void append(const char *);
		bool compare(const char *);
		void push_back(char);
		void pop_back();
	protected:
		size_type mSize;
		char *mDat;
	public:
		string():mDat(0), mSize(0)
		{
		}
		string(const char *source)
		{
			assign(source);
		}
		string(const string & str)
		{
			assign(str.data());
		}
		virtual ~ string()
		{
			delete[]mDat;
		}
		void clear()
		{
			delete[]mDat;
			mDat = nullptr;
		}
		size_type size() const
		{
			return mSize;
		}
		bool empty() const
		{
			return mDat == nullptr;
		}
		void assign(const string & str)
		{
			assign(str.data());
		}
		void append(const string & str)
		{
			append(str.data());
		}
		bool compare(const string & str)
		{
			return compare(str.data());
		}
		char at(size_type posit)
		{
			if (mDat + posit != nullptr)
				return mDat[posit];
			else
				throw std::out_of_range(__func__);
		}
		const char at(size_type posit) const
		{
			if (mDat + posit != nullptr)
				return mDat[posit];
			else
				throw std::out_of_range(__func__);
		}
		const char *data() const
		{
			return mDat;
		}
		char operator[] (size_type posit)
		{
			return at(posit);
		}
		const char operator[] (size_type posit) const
		{
			return at(posit);
		}
		const char *operator=(const char *str)
		{
			assign(str);
			return str;
		}
		string & operator=(const string & str)
		{
			assign(str);
			return *this;
		}
		string & operator+=(char c)
		{
			push_back(c);
			return *this;
		}
		string & operator+=(const char *str)
		{
			append(str);
			return *this;
		}
		string & operator+=(const string & str)
		{
			append(str);
			return *this;
		}
		bool operator==(const string & str)
		{
			return compare(str);
		}
		bool operator!=(const string & str)
		{
			return !compare(str);
		}
		bool operator==(const char *str)
		{
			return compare(str);
		}
		bool operator!=(const char *str)
		{
			return !compare(str);
		}
	};
	inline string::size_type string::strlen(const char *str)
	{
		unsigned long siz = 0;
		for (; str + siz != nullptr && *(str + siz) != '\0'; ++siz);
		return siz;
	}
	void string::assign(const char *source)
	{
		if (mDat != nullptr)
			delete[]mDat;
		mSize = strlen(source);
		mDat = new char[mSize + 1];
		for (int i = 0; i < mSize; ++i)
			mDat[i] = source[i];
		mDat[mSize] = 0;
	}
	void string::append(const char *str)
	{
		char *dat = mDat;
		mDat = new char[mSize + strlen(str) + 1];
		int i = 0;
		for (; i < mSize; ++i)
			mDat[i] = dat[i];
		for (; i < mSize + strlen(str); ++i)
			mDat[i] = str[i - mSize];
		mSize += strlen(str);
		mDat[mSize] = 0;
		delete[]dat;
	}
	bool string::compare(const char *str)
	{
		if (mSize != strlen(str))
			return false;
		for (int i = 0; i < mSize && i < strlen(str); ++i)
			if (mDat[i] != str[i])
				return false;
		return true;
	}
	void string::push_back(char c)
	{
		char *dat = mDat;
		mDat = new char[mSize + 2];
		for (int i = 0; i < mSize; ++i)
			mDat[i] = dat[i];
		mDat[mSize] = c;
		++mSize;
		mDat[mSize] = 0;
		delete[]dat;
	}
	void string::pop_back()
	{
		if (empty())
			throw std::logic_error(__func__);
		char *dat = mDat;
		mDat = new char[--mSize + 1];
		for (int i = 0; i < mSize; ++i)
			mDat[i] = dat[i];
		mDat[mSize] = 0;
		delete[]dat;
	}
}