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

#include <mms/vector.h>
#include <mms/map.h>
#include <mms/set.h>
#include <mms/string.h>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

struct TestAccess {
    static inline const ssize_t& offset(const mms::impl::Container& c) {
        return c.ofs_.offset_;
    }

    static inline const size_t& size(const mms::impl::Container& c) {
        return c.size_;
    }
};

template <class C, class K, class I, class Cmp>
bool isAligned(const mms::impl::SortedSequence<C, K, I, Cmp>& v);

template <class T>
bool isAligned(const mms::impl::Sequence<T>& v);

template <class T>
inline
typename boost::enable_if_c<
    sizeof(typename boost::remove_reference<T>::type) == sizeof(size_t),
    bool
>::type
isAligned(T ptr)
{
    return reinterpret_cast<size_t>(&ptr) % sizeof(void*) == 0;
}

template <class T>
inline
typename boost::enable_if_c<
    sizeof(typename boost::remove_reference<T>::type) != sizeof(size_t) &&
    boost::is_pod<typename boost::remove_reference<T>::type>::value,
    bool
>::type
isAligned(T)
{
    return true;
}

template <class F, class S>
bool isAligned(const std::pair<F, S>& p)
{
    return isAligned(p.first) && isAligned(p.second);
}

inline bool isAligned(const mms::impl::Container& c)
{
    return isAligned(TestAccess::offset(c)) &&
        isAligned(TestAccess::size(c)) &&
        (TestAccess::offset(c)) % sizeof(void*) == 0;
}

template <class T>
bool isAligned(const mms::impl::Sequence<T>& v)
{
    if (!isAligned(static_cast<const mms::impl::Container&>(v))) {
        return false;
    }
    typename mms::impl::Sequence<T>::const_iterator it = v.begin();
    for(; it != v.end(); ++it) {
        if (!isAligned(*it)) {
            return false;
        }
    }
    return true;
}

template <class K, class T, class Cmp, class XK>
bool isAligned(const mms::impl::SortedSequence<K, T, Cmp, XK>& v)
{
    if (!isAligned(static_cast<const mms::impl::Container&>(v))) {
        return false;
    }
    typename mms::impl::SortedSequence<K, T, Cmp, XK>::const_iterator
        it = v.begin();
    for(; it != v.end(); ++it) {
        if (!isAligned(*it)) {
            return false;
        }
    }
    return true;
}
