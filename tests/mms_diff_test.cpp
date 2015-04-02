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

#include <mms/cast.h>
#include <mms/string.h>
#include <mms/vector.h>
#include <mms/map.h>
#include <mms/set.h>
#include <mms/writer.h>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>

template<class T>
struct IntCmp: std::binary_function<T, T, bool>
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        return boost::lexical_cast<int>(lhs.c_str() + 1)
            < boost::lexical_cast<int>(rhs.c_str() + 1);
    }
};

template<class P>
struct Test {
    typedef mms::map<P, mms::string<P>, int, IntCmp> Map;
    typedef mms::set<P, mms::string<P>, IntCmp > Set;
    typedef mms::vector<P, mms::string<P> > Vector;

    Test() {}
    Test(int ii, const std::string& s): i(ii), str(s) {}

    int i;
    mms::string<P> str;
    Map m;
    Set s;
    Vector v;

    template<class A> void traverseFields(A a) const
        { a(i)(str)(m)(s)(v); }
};

template<class A, class B>
std::ostream& operator << (std::ostream& out, const std::pair<A, B>& p)
{
    return out << p.first << " => " << p.second;
}

template<class Iter>
std::string join(Iter begin, Iter end, const std::string& sep)
{
    if (begin == end) {
        return std::string();
    }

    std::ostringstream out;
    out << *begin;
    for (++begin; begin != end; ++begin) {
        out << sep << *begin;
    }
    return out.str();
}

std::string genString(int id)
{
    return "%" + boost::lexical_cast<std::string>(10 * id + 13);
}

int genInt(int id)
{
    return 13 * id + 17;
}


typedef mms::vector< mms::Mmapped, Test<mms::Mmapped> > MTests;

void checkTests(const MTests& t2)
{
    int pos = 0;
    for (MTests::const_iterator i = t2.begin(), ie = t2.end();
            i != ie; ++i, ++pos) {
        BOOST_CHECK_EQUAL(i->i, genInt(pos));
        BOOST_CHECK_EQUAL(i->str[0], i->str[0]);
        BOOST_CHECK_EQUAL(i->str, i->str);
        BOOST_CHECK(!(i->str > i->str));
        BOOST_CHECK(!(i->str < i->str));
        BOOST_CHECK((i->str >= i->str));
        BOOST_CHECK((i->str <= i->str));
        BOOST_CHECK(!(i->str != i->str));

        BOOST_CHECK_EQUAL(i->str, genString(pos));
        BOOST_CHECK(!(i->str > genString(pos)));
        BOOST_CHECK(!(i->str < genString(pos)));
        BOOST_CHECK((i->str >= genString(pos)));
        BOOST_CHECK((i->str <= genString(pos)));
        BOOST_CHECK(!(i->str != genString(pos)));

        BOOST_CHECK_EQUAL(genString(pos), i->str);
        BOOST_CHECK(!(genString(pos) > i->str));
        BOOST_CHECK(!(genString(pos) < i->str));
        BOOST_CHECK((genString(pos) >= i->str));
        BOOST_CHECK((genString(pos) <= i->str));
        BOOST_CHECK(!(genString(pos) != i->str));

        Test<mms::Mmapped>::Vector::const_iterator
            vIt = i->v.begin(), vEnd = i->v.end();
        Test<mms::Mmapped>::Map::const_iterator
            mIt = i->m.begin(), mEnd = i->m.end();
        Test<mms::Mmapped>::Set::const_iterator
            sIt = i->s.begin(), sEnd = i->s.end();

        BOOST_CHECK_EQUAL(i->v.size(), 2);
        BOOST_CHECK_EQUAL(i->m.size(), 2);
        BOOST_CHECK_EQUAL(i->s.size(), 2);

        int idx = 0;
        for (; vIt != vEnd; ++vIt, ++mIt, ++sIt, ++idx) {
            BOOST_CHECK(mIt != mEnd);
            BOOST_CHECK(sIt != sEnd);
            BOOST_CHECK_EQUAL(*vIt, genString(pos * 2 + idx));
            BOOST_CHECK_EQUAL(*sIt, genString(pos * 2 + idx));
            BOOST_CHECK_EQUAL(mIt->first, genString(pos * 2 + idx));
            BOOST_CHECK_EQUAL(mIt->second, genInt(pos * 2 + idx));
        }
        BOOST_CHECK(mIt == mEnd);
        BOOST_CHECK(sIt == sEnd);

        BOOST_CHECK_EQUAL(i->m[genString(pos*2)], genInt(pos*2));
        BOOST_CHECK_EQUAL(i->m[genString(pos*2).c_str()], genInt(pos*2));
        BOOST_CHECK_EQUAL(i->s.count(genString(pos * 2)), 1);
        BOOST_CHECK_EQUAL(i->s.count(genString(pos * 2 + 1)), 1);
        BOOST_CHECK_EQUAL(i->s.count(genString(pos * 2 + 2)), 0);
    }
}

BOOST_AUTO_TEST_CASE( difficult_test )
{
    mms::vector< mms::Standalone, Test<mms::Standalone> > v;
    int COUNT = 20;
    for (int i = 0; i != COUNT; ++i) {
        v.push_back(Test<mms::Standalone>(genInt(i), genString(i)));
        Test<mms::Standalone>& t = v.back();
        t.m.insert(std::make_pair(genString(i * 2),
                    genInt(i * 2)));
        t.m.insert(std::make_pair(genString(i * 2 + 1),
                    genInt(i * 2 + 1)));
        t.s.insert(genString(i * 2));
        t.s.insert(genString(i * 2 + 1));
        t.v.push_back(genString(i * 2));
        t.v.push_back(genString(i * 2 + 1));
    }

    std::stringstream out;
    mms::Writer w(out);
    size_t ofs = mms::safeWrite(w, v);
    std::string buf = out.str();

    typedef mms::vector< mms::Mmapped, Test<mms::Mmapped> > MTests;
    checkTests(mms::cast<MTests>(buf.c_str(), buf.size()));
    checkTests(*reinterpret_cast<const MTests*>(buf.c_str() + ofs));
}
