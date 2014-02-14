// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#include <functional>
#include <cstdlib>

namespace mms {

class Writer;
template<class P, class K, class V, template<class> class Cmp = std::less> class map;
template<class P, class T, template<class> class Cmp = std::less> class set;
template<class P, class T> class vector;
template<class P, class T> class optional;
template<class P> class string;
typedef size_t FormatVersion;

}//namespace mms
