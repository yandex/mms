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


#include "test_config.h"

#include <boost/test/unit_test.hpp>

#include "ptr_recursive.h"

#include <mms/vector.h>
#include <mms/writer.h>
#include <mms/string.h>
#include <mms/ptr.h>

template<class P>
struct Pointee: public mms::Pointee {
    int x;
    Pointee(int xx): x(xx) {}
    template<class A> void traverseFields(A a) const { a(x); }
};

template<class P>
struct Pointers {
    mms::ptr< P, Pointee<P> > ptr1;
    mms::ptr< P, Pointee<P> > ptr2;
    mms::ptr< P, Pointee<P> > ptr3;
    mms::ptr< P, Pointee<P> > ptr4;
    template<class A> void traverseFields(A a) const { a(ptr1)(ptr2)(ptr3); }
};

BOOST_AUTO_TEST_CASE(mms_ptr)
{
    Pointers<mms::Standalone> p;
    p.ptr1 = new Pointee<mms::Standalone>(10);
    p.ptr2 = new Pointee<mms::Standalone>(20);
    p.ptr3 = p.ptr1;
    p.ptr4 = 0;

    std::stringstream out;
    mms::Writer w(out);
    size_t pos = mms::write(w, p);

    std::string buf = out.str();
    const Pointers<mms::Mmapped>& pp = *reinterpret_cast<const Pointers<mms::Mmapped>*>(buf.c_str() + pos);
    BOOST_CHECK_EQUAL(pp.ptr1->x, 10);
    BOOST_CHECK_EQUAL(pp.ptr2->x, 20);
    BOOST_CHECK(pp.ptr1.get() == pp.ptr3.get());
    BOOST_CHECK(pp.ptr4.get() == 0);

    delete p.ptr1.get();
    delete p.ptr2.get();
}


BOOST_AUTO_TEST_CASE(mms_backrefs)
{
    mms::shared_ptr< mms::Standalone, OuterPtr<mms::Standalone> > st;
    st = new OuterPtr<mms::Standalone>(4);

    std::stringstream out;
    size_t pos = mms::write(out, st);

    std::string buf = out.str();
    typedef mms::shared_ptr< mms::Mmapped, OuterPtr<mms::Mmapped> > Mmapped;
    const Mmapped& mm = *reinterpret_cast<const Mmapped*>(buf.c_str() + pos);

    BOOST_REQUIRE(mm->inners.size() == 4);
    BOOST_CHECK(mm->inners[0]->x == 0);
    BOOST_CHECK(mm->inners[0]->outer.get() == mm.get());
    BOOST_CHECK(mm->inners[1]->x == 1);
    BOOST_CHECK(mm->inners[1]->outer.get() == mm.get());
    BOOST_CHECK(mm->inners[2]->x == 2);
    BOOST_CHECK(mm->inners[2]->outer.get() == mm.get());
    BOOST_CHECK(mm->inners[3]->x == 3);
    BOOST_CHECK(mm->inners[3]->outer.get() == mm.get());


    std::stringstream out2;
    mms::Writer w2(out2);
    mms::string<mms::Standalone> header("header");
    mms::write(w2, header);
    size_t pos2 = mms::write(w2, st);

    std::string buf2 = out2.str();
    const Mmapped& mm2 = *reinterpret_cast<const Mmapped*>(buf2.c_str() + pos2);

    BOOST_REQUIRE(mm2->inners.size() == 4);
    BOOST_CHECK(mm2->inners[0]->x == 0);
    BOOST_CHECK(mm2->inners[0]->outer.get() == mm2.get());
    BOOST_CHECK(mm2->inners[1]->x == 1);
    BOOST_CHECK(mm2->inners[1]->outer.get() == mm2.get());
    BOOST_CHECK(mm2->inners[2]->x == 2);
    BOOST_CHECK(mm2->inners[2]->outer.get() == mm2.get());
    BOOST_CHECK(mm2->inners[3]->x == 3);
    BOOST_CHECK(mm2->inners[3]->outer.get() == mm2.get());

}
