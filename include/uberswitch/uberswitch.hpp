#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

#include <tuple>

namespace uberswitch {

    namespace detail {
    
        template <typename T>
        using copy_or_ref_t = typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type;
    
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
    constexpr bool match(const std::tuple<T, Us...> &value, const typename std::decay<T>::type &t, const typename std::decay<Us>::type &... us) {
        return value == std::forward_as_tuple(t, us...);
    }
    
    constexpr struct any final {
        template <typename T>
        friend constexpr bool operator==(const any &, const T &) { return true; }
        
        template <typename T>
        friend constexpr bool operator==(const T &, const any &) { return true; }
        
        friend constexpr bool operator==(const any &, const any &) { return true; }
    } any;

}

static constexpr bool uberswitch_next_nesting_level_ = 0;

#if defined (UBERSWITCH_ALLOW_NESTING) && UBERSWITCH_ALLOW_NESTING
#   include "fameta/counter.hpp"
#   define uberswitch_counter_type_ struct uberswitch_counter_type_: fameta::counter<__COUNTER__, 0, 1, uberswitch_counter_type_> {             \
        static_assert(uberswitch_next_nesting_level_ > 0 && uberswitch_nesting_level_ >= 0, "Eeek! Summon the maintainer, things went havoc!"); \                                                                                                                                                \
        std::size_t idx = 0;                                                                                                                    \
    }                                                                                                                                           \
    /***/
#   define uberswitch_counter_next_() uberswitch_counter_.next<__COUNTER__>()
#else
#   define uberswitch_counter_type_ struct {                                                                                                  \
        static_assert(uberswitch_next_nesting_level_ > 0, "Eeek! Summon the maintainer, things went havoc!");                                 \
        static_assert(uberswitch_nesting_level_ == 0, "#define UBERSWITCH_ALLOW_NESTING to 1 if you want to nest uberswitch constructs"); \
        enum : std::size_t { start = __COUNTER__+1 };                                                                                         \
        std::size_t idx = 0;                                                                                                                  \
    }                                                                                                                                         \
    /***/
#   define uberswitch_counter_next_() __COUNTER__ - uberswitch_counter_.start
#endif

#define uberswitch(...)                                                                                            \
    for (bool uberswitch_var_init_ = true; uberswitch_var_init_; )                                                 \
        for (constexpr bool uberswitch_nesting_level_ = uberswitch_next_nesting_level_;  uberswitch_var_init_;)    \
        for (constexpr bool uberswitch_next_nesting_level_ = uberswitch_nesting_level_+1;  uberswitch_var_init_;)  \
        for (uberswitch_counter_type_ uberswitch_counter_; uberswitch_var_init_;)                                  \
        for (bool uberswitch_matched_ = false; uberswitch_var_init_;)                                              \
        for (auto uberswitch_value_ = uberswitch::value(__VA_ARGS__); uberswitch_var_init_;)                       \
    for (;uberswitch_var_init_; uberswitch_var_init_ = false)                                                      \
    for (bool uberswitch_trying_match_ = false; (uberswitch_trying_match_ ^= true);)                               \
    switch (uberswitch_counter_.idx)                                                                               \
/***/
    
#define case(...)                                                                    \
    case uberswitch_counter_next_():                                                 \
        if (!uberswitch_matched_) {                                                  \
            uberswitch_matched_ = uberswitch::match(uberswitch_value_, __VA_ARGS__); \
                                                                                     \
                if (!uberswitch_matched_) {                                          \
                    uberswitch_trying_match_ = false;                                \
                    uberswitch_counter_.idx += 2;                                    \
                    break;                                                           \
                }                                                                    \
            }                                                                        \
    case uberswitch_counter_next_()                                                  \
/***/
    
#endif //!UBERSWITCH_HPP_

