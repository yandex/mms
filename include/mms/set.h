// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#include "type_traits.h"
#include "writer.h"
#include "copy.h"
#include "version.h"
#include "impl/defs.h"
#include "impl/tags.h"
#include "impl/container.h"
#include "impl/fwd.h"

#include <set>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

namespace mms {

template<class T, template<class> class Cmp>
class set<Mmapped, T, Cmp>: public impl::SortedSequence<T, T, Cmp<T>, impl::Identity<T> >
{
public:
    static FormatVersion formatVersion(Versions& vs)
        { return vs.dependent<T>("set"); }

    typedef set<Mmapped, T, Cmp> MmappedType;
};


template<class T, template<class> class Cmp>
class set<Standalone, T, Cmp>: public std::set<T, Cmp<T> > {
private:
    typedef std::set<T, Cmp<T> > Base;

public:
    typedef set<
        Mmapped,
        typename impl::MmappedType<T>::type,
        Cmp
    > MmappedType;

    set() {}
    set(const Base& s): Base(s) {}
    set(const set& s): Base(s) {}
    set(const MmappedType& s) { impl::copyRange<T>(s, *this); }

    template<class Iter> set(Iter begin, Iter end):
            Base(begin, end) {}

    set& operator = (const set& s) { Base::operator = (s); return *this; }
    set& operator = (const MmappedType& s) { return impl::copyRange<T>(s, *this); }

#if MMS_USE_CXX11
    set(Base&& s): Base(std::move(s)) {}
    set(set&& s): Base(std::move(s)) {}
    set(std::initializer_list<T> l): Base(l) {}
    set& operator = (set&& s) { Base::operator = (std::move(s)); return *this; }
#endif

    template<class Writer>
    size_t writeData(Writer& w) const
    {
        return impl::writeRange(w, Base::begin(), Base::end());
    }

    template<class Writer>
    size_t writeField(Writer& w, size_t pos) const
    {
        return impl::writeRef(w, pos, Base::size());
    }
};

#ifdef MMS_FEATURES_HASH
template<class T, template<class> class Cmp>
inline size_t hash_value(const set<Mmapped, T, Cmp>& s)
    { return impl::hash_range(s.begin(), s.end()); }
#endif

}//namespace mms
