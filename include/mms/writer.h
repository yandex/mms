// mms -- a memory-mapped storage.
//
// See http://wiki.yandex-team.ru/jandekskarty/development/fordevelopers/libs/mms
// for short instruction about its usage.

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
