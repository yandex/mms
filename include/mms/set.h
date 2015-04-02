
/*
 * mms/set.h -- mms version of std::set
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
#include "impl/fwd.h"

#include <set>
#include <stdexcept>
#include <algorithm>

#ifdef MMS_USE_CXX11
#   include <initializer_list>
#endif

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
        return impl::writeRange<typename Base::value_type>(w, Base::begin(), Base::end());
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
