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

#ifdef MMS_USE_CXX11

#include "tools.h"

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>

#include <iostream>
#include <typeinfo>

template <typename C>
struct Generator;

template <typename S>
struct Generator<std::vector<S>> {
    static std::vector<S> generate(int seed, size_t size) {
        return genVector<S>(seed, size);
    }
};

template <typename S>
struct Generator<std::set<S>> {
    static std::set<S> generate(int seed, size_t size) {
        return genSet<S>(seed, size);
    }
};

template <typename K, typename V>
struct Generator<std::map<K, V>> {
    static std::map<K, V> generate(int seed, size_t size) {
        return genMap<K, V>(seed, size);
    }
};

template <>
struct Generator<std::string> {
    static std::string generate(int seed, size_t size) {
        std::vector<char> v = Generator< std::vector<char> >::generate(seed, size);
        return std::string(v.begin(), v.end());
    }
};

typedef boost::mpl::list<
    std::pair<mms::string<mms::Standalone>, std::string>,
    std::pair<mms::vector<mms::Standalone, int>, std::vector<int>>,
    std::pair<mms::set<mms::Standalone, int>, std::set<int>>,
    std::pair<mms::map<mms::Standalone, int, int>, std::map<int, int>>
> TestedTypes;

BOOST_AUTO_TEST_CASE_TEMPLATE(move_cr_from_std_container, Containers, TestedTypes) {
    typedef typename Containers::first_type MMSContainer;
    typedef typename Containers::second_type STDContainer;
    STDContainer stdc = Generator<STDContainer>::generate(42, 1000);
    STDContainer stdcTmp = stdc;
    MMSContainer mmsc(std::move(stdcTmp));
    BOOST_CHECK(stdc == mmsc);
    BOOST_CHECK_EQUAL(stdcTmp.size(), 0);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(move_cr_from_mms_container, Containers, TestedTypes) {
    typedef typename Containers::first_type MMSContainer;
    typedef typename Containers::second_type STDContainer;
    STDContainer stdc = Generator<STDContainer>::generate(666, 1000);
    MMSContainer mmscTmp(stdc);
    MMSContainer mmsc(std::move(mmscTmp));
    BOOST_CHECK(stdc == mmsc);
    BOOST_CHECK_EQUAL(mmscTmp.size(), 0);
}

#endif
