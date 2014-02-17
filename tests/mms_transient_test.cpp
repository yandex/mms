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

#include <mms/writer.h>
#include <mms/cast.h>
#include <mms/transient.h>
#include <mms/ptr.h>
#include <mms/vector.h>

namespace {
template<class P>
struct Simple {
    int x;
    mms::transient<P, int> y;
    
    template<class A>
    void traverseFields(A a) const { a(x)(y); }
};
} // namespace

BOOST_AUTO_TEST_CASE(mms_transient)
{
    Simple<mms::Standalone> t;
    t.x = 10;
    
    std::stringstream out;
    mms::write(out, t);
    std::string str = out.str();
    const Simple<mms::Mmapped>& m = mms::cast< Simple<mms::Mmapped> >(str.c_str(), str.size());
    
    BOOST_CHECK_EQUAL(m.x, 10);
    *m.y = 20;
    BOOST_CHECK_EQUAL(*m.y, 20);
}


namespace {

template<class P> class Outer;

template<class P>
struct Inner: public mms::Pointee {
    
    int num;
    mms::transient<P, int> data;
    mms::ptr<P, Outer<P> > backref;
    
    explicit Inner(int n, Outer<P>* b): num(n), backref(b) {}
    
    template<class A>
    void traverseFields(A a) const { a(num)(data)(backref); }
};

template<class P>
struct Outer: public mms::Pointee {
    
    int num;
    mms::transient<P, double> data;
    mms::vector<P, mms::shared_ptr<P, Inner<P> > > inners;
    
    explicit Outer(int n, size_t count): num(n)
    {
        for (size_t i = 0; i != count; ++i)
            inners.push_back(new Inner<P>(i+n, this));
    }

    explicit Outer(int n, const mms::shared_ptr<P, Inner<P> >& ptr): num(n)
    {
        inners.push_back(ptr);
    }
    
    template<class A>
    void traverseFields(A a) const { a(num)(data)(inners); }
};

} // namespace

BOOST_AUTO_TEST_CASE(mms_shared_transient)
{
    mms::shared_ptr< mms::Standalone, Outer<mms::Standalone> > st1, st2;
    st1 = new Outer<mms::Standalone>(10, 4);
    st2 = new Outer<mms::Standalone>(20, st1->inners[2]);
    
    std::vector<char> writerBuf(sizeof(mms::Writer), 0);
    mms::Writer& w = *reinterpret_cast<mms::Writer*>(&writerBuf[0]);

    std::stringstream out;
    new (&w) mms::Writer(out);
    size_t pos1 = mms::write(w, st1);
    size_t pos2 = mms::write(w, st2);
    w.~Writer();

    std::stringstream out3;
    new (&w) mms::Writer(out3); // We need both writers created on exactly the same place
    mms::impl::addZeroes(w, 64); // Make sure objects in this writer will have different positions
    size_t pos3 = mms::write(w, st1);
    w.~Writer();

    std::string buf = out.str();
    typedef mms::shared_ptr< mms::Mmapped, Outer<mms::Mmapped> > Mmapped;
    const Mmapped& mm1 = *reinterpret_cast<const Mmapped*>(buf.c_str() + pos1);
    const Mmapped& mm2 = *reinterpret_cast<const Mmapped*>(buf.c_str() + pos2);
        // mm1 and mm2 should share objects they point to...
    
    std::string buf3 = out3.str();
    const Mmapped& mm3 = *reinterpret_cast<const Mmapped*>(buf3.c_str() + pos3);
        // ...but mm3 should possess a private copy of them

    // Smoke tests first
    BOOST_CHECK_EQUAL(mm1->num, 10);
    BOOST_REQUIRE_EQUAL(mm1->inners.size(), 4);
    BOOST_CHECK_EQUAL(mm1->inners[0]->num, 10);
    BOOST_CHECK_EQUAL(mm1->inners[1]->num, 11);
    BOOST_CHECK_EQUAL(mm1->inners[2]->num, 12);
    BOOST_CHECK_EQUAL(mm1->inners[3]->num, 13);
        
    BOOST_CHECK_EQUAL(mm2->num, 20);
    BOOST_REQUIRE_EQUAL(mm2->inners.size(), 1);
    BOOST_CHECK_EQUAL(mm2->inners[0]->num, 12);
    
    BOOST_CHECK_EQUAL(mm3->num, 10);
    BOOST_REQUIRE_EQUAL(mm3->inners.size(), 4);
    BOOST_CHECK_EQUAL(mm3->inners[0]->num, 10);
    BOOST_CHECK_EQUAL(mm3->inners[1]->num, 11);
    BOOST_CHECK_EQUAL(mm3->inners[2]->num, 12);
    BOOST_CHECK_EQUAL(mm3->inners[3]->num, 13);

    *mm1->data = 12.345;
    *mm3->data = 34.567;
    BOOST_CHECK_EQUAL(*mm1->inners[0]->backref->data, 12.345);
    BOOST_CHECK_EQUAL(*mm1->inners[1]->backref->data, 12.345);
    BOOST_CHECK_EQUAL(*mm1->inners[2]->backref->data, 12.345);
    BOOST_CHECK_EQUAL(*mm1->inners[3]->backref->data, 12.345);
    BOOST_CHECK_EQUAL(*mm2->inners[0]->backref->data, 12.345);
    BOOST_CHECK_EQUAL(*mm3->inners[0]->backref->data, 34.567);
    BOOST_CHECK_EQUAL(*mm3->inners[1]->backref->data, 34.567);
    BOOST_CHECK_EQUAL(*mm3->inners[2]->backref->data, 34.567);
    BOOST_CHECK_EQUAL(*mm3->inners[3]->backref->data, 34.567);
    
    *mm1->data = 54.321;
    *mm3->data = 76.543;
    BOOST_CHECK_EQUAL(*mm1->inners[0]->backref->data, 54.321);
    BOOST_CHECK_EQUAL(*mm1->inners[1]->backref->data, 54.321);
    BOOST_CHECK_EQUAL(*mm1->inners[2]->backref->data, 54.321);
    BOOST_CHECK_EQUAL(*mm1->inners[3]->backref->data, 54.321);
    BOOST_CHECK_EQUAL(*mm2->inners[0]->backref->data, 54.321);    
    BOOST_CHECK_EQUAL(*mm3->inners[0]->backref->data, 76.543);
    BOOST_CHECK_EQUAL(*mm3->inners[1]->backref->data, 76.543);
    BOOST_CHECK_EQUAL(*mm3->inners[2]->backref->data, 76.543);
    BOOST_CHECK_EQUAL(*mm3->inners[3]->backref->data, 76.543);
}
