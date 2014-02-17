
/*
 * cast.h -- a more convinient wrappers for mmapped objects casting
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

#include "writer.h"
#include <cstddef>
#include <stdexcept>

namespace mms {

template <class T>
const T& unsafeCast(const char* buffer, size_t size) {
    return *reinterpret_cast<const T*>(buffer + size - sizeof(T));
}

template <class T>
const T& safeCast(const char* buffer, size_t size) {
    if (size < sizeof(T) + sizeof(FormatVersion)) {
        throw std::length_error(
            "Size of buffer is less than needed for format version and class"
            );
    }
    const T& object = unsafeCast<T>(buffer, size);
    impl::checkFormatVersion(
            object,
            unsafeCast<size_t>(buffer, size - sizeof(T))
        );
    return object;
}

template <class T>
const T& cast(const char* buffer, size_t size) {
    return unsafeCast<T>(buffer, size);
}

}//namespace mms
