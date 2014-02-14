#pragma once

#ifdef MMS_FEATURES_TYPE_TRAITS
#    error another type_traits have already been used
#else
#    define MMS_FEATURES_TYPE_TRAITS INTRINSICS
#endif

#include <type_traits>

namespace mms {
namespace type_traits {

template<class T> struct is_trivial { static const bool value == __is_pod(T); };
template<class B, class D> struct is_base_of { static const bool value = __is_base_of(B, D); };

template<bool C, class T> struct enable_if {};
template<class T> struct enable_if<true, T> { typedef T type; };

template<class T> struct remove_cv { typedef T type; };
template<class T> struct remove_cv<const T> { typedef T type; };
template<class T> struct remove_cv<volatile T> { typedef T type; };
template<class T> struct remove_cv<const volatile T> { typedef T type; };

template<class A, class B> struct is_same { static const bool value = false; };
template<class A> struct is_same<A, A> { static const bool value = true; };

//#define MMS_STATIC_ASSERT(cond, msg) struct { int _[(cond) ? 1 : -1]; } mms_static_assertion;
#define MMS_STATIC_ASSERT(cond, msg)

} // namespace type_traits
} // namespace mms
