#pragma once
#include <stdexcept>
#include <memory>
#include <deque>
namespace cov {
	namespace allocator_policy {
		static bool full_manual=false;
		static bool delay_construction=true;
		static bool allocate_on_demand=true;
		static bool memory_alignment=true;
		static std::size_t max_recycle_count=1000;
	}
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
				throw std::logic_error("Used an uninitialized object.");
			return *mSource;
		}
		const T& operator*() const
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
			return *mSource;
		}
		T* operator->()
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
			return mSource;
		}
		const T* operator->() const
		{
			if(!usable())
				throw std::logic_error("Used an uninitialized object.");
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
	template<typename T> extern std::allocator<T> sourceHolder<T>::sourceManager;
	template<typename T> const T* sourceHolder<T>::hintAddr=nullptr;
	template<typename T> extern typename allocator<T>::collector allocator<T>::mCollector;
}