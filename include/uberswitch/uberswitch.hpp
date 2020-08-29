#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

// Configuration macros to define before including uberswitch.hpp

/// Define to 1 to allow nesting uberswitch statements. In that case, fameta::counter will be included and used. 
#if !defined(UBERSWITCH_ALLOW_NESTING)
#    define UBERSWITCH_ALLOW_NESTING 0
#endif

/// Define to 0 to get the disable the definition of the case() macro and get only the ubercase() one, 
/// in those very rare situations in which case() might conflict with existing code.
#if !defined(UBERSWITCH_CASE_SHORTNAME)
#    define UBERSWITCH_CASE_SHORTNAME 1
#endif

#include <tuple>

namespace uberswitch {

    namespace detail {
    
        template <typename T>
        using copy_or_ref_t = typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type;
    
    }

    template <typename T, typename... Us>
    constexpr std::tuple<detail::copy_or_ref_t<T>, detail::copy_or_ref_t<Us>...> value(T && t, Us &&... us) {
        return {std::forward<T>(t), std::forward<Us>(us)...};
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

constexpr bool uberswitch_next_nesting_level_ = 0;

#if UBERSWITCH_ALLOW_NESTING
#   include "fameta/counter.hpp"
#   define uberswitch_counter_type_ struct uberswitch_counter_type_: fameta::counter<__COUNTER__, 0, 1, uberswitch_counter_type_> {             \
        static_assert(uberswitch_next_nesting_level_ > 0 && uberswitch_nesting_level_ >= 0, "Eeek! Summon the maintainer, things went havoc!"); \
        std::size_t idx = 0;                                                                                                                    \
    }                                                                                                                                           \
    /***/
#   define uberswitch_counter_next_() uberswitch_counter_.next<__COUNTER__>()
#else
#   define uberswitch_counter_type_ struct {                                                                                              \
        static_assert(uberswitch_next_nesting_level_ > 0, "Eeek! Summon the maintainer, things went havoc!");                             \
        static_assert(uberswitch_nesting_level_ == 0, "#define UBERSWITCH_ALLOW_NESTING to 1 if you want to nest uberswitch constructs"); \
        enum : std::size_t { start = __COUNTER__+1 };                                                                                     \
        std::size_t idx = 0;                                                                                                              \
    }                                                                                                                                     \
    /***/
#   define uberswitch_counter_next_() __COUNTER__ - uberswitch_counter_.start
#endif

// This version is constexpr-safe and noise-free, but breaks the 'continue' keyword, in a away that makes it work just like the 'break' keyword.
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
    
#define ubercase(...)                                                                \
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


// This version cannot be used in constexpr functions, can only be used from c++17 onwards and requires an additional first 'context' parameter,
// which can be any identifier or even just a number, but it doesn't  break the 'continue' keyword.
#define uberswitch_c(id, ...)                                                                       \
    if (constexpr bool uberswitch_nesting_level_ = uberswitch_next_nesting_level_; false); else     \
    if (constexpr bool uberswitch_next_nesting_level_ = uberswitch_nesting_level_+1;  false); else  \
    if (uberswitch_counter_type_ uberswitch_counter_; false); else                                  \
    if (bool uberswitch_matched_ = false; false); else                                              \
    if (auto uberswitch_value_ = uberswitch::value(__VA_ARGS__); false); else                       \
    uberswitch_label_##id##_:                                                                       \
    switch (uberswitch_counter_.idx)                                                                \
/***/
    
#define ubercase_c(id, ...)                                                          \
    case uberswitch_counter_next_():                                                 \
        if (!uberswitch_matched_) {                                                  \
            uberswitch_matched_ = uberswitch::match(uberswitch_value_, __VA_ARGS__); \
                                                                                     \
                if (!uberswitch_matched_) {                                          \
                    uberswitch_counter_.idx += 2;                                    \
                    goto uberswitch_label_##id##_;                                   \
                }                                                                    \
            }                                                                        \
    case uberswitch_counter_next_()                                                  \
/***/

#if UBERSWITCH_CASE_SHORTNAME
#   if defined(__clang__)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wkeyword-macro"
#   endif
#   define case(...) ubercase(__VA_ARGS__)
#   define case_c(...) ubercase_c(__VA_ARGS__)
#   if defined(__clang__)
#       pragma GCC diagnostic pop
#   endif
#endif
    
#endif //!UBERSWITCH_HPP_

