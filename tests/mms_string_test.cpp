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

#include "tools.h"
#include <mms/string.h>

#include <iostream>

BOOST_AUTO_TEST_CASE( string_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t i = 0; i <= MAX_SIZE; ++i) {
        std::string s(i, 'a');
        BOOST_CHECK_EQUAL(s, mmappedString(s));
        std::string s0(i, '\0');
        BOOST_CHECK_EQUAL(s0, mmappedString(s0));
    }
    BOOST_CHECK_EQUAL(mms::string<mms::Mmapped>(), std::string());
    BOOST_CHECK_EQUAL(mms::string<mms::Mmapped>("abc", 3), std::string("abc"));
    BOOST_CHECK_THROW(mms::string<mms::Mmapped>("abc", 1), std::exception);

    BOOST_CHECK_EQUAL(static_cast<std::string>(mms::string<mms::Mmapped>("abc", 3)), "abc");
    BOOST_CHECK_EQUAL(static_cast<std::string>(mms::string<mms::Mmapped>("a\00c", 3)), std::string("a\00c", 3));
}

BOOST_AUTO_TEST_CASE( allign_string_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t i = 0; i <= MAX_SIZE; ++i) {
        std::string s(i, 'a');
        std::stringstream out;
        mms::Writer w(out);
        mms::string<mms::Standalone> mmsString(s);

        size_t ofs = mms::unsafeWrite(w, mmsString);
        std::string buf = out.str();
        mms::string<mms::Mmapped> mappedString =
            *reinterpret_cast<const mms::string<mms::Mmapped> *>(
                    buf.c_str() + ofs);

        BOOST_CHECK(ofs % sizeof(void*) == 0);
        BOOST_CHECK(isAligned(mappedString));
        BOOST_CHECK_EQUAL(w.pos(), alignedSize(sizeof(char) * (i + 1))
                + 2 * sizeof(size_t) //reference size
        );
    }
}



