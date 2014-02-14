#pragma once

#ifdef MMS_FEATURES_HASH
#    error another hash have already been used
#else
#    define MMS_FEATURES_HASH CXX11
#endif

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace mms {

namespace impl { template<size_t N> struct HashHelper {}; }

template<class T>
size_t hash_value(const T& t, impl::HashHelper<sizeof(std::hash<T>)>* = 0)
    { return std::hash<T>()(t); }

template<class T>
struct hash: public std::unary_function<T, size_t> {
    size_t operator()(const T& t) const { return hash_value(t); }
};

namespace impl {

template<class T>
inline size_t hash_combine(size_t hash, const T& t)
{
    return hash ^ (std::hash<T>()(t) + 0x9e3779b9 + (hash << 6) + (hash >> 2)); // stolen from boost
}

template<class Iter>
inline size_t hash_range(Iter begin, Iter end)
{
    size_t h = 0;
    for (; begin != end; ++begin)
        h = hash_combine(h, *begin);
    return h;
}

template<class K, class V, template<class> class Hash, template<class> class Eq>
struct unordered_map_base {
    typedef std::unordered_map< K, V, Hash<K>, Eq<K> > type;
};

template<class T, template<class> class Hash, template<class> class Eq>
struct unordered_set_base {
    typedef std::unordered_set< T, Hash<T>, Eq<T> > type;
};

} // namespace impl
} // namespace mms
