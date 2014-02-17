
/*
 * features/hash/boost.h -- include this file to utilize Boost version
 *                          of unordered containers
 *
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

#ifdef MMS_FEATURES_HASH
#    error another hash have already been used
#else
#    define MMS_FEATURES_HASH BOOST
#endif

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace mms {

template<class T> struct hash: public boost::hash<T> {};

namespace impl {

template<class Iter>
size_t hash_range(Iter begin, Iter end) { return boost::hash_range(begin, end); }

template<class K, class V, template<class> class Hash, template<class> class Eq>
struct unordered_map_base {
    typedef boost::unordered_map< K, V, Hash<K>, Eq<K> > type;
};

template<class T, template<class> class Hash, template<class> class Eq>
struct unordered_set_base {
    typedef boost::unordered_set< T, Hash<T>, Eq<T> > type;
};


} // namespace impl
} // namespace mms
