
/*
 * mms/writer.h -- public definitions of mms::write() and similar functions.
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

#include "impl/defs.h"
#include <iostream>

namespace mms {

class Writer: public impl::WriterBase {
public:
    explicit Writer(std::ostream& out): out_(&out) {}

    void write(const void* data, size_t size)
    {
        out_->write(static_cast<const char*>(data), size);
        advance(size);
    }

private:
    std::ostream* out_;
};

/// Writes element (with its format version) to a stream.
template< class T >
size_t safeWrite(Writer& w, const T& t);

/// Same as above. Returned position will be relative
/// to current stream position.
template< class T >
size_t safeWrite(std::ostream& out, const T& t);


/// Aliases to safeWrite()
template<class T> size_t write(Writer& w, const T& t)
    { return safeWrite(w, t); }

template<class T> size_t write(std::ostream& out, const T& t)
    { return safeWrite(out, t); }


/// Writes element without its version, thus making
/// safeCast() inappropriate.
template< class T >
size_t unsafeWrite(Writer& w, const T& t);

template< class T >
size_t unsafeWrite(std::ostream& out, const T& t);

}//namespace mms

#define MMS_WRITER_IMPL_H
#include "impl/writer-impl.h"
#undef MMS_WRITER_IMPL_H
