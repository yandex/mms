
/*
 * unordered_set.h -- mms version of (std|boost)::unordered_set
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

#include "impl/config.h"

#ifdef MMS_FEATURES_HASH

#include "copy.h"
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
    unordered_set& operator = (const MmappedType& s) { return impl::copyRange<T>(s, *this); }

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
