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
