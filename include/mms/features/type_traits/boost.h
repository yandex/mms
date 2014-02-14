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
