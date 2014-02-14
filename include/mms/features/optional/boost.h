#pragma once

#ifdef MMS_FEATURES_OPTIONAL
#    error another optional implementation has already been used
#else
#    define MMS_FEATURES_OPTIONAL BOOST
#endif

#include <boost/optional.hpp>

namespace mms {
namespace impl {

template<class T> struct optional_base { typedef boost::optional<T> type; };

} // namespace impl
} // namespace mms
