
/*
 * impl/writer-impl.h -- an implementation of mms writer
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

#ifndef MMS_WRITER_IMPL_H
#error "direct inlusion of writer-impl.h is not allowed"
        "include writer.h instead"
#endif

#include "offsets.h"
#include "fwd.h"
#include "../type_traits.h"
#include "../version.h"

#include <vector>
#include <stdexcept>
#include <iostream>

#include <assert.h>


namespace mms {
namespace impl {

template<class Writer, class T>
inline void writePod(Writer& w, const T& t)
{
    w.write(&t, sizeof(t));
}

template<class Writer>
inline void addZeroes(Writer& w, size_t count)
{
    if (count <= sizeof(void*)) {
        static const char buf[sizeof(void*)] = {0};
        if (count)
            w.write(buf, count);
    } else {
        static const std::vector<char> buf(4096, 0);
        for (size_t chunk; (chunk = std::min(count, buf.size())) != 0; count -= chunk)
            w.write(&buf[0], chunk);
    }
}

inline size_t sanitizeAlignment(size_t alignment)
{
    // alignment must be a power of 2
    assert((alignment & (alignment - 1)) == 0);
    return std::min(alignment, sizeof(void*));
}

inline bool isAligned(size_t pos, size_t size, size_t alignment = sizeof(void*))
{
    assert(size != 0);
    alignment = sanitizeAlignment(alignment);
    return (pos & ~(alignment - 1)) == ((pos + size - 1) & ~(alignment - 1))
        && !(pos & (alignment - 1));
}

template<class Writer>
inline void align(Writer& w, size_t alignment = sizeof(void*))
{
    addZeroes(w, (-w.pos()) & (sanitizeAlignment(alignment) - 1));
}

template<class Writer>
inline void alignTransient(Writer& w, size_t alignment = sizeof(void*))
{
    w.putTransient((-w.transientPos()) & (sanitizeAlignment(alignment) - 1));
}


inline size_t nullOfs() { return size_t(-1); }

template<class Writer>
inline size_t writeOffset(Writer& w, size_t dataPos)
{
    size_t offsetPos = w.pos();
    writePod(w, dataPos != nullOfs() ? (dataPos - offsetPos) : 0);
    return offsetPos;
}

template<class Writer>
inline size_t writeRef(Writer& w, size_t dataPos, size_t size)
{
    size_t offsetPos = writeOffset(w, dataPos);
    writePod(w, size);
    return offsetPos;
}



template<class Writer, class TSA>
void writeData(Writer& w, const TSA& t, OfsPopulateIter ofs);

template<class Writer, class TSA>
void writeField(Writer& w, const TSA& t, OfsConsumeIter ofs);

template<class Writer, class T>
inline size_t write(Writer& w, const T& t, bool writeFormatVersion)
{
    Offsets ofs;
    writeData(w, t, impl::OfsPopulateIter(ofs));
    align(w);
    if (writeFormatVersion) {
        writePod(w, Versions().get<T>());
        align(w);
    }
    size_t fieldPos = w.pos();
    writeField(w, t, impl::OfsConsumeIter(ofs));
    return fieldPos;
}

template<class Writer, class T>
inline size_t write(Writer& w, const T& t) { return write(w, t, false); }

template<class Writer, class Iter>
inline size_t writeRange(Writer& w, Iter begin, Iter end)
{
    align(w);
    impl::Offsets ofs;
    for (Iter i = begin; i != end; ++i) {
        impl::writeData(w, *i, impl::OfsPopulateIter(ofs));
    }
    align(w);
    size_t fieldsPos = w.pos();
    for (Iter i = begin; i != end; ++i) {
        impl::writeField(w, *i, impl::OfsConsumeIter(ofs));
    }
    return fieldsPos;
}


class LayoutHelper: public WriterBase {
public:
    explicit LayoutHelper(const Writer& w): WriterBase(w.pos(), 0), writer_(&w) {}
    void write(const void*, size_t size) { advance(size); }
    const Writer* writer() const { return writer_; }
    
    void addPointee(size_t& pos) { pointees_.push_back(&pos); }
    
    void adjustPointees(ssize_t diff)
    {
        for (std::vector<size_t*>::iterator i = pointees_.begin(), ie = pointees_.end(); i != ie; ++i)
            **i += diff;
    }

private:
    const Writer* writer_;
    std::vector<size_t*> pointees_;
};

template<class Writer, class T>
inline void layOut(Writer& w, const T& t, bool writeVersion)
{
    LayoutHelper h(w);
    impl::write(h, t, writeVersion);
    align(h);
    impl::addZeroes(w, h.transientPos());
    h.adjustPointees(h.transientPos());
}

} //namespace impl


template<class T>
inline size_t safeWrite(Writer& w, const T& t)
{
    impl::layOut(w, t, true);
    return impl::write(w, t, true);
}

template<class T>
inline size_t safeWrite(std::ostream& out, const T& t)
{
    Writer w(out);
    impl::layOut(w, t, true);
    return impl::write(w, t, true);
}

template<class T>
inline size_t unsafeWrite(Writer& w, const T& t)
{
    impl::layOut(w, t, false);
    return impl::write(w, t, false);
}

template<class T>
inline size_t unsafeWrite(std::ostream& out, const T& t)
{
    Writer w(out);
    impl::layOut(w, t, false);
    return impl::write(w, t, false);
}

namespace impl {

template<class Writer, class TSA, bool IsTrivial, bool HasMethods>
struct WriteHelper;

// Case 1: nontrivial types not having `write*' methods.
//         Utilize `<class>::traverseFields()' to find out
//         fields and serialize them recursively.
template<class Writer, class TSA>
struct WriteHelper<Writer, TSA, false, false> {

    class WriteData {
    public:
        WriteData(Writer& w, OfsPopulateIter ofs): w_(&w), ofs_(ofs) {}

        template<class U>
        void operator()(const U& u)
        {
            mms::impl::writeData(*w_, u, ofs_);
        }
    private:
        Writer* w_;
        OfsPopulateIter ofs_;
    };

    class WriteField {
    public:
        WriteField(Writer& w,
               OfsConsumeIter dataOfs,
               OfsConsumeIter fieldOfs):
            w_(&w), dataOfs_(dataOfs),
            fieldOfs_(fieldOfs), startPos_(w.pos()) {}

        template<class U>
        void operator()(const U& u)
        {
            addZeroes(*w_, *fieldOfs_ - bytesWritten());
            ++fieldOfs_;
            mms::impl::writeField(*w_, u, dataOfs_);
        }
    private:
        size_t bytesWritten() const
        {
            return w_->pos() - startPos_;
        }

        Writer* w_;
        OfsConsumeIter dataOfs_;
        OfsConsumeIter fieldOfs_;
        size_t startPos_;
    };

    static void writeData (Writer& w, const TSA& t, OfsPopulateIter ofs)
    {
        traverseFields(t, ActionFacade<WriteData>(WriteData(w, ofs)));
        align(w);
    }

    static void writeField(Writer& w, const TSA& t, OfsConsumeIter dataOfs)
    {
        typedef typename MmappedType<TSA>::type TMM;
        const TMM* tmm = reinterpret_cast<const TMM*>(0);
        Offsets fieldOfs = offsets(*tmm);
        size_t startPos = w.pos();
        traverseFields(t, ActionFacade<WriteField>(
            WriteField(w, dataOfs, OfsConsumeIter(fieldOfs))));
        addZeroes(w, startPos +
                sizeof(typename MmappedType<TSA>::type) - w.pos());
    }

};

// Case 2: trivial types.
//         Just write them as they are.
template<class Writer, class TSA, bool HasMethods>
struct WriteHelper<Writer, TSA, true, HasMethods> {
    static void writeData (Writer&, const TSA&, OfsPopulateIter) {}
    static void writeField(Writer& w, const TSA& t, OfsConsumeIter)
    {
        writePod(w, t);
    }
};

// Case 3: nontrivial types having `write*' methods.
//         Invoke them to do the job.
template<class Writer, class TSA>
struct WriteHelper<Writer, TSA, false, true> {
    static void writeData (Writer& w, const TSA& t, OfsPopulateIter ofs)
    {
        *ofs++ = t.writeData(w);
    }
    static void writeField(Writer& w, const TSA& t, OfsConsumeIter ofs)
    {
        t.writeField(w, *ofs);
        ++ofs;
    }
};

template<class Writer, class TSA>
struct MetaWriteHelper {
    typedef WriteHelper<
        Writer, TSA,
        mms::type_traits::is_trivial<TSA>::value,
        sizeof(hasWriters<TSA>(0, 0)) == sizeof(Yes)
    > type;
};


template<class Writer, class TSA>
inline void writeData(Writer& w, const TSA& t, OfsPopulateIter ofs)
{
    MetaWriteHelper<Writer, TSA>::type::writeData(w, t, ofs);
}

template<class Writer, class TSA>
inline void writeField(Writer& w, const TSA& t, OfsConsumeIter ofs)
{
    MetaWriteHelper<Writer, TSA>::type::writeField(w, t, ofs);
}

}//namespace impl
}//namespace mms

