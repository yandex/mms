#pragma once

#if __cplusplus >= 201103L

#    ifndef MMS_FEATURES_TYPE_TRAITS
#        include "../features/type_traits/c++11.h"
#    endif

#    ifndef MMS_FEATURES_HASH
#        include "../features/hash/c++11.h"
#    endif

#    ifndef MMS_USE_CXX11
#        include "../features/c++11.h"
#    endif

#endif


#ifdef BOOST_CONFIG_HPP

#    ifndef MMS_FEATURES_TYPE_TRAITS
#        include "../features/type_traits/boost.h"
#    endif

#    ifndef MMS_FEATURES_HASH
#        include "../features/hash/boost.h"
#    endif

#    ifndef MMS_FEATURES_OPTIONAL
#        include "../features/optional/boost.h"
#    endif

#endif


#ifndef MMS_FEATURES_TYPE_TRAITS
#   include "../features/type_traits/intrinsics.h"
#endif
