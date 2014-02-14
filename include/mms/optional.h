// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

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
