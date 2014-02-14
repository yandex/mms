#pragma once

#ifdef MMS_FEATURES_TYPE_TRAITS
#    error another type_traits have already been used
#else
#    define MMS_FEATURES_TYPE_TRAITS CXX11
#endif

#include <type_traits>

namespace mms {
namespace type_traits {

template<class T> struct is_trivial: public std::is_trivial<T> {};
template<class B, class D> struct is_base_of: public std::is_base_of<B, D> {};
template<class A, class B> struct is_same: public std::is_same<A, B> {};
template<class T> struct remove_cv: public std::remove_cv<T> {};
template<bool C, class T> struct enable_if: public std::enable_if<C, T> {};

#define MMS_STATIC_ASSERT(cond,msg) static_assert(cond, msg)

} // namespace type_traits
} // namespace mms
