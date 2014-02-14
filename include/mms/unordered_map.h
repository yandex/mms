// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#ifdef MMS_FEATURES_HASH

#include "impl/defs.h"
#include "cast.h"
#include "impl/hashtable.h"
#include "type_traits.h"
#include "version.h"
#include "impl/pair.h"

namespace mms {

template<
    class P, class K, class V,
    template<class> class Hash = mms::hash, template<class> class Eq = std::equal_to
> class unordered_map;


template<class K, class V, template<class> class Hash, template<class> class Eq>
class unordered_map<Mmapped, K, V, Hash, Eq>:
    public impl::Hashtable< K, std::pair<K, V>, Hash, Eq, impl::Select1st>
{
    typedef impl::Hashtable< K, std::pair<K, V>, Hash, Eq, impl::Select1st> Base;
public:
    typedef V mapped_type;

    const V& at(const K& k) const
    {
        typename Base::const_iterator i = Base::find(k);
        if (i == Base::end())
            throw std::out_of_range("unordered_map::at()");
        return i->second;
    }

    const V& operator[](const K& k) const { return at(k); }

    const V& at(const K& k, const V& dflt) const
    {
        typename Base::const_iterator i = Base::find(k);
        return (i != Base::end()) ? i->second : dflt;
    }

    typedef unordered_map<Mmapped, K, V, Hash, Eq> MmappedType;
    static FormatVersion formatVersion(Versions& vs) { return vs.dependent<K, V>("unordered_map"); }
};


template<class K, class V, template<class> class Hash, template<class> class Eq>
class unordered_map<Standalone, K, V, Hash, Eq>: public impl::unordered_map_base<K, V, Hash, Eq>::type {
    typedef typename impl::unordered_map_base<K, V, Hash, Eq>::type Base;
    typedef impl::Hashtable<
        typename MmappedType<K>::type,
        std::pair<typename MmappedType<K>::type, typename MmappedType<V>::type>,
        Hash, Eq, impl::Select1st
    > Impl;
public:
    typedef unordered_map<Mmapped, typename MmappedType<K>::type, typename MmappedType<V>::type, Hash, Eq> MmappedType;

    unordered_map(): Base() {}
    unordered_map(const Base& m): Base(m) {}
    unordered_map(const unordered_map& m): Base(m) {}

    template<class Iter> unordered_map(Iter begin, Iter end): Base(begin, end) {}
    unordered_map(const MmappedType& mm): Base(mm.begin(), mm.end()) {}
    unordered_map& operator = (const unordered_map& mm) { Base::operator = (mm); return *this; }
    unordered_map& operator = (const MmappedType& mm) { Base::assign(mm.begin(), mm.end()); return *this; }

#if MMS_USE_CXX11
    unordered_map(unordered_map&& m): Base(std::move(m)) {}
    unordered_map(Base&& m): Base(std::move(m)) {}
    unordered_map(std::initializer_list<typename Base::value_type> l): Base(l) {}
    unordered_map& operator = (unordered_map&& mm) { Base::operator = (std::move(mm)); return *this; }
#endif

    template<class Writer>
    size_t writeData(Writer& w) const { return Impl::writeData(w, *this); }

    template<class Writer>
    size_t writeField(Writer& w, size_t dataPos) const { return Impl::writeField(w, *this, dataPos); }
};

template<class K, class V, template<class> class Hash, template<class> class Eq>
inline size_t hash_value(const unordered_map<Mmapped, K, V, Hash, Eq>& m)
    { return impl::hash_range(m.begin(), m.end()); }

} // namespace mms

#endif // MMS_FEATURES_HASH

