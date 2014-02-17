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

#ifdef MMS_FEATURES_OPTIONAL

#include <boost/test/unit_test.hpp>

#include <mms/string.h>
#include <mms/optional.h>
#include <mms/vector.h>
#include <mms/cast.h>
#include <mms/copy.h>

#include <iostream>


template<class P>
struct OptionalStruct {
    mms::optional<P, int> optionalInt;
    mms::optional<P, double> optionalDouble;
    mms::optional<P, mms::string<P> > optionalString;
    mms::optional<P, mms::optional<P, int> > optionalOptionalInt;
    mms::vector<P, mms::optional<P, int> > vectorOptionalInt;

    template <class A> void traverseFields(A a) const
    { a(optionalInt)(optionalDouble)(optionalString)(optionalOptionalInt)(vectorOptionalInt); }

    typedef OptionalStruct<mms::Mmapped> MmappedType;

    bool operator == (const OptionalStruct<P>& rhs) const
    {
        return optionalInt == rhs.optionalInt
            && optionalDouble == rhs.optionalDouble
            && optionalString == rhs.optionalString
            && optionalOptionalInt == rhs.optionalOptionalInt
            && vectorOptionalInt == rhs.vectorOptionalInt;
    }

    bool operator != (const OptionalStruct<P>& rhs) const { return !(*this == rhs); }
};


BOOST_AUTO_TEST_CASE( optional_test )
{
    std::stringstream out;
    mms::Writer w(out);

    OptionalStruct<mms::Standalone> mmsOptional;
    mmsOptional.optionalInt = 35;
    mmsOptional.optionalString = mms::string<mms::Standalone>("abcdefg");
    mmsOptional.optionalOptionalInt = mms::optional<mms::Standalone, int>(13);
    mmsOptional.optionalOptionalInt = mms::optional<mms::Standalone, int>(13);
    mmsOptional.vectorOptionalInt.resize(100);
    for(size_t i = 13; i <= 57; ++i) mmsOptional.vectorOptionalInt[i] = i;

    size_t ofs = mms::unsafeWrite(w, mmsOptional);
    std::string buf = out.str();

    OptionalStruct<mms::Mmapped> mappedOptional =
            mms::unsafeCast<OptionalStruct<mms::Mmapped> >(buf.c_str(), buf.size());

    BOOST_CHECK(ofs % sizeof(void*) == 0);

    BOOST_CHECK(static_cast<bool>(mappedOptional.optionalInt));
    BOOST_CHECK_EQUAL(*mappedOptional.optionalInt, 35);
    BOOST_CHECK_EQUAL(mappedOptional.optionalInt.get_value_or(67), 35);

    BOOST_CHECK(!mappedOptional.optionalDouble.is_initialized());
    BOOST_CHECK(!mappedOptional.optionalDouble);
    BOOST_CHECK(!static_cast<bool>(mappedOptional.optionalDouble));
    BOOST_CHECK_EQUAL(mappedOptional.optionalDouble.get_value_or(1.0), 1.0);

    BOOST_CHECK(mappedOptional.optionalString.is_initialized());
    BOOST_CHECK_EQUAL(mappedOptional.optionalString->length(), 7);
    BOOST_CHECK_EQUAL(mappedOptional.optionalString.get(), "abcdefg");
    BOOST_CHECK_EQUAL(*mappedOptional.optionalString.get_ptr(), "abcdefg");

    BOOST_CHECK(static_cast<bool>(mappedOptional.optionalOptionalInt));
    BOOST_CHECK(*mappedOptional.optionalOptionalInt);
    BOOST_CHECK_EQUAL(*mappedOptional.optionalOptionalInt.get(), 13);
    
    BOOST_CHECK(mappedOptional.optionalInt != *mappedOptional.optionalOptionalInt);
    BOOST_CHECK(!(mappedOptional.optionalInt == *mappedOptional.optionalOptionalInt));
    
    OptionalStruct<mms::Standalone> copied;
    mms::copy(mappedOptional, copied);
    BOOST_CHECK(copied == mmsOptional);
}

#endif // MMS_FEATURES_OPTIONAL
