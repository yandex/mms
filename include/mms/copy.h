#pragma once

#include "type_traits.h"
#include "cast.h"
#include "impl/offsets.h"

namespace mms {

/**
 * Converts an Mmapped record into its Standalone counterpart.
 */
template<class TSA>
inline void copy(const typename impl::MmappedType<TSA>::type& from, TSA& to);


namespace impl {

template<class TSA, bool HasTraverseFields>
struct CopyHelper {};

template<class TSA>
struct CopyHelper<TSA, false> {
    static void copy(const typename impl::MmappedType<TSA>::type& from, TSA& to)
    {
        to = from;
    }
};

template<class TSA>
struct CopyHelper<TSA, true> {

    class CopyAction {
    public:
        explicit CopyAction(OfsConsumeIter ofs): ofs_(ofs) {}

        template<class U>
        void operator()(const U& dest)
        {
            typedef typename impl::MmappedType<U>::type Mmapped;
            const Mmapped* src = reinterpret_cast<const Mmapped*>(*ofs_);
            mms::copy(*src, const_cast<U&>(dest));
            ++ofs_;
        }

    private:
        OfsConsumeIter ofs_;
    };

    static void copy(const typename impl::MmappedType<TSA>::type& from, TSA& to)
    {
        Offsets ofs = offsets(from);
        traverseFields(to, ActionFacade<CopyAction>(
            CopyAction(OfsConsumeIter(ofs))
        ));
    }
};

template<class Value, class From, class To>
inline To& copyRange(const From& from, To& to)
{
    to.erase(to.begin(), to.end());
    for (typename From::const_iterator i = from.begin(), ie = from.end(); i != ie; ++i) {
        Value t;
        mms::copy(*i, t);
        to.insert(to.end(), t); // Works for both sequenced and associative containers
    }
    return to;
}

}

template<class TSA>
inline void copy(const typename impl::MmappedType<TSA>::type& from, TSA& to)
{
    typedef typename impl::MmappedType<TSA>::type TMM;
    impl::CopyHelper<TSA,
        impl::HasTraverseFields<TSA>::value &&
        impl::HasTraverseFields<TMM>::value
    >::copy(from, to);
}


template<class TSA>
inline void read(const void* ptr, TSA& dest)
{
    copy(*reinterpret_cast<const typename impl::MmappedType<TSA>::type*>(ptr), dest);
}

template<class TSA>
inline void read(const void* buffer, size_t size, TSA& dest)
{
    copy(cast<typename impl::MmappedType<TSA>::type>(buffer, size), dest);
}


template<class TSA>
inline TSA read(const void* ptr)
{
    TSA t;
    read(ptr, t);
    return t;
}

template<class TSA>
inline TSA read(const void* buffer, size_t size)
{
    TSA t;
    read(buffer, size, t);
    return t;
}

} // namespace mms
