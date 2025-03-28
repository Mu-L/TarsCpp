﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#pragma once

#include <vector>
#include <stdlib.h>
#include <string.h>
#include "util/tc_platform.h"

using namespace std;

namespace tars
{
/////////////////////////////////////////////////
/**
 * @file tc_loop_queue.h 
 * @brief 循环队列,大小固定 . 
 * @brief Circular queue, fixed size.
 *  
 */
/////////////////////////////////////////////////

template<typename T>
class TC_LoopQueue
{
public:
    typedef vector<T> queue_type;

    TC_LoopQueue(size_t iSize)
    {
        //做个保护 最多不能超过 1000000
        //Make a protection. No more than 1000000.
        // assert(iSize<1000000);
        _iBegin = 0;
        _iEnd = 0;
        _iCapacitySub = iSize;
        _iCapacity = iSize + 1;
        _p=(T*)malloc(_iCapacity*sizeof(T));
        //_p= new T[_iCapacity];
    }
    ~TC_LoopQueue()
    {
        free(_p);
        //delete _p;
    }

    bool push_back(const T &t,bool & bEmpty, size_t & iBegin, size_t & iEnd)
    {
        bEmpty = false;
        //uint32_t iEnd = _iEnd;
        iEnd = _iEnd;
        iBegin = _iBegin;
        if((iEnd > _iBegin && iEnd - _iBegin < 2) ||
                ( _iBegin > iEnd && _iBegin - iEnd > (_iCapacity-2) ) )
        {
            return false;
        }
        else
        { 
            memcpy(_p+_iBegin,&t,sizeof(T));
            //*(_p+_iBegin) = t;

            if(_iEnd == _iBegin)
                bEmpty = true;

            if(_iBegin == _iCapacitySub)
                _iBegin = 0;
            else
                _iBegin++;

            if(!bEmpty && 1 == size())
                bEmpty = true;

            return true;
        }
    }

    bool push_back(const T &t,bool & bEmpty)
    {
        bEmpty = false;
        size_t iEnd = _iEnd;
        if((iEnd > _iBegin && iEnd - _iBegin < 2) ||
                ( _iBegin > iEnd && _iBegin - iEnd > (_iCapacity-2) ) )
        {
            return false;
        }
        else
        { 
            memcpy(_p+_iBegin,&t,sizeof(T));
            //*(_p+_iBegin) = t;

            if(_iEnd == _iBegin)
                bEmpty = true;

            if(_iBegin == _iCapacitySub)
                _iBegin = 0;
            else
                _iBegin++;

            if(!bEmpty && 1 == size())
                bEmpty = true;
#if 0
            if(1 == size())
                bEmpty = true;
#endif

            return true;
        }
    }

    bool push_back(const T &t)
    {
        bool bEmpty;
        return push_back(t,bEmpty);
    }

    bool push_back(const queue_type &vt)
    {
        size_t iEnd=_iEnd;
        if(vt.size()>(_iCapacity-1) ||
                (iEnd>_iBegin && (iEnd-_iBegin)<(vt.size()+1)) ||
                ( _iBegin>iEnd && (_iBegin-iEnd)>(_iCapacity-vt.size()-1) ) )
        {
            return false;
        }
        else
        { 
            for(size_t i=0;i<vt.size();i++)
            {
                memcpy(_p+_iBegin,&vt[i],sizeof(T));
                //*(_p+_iBegin) = vt[i];
                if(_iBegin == _iCapacitySub)
                    _iBegin = 0;
                else
                    _iBegin++;
            }
            return true;
        }
    }

    bool pop_front(T &t)
    {
        if(_iEnd==_iBegin)
        {
            return false;
        }
        memcpy(&t,_p+_iEnd,sizeof(T));
        //t = *(_p+_iEnd);

        if(_iEnd == _iCapacitySub)
            _iEnd = 0;
        else
            _iEnd++;
        return true;
    }

    bool pop_front()
    {
        if(_iEnd==_iBegin)
        {
            return false;
        }
        if(_iEnd == _iCapacitySub)
            _iEnd = 0;
        else
            _iEnd++;
        return true;
    }

    bool get_front(T &t)
    {
        if(_iEnd==_iBegin)
        {
            return false;
        }
        memcpy(&t,_p+_iEnd,sizeof(T));
        //t = *(_p+_iEnd);
        return true;
    }

    bool empty()
    {
        if(_iEnd == _iBegin)
        {
            return true;
        }
        return false;
    }

	size_t size()
    {
		size_t iBegin=_iBegin;
		size_t iEnd=_iEnd;
        if(iBegin<iEnd)
            return iBegin+_iCapacity-iEnd;
        return iBegin-iEnd;
    }

	size_t getCapacity()
    {
        return _iCapacity;
    }

private:
    T * _p;
    size_t _iCapacity;
	size_t _iCapacitySub;
    std::atomic<size_t> _iBegin;
    std::atomic<size_t> _iEnd;
//	size_t _iBegin;
//	size_t _iEnd;
};

}


