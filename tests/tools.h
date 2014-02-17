/*
 * Copyright (c) 2011-2014 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru>
 *
 * This file is part of mms, the memory-mapped storage library.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#pragma once

#include <mms/cast.h>
#include "align_tools.h"

#include <mms/vector.h>
#include <mms/string.h>
#include <mms/set.h>
#include <mms/string.h>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/mpl/bool.hpp>

#include <functional>
#include <sstream>

inline boost::ptr_vector<std::string>& allStrings()
{
    static boost::ptr_vector<std::string> s;
    return s;
}

template <class K, class V, template<class> class Cmp>
inline mms::map<mms::Mmapped, K, V, Cmp> mmappedMap(const std::map<K, V, Cmp<K> >& m)
{
    std::stringstream out;
    mms::map<mms::Standalone, K, V, Cmp> mmsMap(m);
    mms::unsafeWrite(out, mmsMap);
    allStrings().push_back(new std::string(out.str()));
    return mms::cast<mms::map<mms::Mmapped, K, V, Cmp> >(
            allStrings().back().c_str(), allStrings().back().size());
}

template <class T, template<class> class Cmp>
inline mms::set<mms::Mmapped, T, Cmp> mmappedSet(const std::set<T, Cmp<T> >& s)
{
    std::stringstream out;
    mms::set<mms::Standalone, T, Cmp> mmsSet(s);
    mms::unsafeWrite(out, mmsSet);
    allStrings().push_back(new std::string(out.str()));
    return mms::cast<mms::set<mms::Mmapped, T, Cmp> >(
            allStrings().back().c_str(), allStrings().back().size());
}

template <class T>
inline mms::vector<mms::Mmapped, T> mmappedVector(const std::vector<T>& v)
{
    std::stringstream out;
    mms::vector<mms::Standalone, T> mmsVector(v);
    mms::unsafeWrite(out, mmsVector);
    allStrings().push_back(new std::string(out.str()));
    return mms::cast<mms::vector<mms::Mmapped, T> >(
            allStrings().back().c_str(), allStrings().back().size());
}

inline mms::string<mms::Mmapped> mmappedString(const std::string& s)
{
    std::stringstream out;
    mms::string<mms::Standalone> mmsString(s);
    mms::unsafeWrite(out, mmsString);
    allStrings().push_back(new std::string(out.str()));
    return mms::cast<mms::string<mms::Mmapped> >(
            allStrings().back().c_str(), allStrings().back().size());
}

inline size_t machineWord(const void* p)
{
    return reinterpret_cast<size_t>(p) / sizeof(void*);
}

inline size_t alignedSize(size_t size)
{
    if (size == 0) return 0;
    return ((size - 1) / sizeof(void*) + 1) * sizeof(void*);
}

template <class T>
struct GenT
{
    T operator() (int seed) const
    {
        return static_cast<T>(seed * 17);
    }
};

template <>
struct GenT<bool>
{
    bool operator() (int seed) const
    {
        return (seed * 17) % 15 >= 7;
    }
};

template <class T>
inline std::vector<T> genVector(int seed, size_t size)
{
    std::vector<T> result;
    for(size_t i = 0; i < size; ++i) {
        result.push_back(GenT<T>()(seed));
        seed *= 189;
    }
    return result;
}

template <class T>
inline std::set<T> genSet(int seed, size_t size)
{
    std::set<T> result;
    for(size_t i = 0; i < size; ++i) {
        result.insert(GenT<T>()(seed));
        seed *= 189;
    }
    return result;
}

template <class K, class V>
inline std::map<K, V> genMap(int seed, size_t size)
{
    std::map<K, V> result;
    for(size_t i = 0; i < size; ++i) {
        result.insert(std::pair<K, V>(GenT<K>()(seed), GenT<V>()(seed * 19)));
        seed *= 185;
    }
    return result;
}

template <class T>
struct LexicographicalCompare
    :public std::binary_function<T, T, bool>
{
    bool operator() (const T& t1, const T& t2) const
    {
        return lexicographical_compare(t1.begin(), t1.end(),
                t2.begin(), t2.end());
    }
};
