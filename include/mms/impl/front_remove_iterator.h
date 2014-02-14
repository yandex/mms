// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

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
