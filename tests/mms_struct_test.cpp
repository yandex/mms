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
#include <mms/vector.h>
#include <mms/map.h>
#include <mms/set.h>
#include <mms/writer.h>
#include <mms/copy.h>
#include <iostream>
#include <sstream>
#include <functional>
#include <boost/lexical_cast.hpp>

#include <boost/assign/std/vector.hpp>
#include "tools.h"

template <class T1, class T2, class T3>
struct TraverseSimple {

    T1 v1;
    T2 v2;
    T3 v3;

    template<class A> void traverseFields(A a) const { a(v1)(v2)(v3); }
    typedef TraverseSimple<T1, T2, T3> MmappedType;
};

template <class T1, class T2, class T3>
bool operator == (const TraverseSimple<T1, T2, T3>& lhs,
                  const TraverseSimple<T1, T2, T3>& rhs)
{
    return lhs.v1 == rhs.v1 && lhs.v2 == rhs.v2 && lhs.v3 == rhs.v3;
}

template <class T1, class T2, class T3>
TraverseSimple<T1, T2, T3> genTraverseSimple(int seed)
{
    TraverseSimple<T1, T2, T3> pod;
    pod.v1 = GenT<T1>()(seed);
    pod.v2 = GenT<T2>()(seed * 57);
    pod.v3 = GenT<T3>()(seed * 1543);
    return pod;
}

template <class T1, class T2, class T3>
void testTraverseSimple(int seed)
{
    typedef TraverseSimple<T1, T2, T3> SIMPLE;
    SIMPLE podOrig = genTraverseSimple<T1, T2, T3>(seed);
    std::stringstream out;
    mms::Writer w(out);
    size_t ofs = mms::unsafeWrite(w, podOrig);
    BOOST_CHECK_EQUAL(w.pos(), sizeof(SIMPLE));
    BOOST_CHECK_EQUAL(ofs, 0);

    std::string buf = out.str();
    SIMPLE podReaded = *((const SIMPLE*) (buf.c_str() + ofs));
    BOOST_CHECK(podOrig == podReaded);
    
    SIMPLE copied;
    mms::copy(podReaded, copied);
    BOOST_CHECK(podOrig == copied);
}

BOOST_AUTO_TEST_CASE( pod_test )
{
    for(int seed = -1300; seed <= 157092; seed += 10212) {
        testTraverseSimple<int, int, int>(seed);
        testTraverseSimple<char, int, int>(seed);
        testTraverseSimple<int, char, int>(seed);
        testTraverseSimple<int, int, char>(seed);
        testTraverseSimple<int, size_t, int>(seed);
        testTraverseSimple<char, size_t, int>(seed);
        testTraverseSimple<int, size_t, char>(seed);
        testTraverseSimple<char, char, int>(seed);
        testTraverseSimple<char, char, char>(seed);
        testTraverseSimple<uint32_t, uint16_t, uint32_t>(seed);
        testTraverseSimple<uint32_t, char, uint32_t>(seed);
        testTraverseSimple<uint16_t, char, uint16_t>(seed);
    }
}

