/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SAFE_FIFO_HPP
#define SAFE_FIFO_HPP

template <class T>
class SafeFifo
{
public:
	void push(T& t)
	{
		SimpleMutexHolder smh(mutex_);
		data_.push_back(t);
	}

	T pop()
	{
		SimpleMutexHolder smh(mutex_);
		T returned = *data_.begin();
		data_.pop_front();
		return returned;
	}

	uint count()
	{
		SimpleMutexHolder smh(mutex_);
		return data_.size();
	}

private:
	mutable SimpleMutex mutex_;
	std::list<T> data_;
};

#endif // SAFE_FIFO_HPP
