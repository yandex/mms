
/*
 * mms/transient.h -- support for mutable porititions in mmapped data
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

#include "version.h"
#include "impl/tags.h"
#include "impl/container.h"

namespace mms {

template<class P, class T> class transient;

template<class T>
class transient<Mmapped, T> {
public:
    typedef transient<Mmapped, T> MmappedType;
    static FormatVersion formatVersion(Versions& vs) { return vs.dependent<T>("transient"); }

    ~transient()
    {
        MMS_STATIC_ASSERT(mms::type_traits::is_trivial<T>::value,
            "transient nontrivial types are not implemented");
    }

    T* get() const { return ptr(); }
    T& operator *() const { return *get(); }
    T* operator->() const { return get(); }

private:
    impl::Offset ofs_;

    T* ptr() const { return const_cast<T*>(ofs_.ptr<T>()); }
};

template<class T>
class transient<Standalone, T> {
public:
    typedef transient<Mmapped, T> MmappedType;

    template<class Writer>
    size_t writeData(Writer&) const { return impl::nullOfs(); }

    template<class Writer>
    size_t writeField(Writer& w, size_t /*dataPos*/) const
    {
        MMS_STATIC_ASSERT(mms::type_traits::is_trivial<T>::value,
            "transient nontrivial types are not implemented");

        if (!impl::isAligned(w.transientPos(), sizeof(T)))
            impl::alignTransient(w);
        size_t pos = w.transientPos();
        w.putTransient(sizeof(T));
        return impl::writeOffset(w, pos);
    }

    transient() {} // Make it non-POD
};


}
