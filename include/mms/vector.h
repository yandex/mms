// mms -- a memory-mapped storage.
//
// See http://wiki.yandex-team.ru/jandekskarty/development/fordevelopers/libs/mms
// for short instruction about its usage.

#pragma once

#include "type_traits.h"
#include "writer.h"
#include "copy.h"
#include "version.h"
#include "impl/tags.h"
#include "impl/container.h"
#include "impl/fwd.h"

#include <vector>

namespace mms {

namespace vector_impl {

template <bool fastWrite>
struct WriteSelector;

template <>
struct WriteSelector<true> {
    template <class Writer, class Iterator>
    size_t operator()(Writer& w, Iterator begin, Iterator end) {
        impl::align(w);
        size_t res = w.pos();

        w.write(
                &*begin,
                (end - begin)*sizeof(
                    typename std::iterator_traits<Iterator>::value_type));
        return res;
    }
};

template <>
struct WriteSelector<false> {
    template <class Writer, class Iterator>
    size_t operator()(Writer& w, Iterator begin, Iterator end) {
        return impl::writeRange(w, begin, end);
    }
};

} // namespace vector_impl

template<class T>
class vector<Mmapped, T>: public impl::Sequence<T>
{
public:
    static FormatVersion formatVersion(Versions& vs) { return vs.dependent<T>("vector"); }
    typedef vector<Mmapped, T> MmappedType;
};


template<class T>
class vector<Standalone, T>: public std::vector<T>
{
private:
    typedef std::vector<T> Base;
public:
    typedef vector<Mmapped, typename impl::MmappedType<T>::type> MmappedType;

    vector() {}
    vector(const Base& v):Base(v) {}
    vector(const vector& v): Base(v) {}

    explicit vector(size_t size, const T& val = T()): Base(size, val) {}
    template<class Iter> vector(Iter begin, Iter end): Base(begin, end) {}

    explicit vector(const MmappedType& v) { impl::copyRange<T>(v, *this); }
    vector& operator = (const vector& v) { Base::operator = (v); return *this; }
    vector& operator = (const MmappedType& v) { return impl::copyRange<T>(v, *this); }

#if MMS_USE_CXX11
    vector(Base&& v): Base(std::move(v)) {}
    vector(vector&& v): Base(std::move(v)) {}
    vector(std::initializer_list<T> l): Base(l) {}
    vector& operator = (vector&& v) { Base::operator = (std::move(v)); return *this; }
#endif

    template<class Writer>
    size_t writeData(Writer& w) const
    {
        return vector_impl::WriteSelector<
                    mms::type_traits::is_trivial<T>::value &&
                    !mms::type_traits::is_same<T, bool>::value>()(
                w, Base::begin(), Base::end());
    }

    template<class Writer>
    size_t writeField(Writer& w, size_t pos) const
    {
        return impl::writeRef(w, pos, Base::size());
    }

};

#ifdef MMS_FEATURES_HASH
template<class T>
inline size_t hash_value(const vector<Mmapped, T>& v)
    { return impl::hash_range(v.begin(), v.end()); }
#endif

// size of vector is just size and offset for any type
// base empty class optimization must work in this case
MMS_STATIC_ASSERT(sizeof(vector<Mmapped, void*>) == 2 * sizeof(size_t), "mms::vector size mismatch");

}//namespace mms
