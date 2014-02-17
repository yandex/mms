
/*
 * features/type_traits/boost.h -- include this file to utilize Boost
 *                                 version of type traits
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

#ifdef MMS_FEATURES_TYPE_TRAITS
#    error another type_traits have already been used
#else
#    define MMS_FEATURES_TYPE_TRAITS BOOST
#endif

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

namespace mms {
namespace type_traits {

template<class T> struct is_trivial: public boost::is_pod<T> {};
template<class B, class D> struct is_base_of: public boost::is_base_of<B, D> {};
template<class A, class B> struct is_same: public boost::is_same<A, B> {};
template<class T> struct remove_cv: public boost::remove_cv<T> {};
template<bool C, class T> struct enable_if: public boost::enable_if_c<C, T> {};

#define MMS_STATIC_ASSERT(cond, msg) BOOST_STATIC_ASSERT(cond)

} // namespace type_traits
} // namespace mms
