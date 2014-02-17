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
#include <mms/set.h>
#include <mms/map.h>
#include <mms/optional.h>
#include <mms/impl/pair.h>
#include <mms/string.h>

#include <mms/ptr.h>

#define DUPLICATE_STRUCT_ENTIRE \
    int a, b, c; \
    double d, e; \
    mms::vector<P, mms::vector<P, int> > f, g; \
    mms::set<P, mms::vector<P, int> > h, i; \
    mms::optional<P, mms::vector<P, int> > j, k; \
    mms::string<P> l, m; \
    mms::map<P, mms::vector<P, int>, mms::set<P, int> > n, o; \
\
    template <class Op> \
    void traverseFields(Op op) const \
    { op(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)(m)(n)(o); }

template <class P>
struct DuplicateStruct
{
    DUPLICATE_STRUCT_ENTIRE
};

template <class P>
struct DuplicateStructWithOldStyleFormatVersion
{
    static mms::FormatVersion formatVersion() { return 21093; }
    DUPLICATE_STRUCT_ENTIRE
};

template <class P>
struct DuplicateStructWithNewStyleFormatVersion
{
    static mms::FormatVersion formatVersion(mms::Versions&) { return 21093; }
    DUPLICATE_STRUCT_ENTIRE
};

BOOST_AUTO_TEST_CASE(mms_version_test)
{
    typedef mms::Standalone S;
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<int>()),
            1269558899428530646ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<double>()),
            1269558897037139485ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<std::pair<int,double> >()),
            1427110276983316619ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<mms::vector<S,int> >()),
            5908932320964122251ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<mms::set<S,int> >()),
            7610443259246909065ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<mms::map<S,int,double> >()),
            3018027871432005466ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<mms::optional<S,int> >()),
            2964192378110927965ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<mms::string<S> >()),
            493519961992996882ull);

    BOOST_CHECK_EQUAL((mms::impl::formatVersion<
            mms::optional<S,mms::vector<S, int> > >()),
            9268741120498963650ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<
            mms::vector<S, mms::set<S, mms::vector<S, int> > > >()),
            1285385650391235107ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<DuplicateStruct<S> >()),
            5848163006848953414ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<
            DuplicateStructWithOldStyleFormatVersion<S> >()),
            5848173095171722083ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<
            DuplicateStructWithNewStyleFormatVersion<S> >()),
            5848173095171722083ull);
    BOOST_CHECK_EQUAL((mms::impl::formatVersion<
            mms::shared_ptr<S, OuterPtr<S> > >()),
            10833728818936060638ull);
}
