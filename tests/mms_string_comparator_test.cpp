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

#include <mms/string.h>

#include <iostream>
#include "tools.h"

template <class S1, class S2>
void checkEqualStrings(const S1& lhs, const S2& rhs)
{
        BOOST_CHECK_EQUAL(lhs.size(), rhs.size());
        BOOST_CHECK_EQUAL(lhs.length(), rhs.length());

        for(size_t i = 0; i < lhs.length(); ++i) {
            BOOST_CHECK_EQUAL(lhs[i], rhs[i]);
        }

        BOOST_CHECK_EQUAL(lhs, rhs);
        BOOST_CHECK(!(lhs > rhs));
        BOOST_CHECK(!(lhs < rhs));
        BOOST_CHECK((lhs >= rhs));
        BOOST_CHECK((lhs <= rhs));
        BOOST_CHECK(!(lhs != rhs));
        
        BOOST_CHECK_EQUAL(lhs, lhs);
        BOOST_CHECK(!(lhs > lhs));
        BOOST_CHECK(!(lhs < lhs));
        BOOST_CHECK((lhs >= lhs));
        BOOST_CHECK((lhs <= lhs));
        BOOST_CHECK(!(lhs != lhs));
}

void checkEqualStringsAll(const std::string& s)
{
    mms::string<mms::Mmapped> mm = mmappedString(s);
    mms::string<mms::Standalone> sa(s);
    checkEqualStrings(s, mm);
    checkEqualStrings(s, sa);

    checkEqualStrings(mm, s);
    checkEqualStrings(mm, sa);
    checkEqualStrings(mm, mm);

    checkEqualStrings(sa, s);
    checkEqualStrings(sa, mm);
    checkEqualStrings(sa, sa);
}

BOOST_AUTO_TEST_CASE( equal_cmp_test )
{
    checkEqualStringsAll("123");
    checkEqualStringsAll("dakljsklekjl");
    checkEqualStringsAll(std::string("3290\037289", 10));
}

template <class S1, class S2>
void checkLessStrings(const S1& lhs, const S2& rhs)
{
        BOOST_CHECK(!(lhs == rhs));
        BOOST_CHECK(!(rhs == lhs));
        BOOST_CHECK(lhs != rhs);
        BOOST_CHECK(rhs != lhs);

        BOOST_CHECK(lhs < rhs);
        BOOST_CHECK(lhs <= rhs);

        BOOST_CHECK(!(lhs > rhs));
        BOOST_CHECK(!(lhs >= rhs));

        BOOST_CHECK(!(rhs < lhs));
        BOOST_CHECK(!(rhs <= lhs));

        BOOST_CHECK(rhs > lhs);
        BOOST_CHECK(rhs >= lhs);
}

void checkLessStringsAll(const std::string& s1, const std::string& s2)
{
    mms::string<mms::Mmapped> mm1 = mmappedString(s1);
    mms::string<mms::Standalone> sa1(s1);

    mms::string<mms::Mmapped> mm2 = mmappedString(s2);
    mms::string<mms::Standalone> sa2(s2);

    checkLessStrings(s1, s2);
    checkLessStrings(s1, mm2);
    checkLessStrings(s1, sa2);

    checkLessStrings(mm1, s2);
    checkLessStrings(mm1, mm2);
    checkLessStrings(mm1, sa2);

    checkLessStrings(sa1, s2);
    checkLessStrings(sa1, mm2);
    checkLessStrings(sa1, sa2);
}

BOOST_AUTO_TEST_CASE( less_cmp_test )
{
    checkLessStringsAll("123", "1234");
    checkLessStringsAll(std::string("123\0", 4), "1234");
    checkLessStringsAll("123", std::string("123\0", 4));
    checkLessStringsAll(std::string("123\02", 5), std::string("123\03", 5));
    checkLessStringsAll(std::string("123\02\0", 6),
            std::string("123\02\02", 7));

    checkLessStringsAll("123432", "444");
}


