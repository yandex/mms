
/*
 * impl/front_remove_iterator.h
 *
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

#include <iterator>

namespace mms {

// An opposite class to std::back_insert_iterator.
template<class Container>
class front_remove_iterator: public std::iterator<std::input_iterator_tag, typename Container::value_type> {
public:
    front_remove_iterator(): container_() {}
    explicit front_remove_iterator(Container& c): container_(&c) {}

    front_remove_iterator<Container>& operator++()
    {
        container_->pop_front();
        return *this;
    }
    
    typename Container::value_type& operator*() const { return container_->front(); }
    typename Container::value_type* operator->() const { return &**this; }

    bool operator == (const front_remove_iterator<Container>& other) const { return at_end() && other.at_end(); }
    bool operator != (const front_remove_iterator<Container>& other) const { return !(*this == other); }

private:
    Container* container_;

    bool at_end() const { return !container_ || container_->empty(); }

    void operator++(int); // not defined
};

}//namespace mms
