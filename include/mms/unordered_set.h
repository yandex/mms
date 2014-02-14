// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#ifdef MMS_FEATURES_HASH

#include "impl/defs.h"
#include "impl/hashtable.h"
#include "type_traits.h"
#include "version.h"

namespace mms {

template<
    class P, class T, template<class> class Hash = mms::hash,
    template<class> class Eq = std::equal_to
> class unordered_set;


template<class T, template<class> class Hash, template<class> class Eq>
class unordered_set<Mmapped, T, Hash, Eq>: public impl::Hashtable<T, T, Hash, Eq, impl::Identity> {
private:
    typedef impl::Hashtable<T, T, Hash, Eq, impl::Identity> Base;
public:
    typedef unordered_set<Mmapped, T, Hash, Eq> MmappedType;
    static FormatVersion formatVersion(Versions& vs) { return vs.dependent<T>("unordered_set"); }
};

template<class T, template<class> class Hash, template<class> class Eq>
class unordered_set<Standalone, T, Hash, Eq>: public impl::unordered_set_base<T, Hash, Eq>::type {
private:
    typedef typename impl::unordered_set_base<T, Hash, Eq>::type Base;
    typedef impl::Hashtable<
        typename mms::MmappedType<T>::type,
        typename mms::MmappedType<T>::type,
        Hash, Eq, impl::Identity
    > Impl;
public:
    typedef unordered_set<Mmapped, typename mms::MmappedType<T>::type, Hash, Eq> MmappedType;

    unordered_set() {}
    unordered_set(const Base& s): Base(s) {}
    unordered_set(const unordered_set& s): Base(s) {}

    template<class Iter> unordered_set(Iter begin, Iter end): Base(begin, end) {}
    unordered_set(const MmappedType& s): Base(s.begin(), s.end()) {}
    unordered_set& operator = (const unordered_set& s) { Base::operator = (s); return *this; }
    unordered_set& operator = (const MmappedType& s) { Base::assign(s.begin(), s.end()); return *this; }

#if MMS_USE_CXX11
    unordered_set(unordered_set&& s): Base(std::move(s)) {}
    unordered_set(Base&& s): Base(std::move(s)) {}
    unordered_set(std::initializer_list<T> l): Base(l) {}
    unordered_set& operator = (unordered_set&& s) { Base::operator = (std::move(s)); return *this; }
#endif

    template<class Writer>
    size_t writeData(Writer& w) const { return Impl::writeData(w, *this); }

    template<class Writer>
    size_t writeField(Writer& w, size_t dataPos) const { return Impl::writeField(w, *this, dataPos); }
};

template<class T, template<class> class Hash, template<class> class Eq>
inline size_t hash_value(const unordered_set<Mmapped, T, Hash, Eq>& s)
    { return impl::hash_range(s.begin(), s.end()); }

} // namespace mms

#endif
