
#ifndef DTL_DTL_LPOW_HPP
#define DTL_DTL_LPOW_HPP

#include <concepts>
#include <type_traits>

namespace dtl {

template <typename T>
concept strict_multipliable = requires(T& tr, T const& tc)
{
  {tc * tc} -> std::convertible_to<T>;
  {tr *= tc} -> std::convertible_to<T&>;
};


template <strict_multipliable B, std::integral E>
constexpr B lpow(B const& base, E exp) {
  if constexpr (std::is_signed_v<E>) {
    if (exp < E{}) {
      return B{1} / lpow(base, static_cast<std::make_unsigned_t<E>>(-exp));
    } else {
      return lpow(base, static_cast<std::make_unsigned_t<E>>(exp));
    }
  } else {
    auto mult = base;
    auto res = B{1};
    while (exp > E{0}) {
      if (exp & 1) { res *= mult;}
      mult *= mult;
      exp >>= 1;
    }
    return res;
  }
}

}

#endif
