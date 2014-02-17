
/*
 * mms/optional.h -- mms version of boost::optional
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

#ifdef MMS_FEATURES_OPTIONAL

#include "writer.h"
#include "version.h"
#include "impl/fwd.h"
#include "impl/tags.h"
#include "impl/container.h"

namespace mms {

template<typename T>
class optional<Mmapped, T> : public impl::Offset
{
private:
    typedef bool (optional::*unspecified_bool_type)() const;

public:
    optional(const optional& c)
        : impl::Offset(c)
    {
        if (c.offset_ == 0) {
            offset_ = 0;
        }
    }

    optional& operator = (const optional& c) {
        impl::Offset::operator=(c);
        if (c.offset_ == 0) {
            offset_ = 0;
        }
        return *this;
    }

    bool is_initialized() const { return offset_ != 0; }
    bool operator!() const { return !is_initialized(); }

    operator unspecified_bool_type() const
        { return is_initialized() ? &optional::is_initialized : 0; }

    const T * get_ptr() const
    {
        if (!is_initialized()) {
            throw std::logic_error("mms::optional::get_ptr");
        }
        return ptr<T>();
    }

    const T & get_value_or(const T & defaultValue) const
    {
        if (!is_initialized()) {
            return defaultValue;
        }
        return *ptr<T>();
    }

    const T * operator->() const { return get_ptr(); }
    const T & operator*() const { return *get_ptr(); }
    const T & get() const { return *get_ptr(); }

    bool operator == (const optional& c) const
    {
        return is_initialized() == c.is_initialized() &&
            (!is_initialized() || get() == c.get());
    }

    bool operator != (const optional& c) const
        { return !(*this == c); }

    static FormatVersion formatVersion(Versions& vs)
        { return vs.dependent<T>("optional"); }

    typedef optional<Mmapped, T> MmappedType;
};


template<class T>
class optional<Standalone, T>: public impl::optional_base<T>::type {
private:
    typedef typename impl::optional_base<T>::type Base;

public:
    typedef optional<Mmapped, typename impl::MmappedType<T>::type> MmappedType;

    optional() {}
    optional(const T& value): Base(value) {}
    optional& operator = (const T& value) { Base::operator=(value); return *this; }

    optional& operator = (const MmappedType& t)
    {
        if (t)
            Base::operator=(T(*t));
        return *this;
    }

    optional(const MmappedType& t) { *this = t; }

    template<class Writer>
    size_t writeData(Writer& w) const
    {
        return *this ? impl::write(w, Base::get()) : impl::nullOfs();
    }

    template<class Writer>
    size_t writeField(Writer& w, size_t pos) const
    {
        return impl::writeOffset(w, pos);
    }
};


} // namespace mms

#endif // defined(MMS_FEATURES_OPTIONAL)
