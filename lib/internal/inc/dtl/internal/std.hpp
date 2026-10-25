
#ifndef DOOC_TEMPLATE_LIBRARY_DTL_INTERNAL_STD_HPP
#define DOOC_TEMPLATE_LIBRARY_DTL_INTERNAL_STD_HPP

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

#if __has_include(<inplace_vector>)
#include <inplace_vector>
#endif
#if defined(__cpp_lib_inplace_vector) && ____cpp_lib_inplace_vector >= 202603L
namespace dtl::etd {
using std::inplace_vector;
}
#else
#include "dtl/internal/std/inplace_vector.hpp"
namespace dtl::etd {
using dtl::internal::inplace_vector;
}
#endif

#endif
