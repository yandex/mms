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

#ifdef MMS_FEATURES_HASH

#include <boost/test/unit_test.hpp>

#include <mms/unordered_map.h>
#include <mms/unordered_set.h>
#include <mms/string.h>
#include <mms/cast.h>
#include <mms/copy.h>
#include <boost/lexical_cast.hpp>

BOOST_AUTO_TEST_CASE(test_hash_default_default)
{
    {//map
    typedef mms::unordered_map<mms::Mmapped, int, int> HashMap;

    HashMap mm;
    BOOST_REQUIRE_EQUAL(mm.size(), 0);
    BOOST_REQUIRE(mm.empty());
    for (unsigned i = 0; i < 100; ++i) {
        BOOST_CHECK(mm.find(i) == mm.end());
        BOOST_CHECK_THROW(mm[i], std::out_of_range);
    }
    }

    {//set
    typedef mms::unordered_set<mms::Mmapped, int> HashSet;

    HashSet ms;
    BOOST_REQUIRE_EQUAL(ms.size(), 0);
    BOOST_REQUIRE(ms.empty());
    for (unsigned i = 0; i < 100; ++i) {
        BOOST_CHECK(ms.find(i) == ms.end());
        BOOST_CHECK_EQUAL(ms.count(i), 0);
    }
    }
}

BOOST_AUTO_TEST_CASE(test_hash_map_simple)
{
    typedef mms::unordered_map<mms::Standalone, mms::string<mms::Standalone>, int> HashMap;

    HashMap st;
    st.insert(std::make_pair("0", 10));
    st.insert(std::make_pair("1", 1));

    BOOST_CHECK_EQUAL(st["0"], 10);
    BOOST_CHECK_EQUAL(st["1"], 1);
    for (unsigned i = 2; i < 100; ++i)
        st.insert(std::make_pair(boost::lexical_cast<std::string>(i), i));

    std::ostringstream out;
    mms::write(out, st);
    std::string buf = out.str();
    const HashMap::MmappedType& mm = mms::cast<HashMap::MmappedType>(buf.c_str(), buf.size());

    BOOST_REQUIRE_EQUAL(mm.size(), 100);
    BOOST_REQUIRE(mm.find("12") != mm.end());
    BOOST_CHECK_EQUAL(mm.find("12")->second, 12);
    BOOST_CHECK_EQUAL(mm["12"], 12);
    BOOST_CHECK_EQUAL(mm["34"], 34);

    BOOST_CHECK(mm.find("nonexistent") == mm.end());
    BOOST_CHECK_THROW(mm["nonexistent"], std::out_of_range);
}

BOOST_AUTO_TEST_CASE(test_hash_set)
{
    typedef mms::unordered_set<mms::Standalone, mms::string<mms::Standalone> > HashSet;

    HashSet st;
    st.insert("0");
    st.insert("1");
    BOOST_CHECK_EQUAL(st.size(), 2);
    BOOST_CHECK_EQUAL(st.count("0"), 1);
    BOOST_CHECK_EQUAL(st.count("1"), 1);
    for (unsigned i = 2; i < 100; ++i)
        st.insert(boost::lexical_cast<std::string>(i));

    std::ostringstream out;
    mms::write(out, st);
    std::string buf = out.str();
    const HashSet::MmappedType& mm = mms::cast<HashSet::MmappedType>(buf.c_str(), buf.size());

    BOOST_REQUIRE_EQUAL(mm.size(), 100);
    BOOST_REQUIRE(mm.find("12") != mm.end());
    BOOST_CHECK(mm.find("nonexistent") == mm.end());
}

BOOST_AUTO_TEST_CASE(test_hash_map_copy)
{
    typedef mms::unordered_map<mms::Standalone, mms::string<mms::Standalone>, int> HashMap;

    HashMap st;
    st.insert(std::make_pair("0", 10));
    st.insert(std::make_pair("1", 1));

    std::ostringstream out;
    mms::write(out, st);
    std::string buf = out.str();
    const HashMap::MmappedType& mm = mms::cast<HashMap::MmappedType>(buf.c_str(), buf.size());

    HashMap copied;
    mms::copy(mm, copied);
    BOOST_CHECK(copied == st);
}

BOOST_AUTO_TEST_CASE(test_hash_set_copy)
{
    typedef mms::unordered_set<mms::Standalone, mms::string<mms::Standalone> > HashSet;

    HashSet st;
    st.insert("0");
    st.insert("1");

    std::ostringstream out;
    mms::write(out, st);
    std::string buf = out.str();
    const HashSet::MmappedType& mm = mms::cast<HashSet::MmappedType>(buf.c_str(), buf.size());

    HashSet copied;
    mms::copy(mm, copied);
    BOOST_CHECK(copied == st);
}

#endif // MMS_FEATURES_HASH
