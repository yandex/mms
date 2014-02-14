#pragma once

#include "defs.h"
#include "../type_traits.h"

namespace mms {
namespace impl {

class StoreOffsets {
public:
    explicit StoreOffsets(OfsPopulateIter ofs): ofs_(ofs) {}

    template<class U>
    void operator()(const U& u)
    {
        *ofs_++ = reinterpret_cast<size_t>(&u);
    }
private:
    OfsPopulateIter ofs_;
};

template<class T>
inline Offsets offsets(const T& t)
{
    Offsets ofs;
    traverseFields(t, ActionFacade<StoreOffsets>(
        StoreOffsets(OfsPopulateIter(ofs))
    ));
    return ofs;
}

} // namespace impl
} // namespace mms
