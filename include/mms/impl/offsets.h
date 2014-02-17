
/*
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
