#pragma once
#include <stdexcept>
#include <memory>
#include <list>
namespace cov {
template<typename T>
class sourceHolder {
public:
	static constexpr bool memAlign=true;
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
		if(memAlign&&hintAddr!=nullptr) {
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
	bool usable() const
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
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		*mSource=dat;
	}
	T& data()
	{
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		return *mSource;
	}
	const T& data() const
	{
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
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
	virtual ~sourceHolder()=default;
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
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		return *mSource;
	}
	const T& operator*() const
	{
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		return *mSource;
	}
	T* operator->()
	{
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		return mSource;
	}
	const T* operator->() const
	{
		if(!mUsable)
			throw std::logic_error("Used of uninitialized object.");
		return mSource;
	}
	operator bool() const
	{
		return mAvailable;
	}
};
template<typename> class allocator;
template<typename T>
class holder {
	friend class allocator<T>;
	std::shared_ptr<sourceHolder<T>> mDat;
	static void deleter(sourceHolder<T>*);
public:
	holder():mDat(new sourceHolder<T>(),deleter)
	{
		mDat->allocate();
	}
	holder(const holder&)=default;
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
	bool operator==(const holder& obj) const
	{
		return this->dat()==obj.dat();
	}
	bool operator!=(const holder& obj) const
	{
		return this->dat()!=obj.dat();
	}
};
template<typename T>
class allocator {
	friend void holder<T>::deleter(sourceHolder<T>*);
	class collector {
		friend class allocator;
		friend void holder<T>::deleter(sourceHolder<T>*);
		std::list<sourceHolder<T>*>recycleStack;
		~collector();
	};
	static collector mCollector;
public:
	constexpr static std::size_t stack_max_size=1000;
	static inline holder<T> allocate()
	{
		if(!mCollector.recycleStack.empty()) {
			holder<T> ret(mCollector.recycleStack.front());
			mCollector.recycleStack.pop_front();
			return ret;
		}
		return holder<T>();
	}
	template<typename...Args>static inline holder<T> allocate(Args&&...args)
	{
		if(!mCollector.recycleStack.empty()) {
			holder<T> ret(mCollector.recycleStack.front());
			mCollector.recycleStack.pop_front();
			ret.raw_data().construct(args...);
			return ret;
		}
		return holder<T>(args...);
	}
	static inline void recycle()
	{
		while(mCollector.recycleStack.size()>stack_max_size) {
			mCollector.recycleStack.back()->deallocate();
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
	if(ptr!=nullptr) {
		ptr->destroy();
		allocator<T>::mCollector.recycleStack.push_front(ptr);
	}
	allocator<T>::recycle();
}
template<typename T> std::allocator<T> sourceHolder<T>::sourceManager;
template<typename T> const T* sourceHolder<T>::hintAddr=nullptr;
template<typename T> typename allocator<T>::collector allocator<T>::mCollector;
}