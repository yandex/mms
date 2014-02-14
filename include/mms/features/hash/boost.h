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
