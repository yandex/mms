
/*
 * mms/ptr.h -- support for pointers in mmapped objects
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
#include "impl/container.h"
#include "writer.h"
#include "type_traits.h"
#include "version.h"

#include <memory>
#include <stdexcept>

namespace mms {

namespace impl {

class LayoutHelper;
template<class T> class PtrBase;

class PointeeHeader {
private:
    static const uint32_t Magic = 0x00445450; // = "PTD\0";
    static const size_t PosMask = ~size_t(1);
    static const size_t HasPos = 1;

public:
    PointeeHeader(): magic_(Magic), writerId_(), pos_(0) {}

    template<class T>
    static PointeeHeader& get(T* ptr)
    {
        char* p = reinterpret_cast<char*>(ptr);
        PointeeHeader* hdr = reinterpret_cast<PointeeHeader*>(p - sizeof(PointeeHeader));
        if (hdr->magic_ != Magic)
            throw std::logic_error("pointee magic not found (pointee allocated on stack?)");
        return *hdr;
    }

    template<class Writer>
    bool isWritten(Writer* writer) const { return writerId_ == writer->id(); }

    size_t position() const { return pos_ & PosMask; }

    template<class Writer>
    void beganWriting(Writer* writer) { writerId_ = writer->id(); }

    void beganWriting(LayoutHelper* writer)
    {
        if (writer->id() != writerId_ && writer->writer()->id() != writerId_) {
            // Will write to another writer, clean up from previous write
            pos_ = 0;
            writer->addPointee(pos_);
        }
        writerId_ = writer->id();
    }

    void endedWriting(size_t pos)
    {
        if (pos & ~PosMask)
            throw std::logic_error("object position is not a multiple of machine-word-size");
        if (!(pos_ & HasPos)) pos_ = pos | HasPos;
    }
    void clean() { pos_ = 0; }

private:
    uint32_t magic_;
    WriterID writerId_;
    size_t pos_;
};

} // namespace impl

/**
 * A base class for all user classes pointed to via mms::ptr<>
 * and similar classes.
 * This MUST be the first base of any user class, and
 * it MUST be created in the heap.
 */
class Pointee {
public:
    void* operator new(size_t size)
    {
        typedef impl::PointeeHeader Hdr;
        char* ptr = static_cast<char*>(::operator new(size + sizeof(Hdr)));
        new(ptr) Hdr();
        return ptr + sizeof(Hdr);
    }

    void operator delete(void* p)
    {
        typedef impl::PointeeHeader Hdr;
        char* ptr = reinterpret_cast<char*>(p) - sizeof(Hdr);
        reinterpret_cast<Hdr*>(ptr)->~PointeeHeader();
        ::operator delete(ptr);
    }

protected:
    Pointee() { checkMagic(); }
    Pointee(const Pointee&) { checkMagic(); }

private:
    void checkMagic() { impl::PointeeHeader::get(this); }

    // Since ponintees aciqure memory in a very special manner,
    // it is impossible to provide working definitions for these operators
    void* operator new(size_t, const std::nothrow_t&);
    void* operator new(size_t, void*);
    void* operator new[](size_t);
    void* operator new[](size_t, const std::nothrow_t&);
    void* operator new[](size_t, void*);
    void operator delete(void*, const std::nothrow_t&);
    void operator delete(void*, void*);
    void operator delete[](void*);
    void operator delete[](void*, const std::nothrow_t&);
    void operator delete[](void*, void*);
};


namespace impl {

template<class T>
class PlainPtr {
public:
    typedef T element_type;

    PlainPtr(): p_(0) {}
    PlainPtr(T* ptr): p_(ptr) {}

    T* get() const { return p_; }
    T& operator *() const { return *get(); }
    T* operator->() const { return get(); }

    void reset(T* ptr) { p_ = ptr; }
    PlainPtr& operator = (T* ptr) { reset(ptr); return *this; }

    typedef PlainPtr<T> MmappedType;

private:
    T* p_;
};

template<class T>
class MmappedPtr {
public:
    const T* get() const { return (ofs_.ptr< MmappedPtr<T> >() != this) ? ofs_.ptr<T>() : 0; }
    const T& operator *() const { return *get(); }
    const T* operator->() const { return get(); }

    static FormatVersion formatVersion(Versions& vs)
        { return vs.dependent<T>("ptr"); }

private:
    Offset ofs_;
};

template<class H>
class PtrBase: public H {
public:
    typedef typename H::element_type element_type;

    ~PtrBase() {
        typedef mms::type_traits::is_base_of<Pointee, element_type> Assert;
        MMS_STATIC_ASSERT(Assert::value, "class mms::ptr points to must be derived from mms::Pointee");
    }

    PtrBase(): H() {}
    PtrBase(element_type* ptr): H(ptr) {}

    template<class Writer>
    size_t writeData(Writer& w) const
    {
        // Actually writeData() is called twice:
        // 1) with Writer=mms::impl::LayoutHelper to determine
        //    the exact location of pointees in output stream;
        // 2) with Writer=mms::Writer to perform actual writing.

        if (H::get()) {
            impl::PointeeHeader& hdr = impl::PointeeHeader::get(H::get());
            if (!hdr.isWritten(&w)) {
                hdr.beganWriting(&w);
                hdr.endedWriting(impl::write(w, *H::get()));
            }
            return hdr.position();
        } else {
            return impl::nullOfs();
        }
    }

    template<class Writer>
    size_t writeField(Writer& w, size_t pos) const { return impl::writeOffset(w, pos); }
};

} // namespace impl


#define MMS_DEFINE_PTR(ptr,base) \
    template<class P, class T> class ptr; \
    \
    template<class T> \
    class ptr<Standalone, T>: public impl::PtrBase< base<T> > { \
        typedef impl::PtrBase< base<T> > Base; \
    public: \
        ptr() {} \
        ptr(T* p): Base(p) {} \
        \
        /* We cannot use impl::MmappedType here due to possible   */ \
        /* circular dependencies (such as                         */ \
        /*   class A { ptr<B>; };                                 */ \
        /*   class B { ptr<A>; };                                 */ \
        /* ), which leads to references to impl::MmappedType      */ \
        /* being instantiated.                                    */ \
        typedef ptr<Mmapped, typename impl::MmappedTypeAux< \
            typename mms::type_traits::remove_cv<T>::type, \
            sizeof(impl::hasMmappedType<T>(0)) == sizeof(impl::Yes) \
        >::type> MmappedType; \
        \
        template<class Writer> \
        size_t writeData(Writer& w) const { return Base::writeData(w); } \
        \
        template<class Writer> \
        size_t writeField(Writer& w, size_t pos) const { return Base::writeField(w, pos); } \
    }; \
    \
    template<class T> \
    class ptr<Mmapped, T>: public impl::MmappedPtr<T> { \
    public: \
        typedef ptr<Mmapped, T> MmappedType; \
    }

MMS_DEFINE_PTR(ptr, impl::PlainPtr);

#if MMS_USE_CXX11
MMS_DEFINE_PTR(unique_ptr, std::unique_ptr);
MMS_DEFINE_PTR(shared_ptr, std::shared_ptr);
#else
MMS_DEFINE_PTR(auto_ptr, std::auto_ptr);
// TODO: add support for boost::shared_ptr
#endif

#undef MMS_DEFINE_PTR

} // namespace mms
