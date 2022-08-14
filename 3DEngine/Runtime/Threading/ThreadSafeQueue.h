#pragma once

#include <mutex>

template<typename _Ty, size_t capacity>
class Queue
{
public:

    inline bool push_back(const _Ty& item)
    {
        bool result = false;
        mLock.lock();
        size_t next = (mHead + 1) % capacity;
        if (next != mTail)
        {
            mData[mHead] = item;
            mHead = next;
            result = true;
        }
        mLock.unlock();
        return result;
    }
    inline bool pop_front(_Ty& item)
    {
        bool result = false;
        mLock.lock();
        if (mTail != mHead)
        {
            item = mData[mTail];
            mTail = (mTail + 1) % capacity;
            result = true;
        }
        mLock.unlock();
        return result;
    }


private:

	_Ty mData[capacity];
	size_t mHead = 0, mTail = 0;
	std::mutex mLock;

};