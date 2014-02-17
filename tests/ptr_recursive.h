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


#pragma once

#include <mms/ptr.h>

template<class P>
struct OuterPtr;

template<class P>
struct InnerPtr: public mms::Pointee {
    int x;
    mms::ptr< P, OuterPtr<P> > outer;
    InnerPtr(int xx, OuterPtr<P>* p): x(xx), outer(p) {}

    template<class A> void traverseFields(A a) const { a(x)(outer); }
};

template<class P>
struct OuterPtr: public mms::Pointee {

    mms::vector< P, mms::shared_ptr< P, InnerPtr<P> > > inners;
        // unique_ptr<> would be better here, but not on Hardy :(

    explicit OuterPtr(size_t innerCount)
    {
        for (size_t i = 0; i != innerCount; ++i)
            inners.push_back(new InnerPtr<P>(i, this));
    }

    template<class A> void traverseFields(A a) const { a(inners); }
};
