#pragma once
#include "covstdlib.hpp"
namespace cov
{
	template<typename> class list;
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
			throw cov::error("E0005");
		node *current = it.mDat;
		node *newdat = new node(current->backward(), new T(dat), current);
		current->backward()->forward(newdat);
		current->backward(newdat);
		++mSize;
	}
	template < typename...Args > void emplace(base_iterator & it, Args && ... args)
	{
		if(!it.usable())
			throw cov::error("E0005");
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
			throw cov::error("E0005");
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
			throw cov::error("E0005");
		return *mFront.forward()->data();
	}
	const T & front() const
	{
		return this->front();
	}
	T & back()
	{
		if(!mBack.backward()->usable())
			throw cov::error("E0005");
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
			throw cov::error("E0005");
		return *this->mDat->data();
	}
	virtual const T & data() const
	{
		return this->data();
	}
	virtual void data(const T & dat)
	{
		if(this->mDat == nullptr)
			throw cov::error("E0005");
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
			throw cov::error("E0005");
		this->mDat = this->mDat->forward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw cov::error("E0005");
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
			throw cov::error("E0005");
		this->mDat = this->mDat->backward();
	}
	virtual void backward() override
	{
		if(this->mDat == nullptr)
			throw cov::error("E0005");
		this->mDat = this->mDat->forward();
	}
};