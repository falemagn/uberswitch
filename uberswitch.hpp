#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

#include <tuple>

namespace uberswitch {

    namespace detail {
    
        template <typename T>
        using copy_or_ref_t = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;
    
    }

    template <typename T, typename... Us>
    constexpr std::tuple<detail::copy_or_ref_t<T>, detail::copy_or_ref_t<Us>...> value(T && t, Us &&... us) {
        return {t, us...};
    }

    template <typename T, typename... Us, typename... Xs>
    constexpr bool match(const std::tuple<T, Us...> &value, Xs &&... xs) {
        return value == std::forward_as_tuple(std::forward<Xs>(xs)...);
    }
    
    template <typename T, typename... Us>
    constexpr bool match(const std::tuple<T, Us...> &value, const std::decay_t<T> &t, const std::decay_t<Us> &... us) {
        return value == std::forward_as_tuple(t, us...);
    }
    
    inline constexpr struct any final {
        template <typename T>
        friend constexpr bool operator==(const any &, const T &) { return true; }
        
        template <typename T>
        friend constexpr bool operator==(const T &, const any &) { return true; }
    } any;

}

#define uberswitch(...)                                                              \
    if (bool uberswitch_matched_ = false; false); else                               \
    if (std::size_t uberswitch_idx_ = __COUNTER__+1; false); else                    \
    if (auto uberswitch_value_ = uberswitch::value(__VA_ARGS__); false); else        \
    for (bool uberswitch_trying_match_ = false; (uberswitch_trying_match_ ^= true);) \
    switch (uberswitch_idx_)                                                         \
/***/
    
#define case(...)                                                                                  \
    case __COUNTER__: if (!uberswitch_matched_) {                                                  \
                          uberswitch_matched_ = uberswitch::match(uberswitch_value_, __VA_ARGS__); \
                                                                                                   \
                          if (!uberswitch_matched_) {                                              \
                              uberswitch_trying_match_ = false;                                    \
                              uberswitch_idx_ += 2;                                                \
                              break;                                                               \
                          }                                                                        \
                      }                                                                            \
    case __COUNTER__                                                                               \
/***/
    
#endif //!UBERSWITCH_HPP_
