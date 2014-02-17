
/*
 * mms/map.h -- mms version of std::map
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

#include "type_traits.h"
#include "writer.h"
#include "copy.h"
#include "version.h"
#include "impl/defs.h"
#include "impl/tags.h"
#include "impl/container.h"
#include "impl/pair.h"
#include "impl/fwd.h"

#include <map>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace mms {

// TODO: store map as two separate vectors instead of vector<pair>
template<class K, class V, template<class> class Cmp>
class map<Mmapped, K, V, Cmp>: public impl::SortedSequence<
    K,
    std::pair<const K, V>,
    Cmp<K>,
    impl::Select1st<std::pair<const K, V> >
> {
private:
    typedef impl::SortedSequence<K, std::pair<const K, V>,
            Cmp<K>, impl::Select1st< std::pair<const K, V> > > Base;

public:
    typedef V mapped_type;

    static FormatVersion formatVersion(Versions& vs)
    { return vs.dependent<K, V>("map"); }

    const V& operator [](const K& key) const { return at(key); }

    const V& at(const K& key, const V& dflt) const
    {
        typename Base::const_iterator i = Base::find(key);
        return (i != Base::end()) ? i->second : dflt;
    }

    const V& at(const K& key) const
    {
        typename Base::const_iterator i = Base::find(key);
        if (i == Base::end())
            throw std::out_of_range("mms::map::operator[]");
        return i->second;
    }

    typedef map<Mmapped, K, V, Cmp> MmappedType;
};


template<class K, class V, template<class> class Cmp>
class map<Standalone, K, V, Cmp>: public std::map<K, V, Cmp<K> >
{
private:
    typedef std::map<K, V, Cmp<K> > Base;

public:
    typedef map<
        Mmapped,
        typename impl::MmappedType<K>::type,
        typename impl::MmappedType<V>::type,
        Cmp
    > MmappedType;

    map() {}
    map(const Base& m): Base(m) {}
    map(const map& m): Base(m) {}

    template<class Iter> map(Iter begin, Iter end): Base(begin, end) {}
    map(const MmappedType& m) { impl::copyRange< std::pair<K,V> >(m, *this); }
    map& operator = (const MmappedType& m) { return impl::copyRange< std::pair<K,V> >(m, *this); }
    map& operator = (const map& m) { Base::operator = (m); return *this; }

#if MMS_USE_CXX11
    map(Base&& m): Base(std::move(m)) {}
    map(map&& m): Base(std::move(m)) {}
    map(std::initializer_list<typename Base::value_type> l): Base(l) {}
    map& operator = (map&& m) { Base::operator = (std::move(m)); return *this; }
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
template<class K, class V, template<class> class Cmp>
inline size_t hash_value(const map<Mmapped, K, V, Cmp>& m)
    { return impl::hash_range(m.begin(), m.end()); }
#endif

}//namespace mms
