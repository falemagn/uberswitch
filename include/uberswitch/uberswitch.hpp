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

    struct number_of_arguments_is_not_right
    {
        template <std::size_t R>
        struct required
        {
            template <std::size_t P>
            struct provided
            {
                enum : bool { too_many_arguments_check = !(R < P) };
                enum : bool { too_few_arguments_check  = !(R > P) };
                
                static_assert(too_many_arguments_check, "Too many arguments given to case(...): they must be as many as the number of items of the switch() value");
                static_assert(too_few_arguments_check, "Too few arguments given to case(...): they must be as many as the number of items of the switch() value");
            };
        };        
    };
    
    template <std::size_t N>
    struct types_not_comparable_for_argument_number
    {
        template <typename R>
        struct in_switch
        {
            template <typename P>
            struct in_case
            {
                enum : bool { arguments_are_comparable_check = N && !N };
                
                static_assert(arguments_are_comparable_check, "operator== not defined for an argument in switch(...) and case(...)");
            };
        };        
    };
    
    namespace detail {
    
        template <typename Tuple, std::size_t idx, std::size_t size>
        struct matcher
        {
            template <typename T, typename = bool>
            struct can_match: std::false_type {};
            
            template <typename T>
            struct can_match<T, decltype(std::get<idx>(std::declval<Tuple>()) == std::declval<T>())>: std::true_type {};
            
            // So far, so good
            template <typename First, typename... Rest, typename std::enable_if<can_match<First>::value>::type* = nullptr>
            constexpr static auto match(bool good_so_far, const Tuple &tuple, First &&first, Rest &&... rest)
            {
                return matcher<Tuple, idx+1, size>::match(good_so_far && (std::get<idx>(tuple) == std::forward<First>(first)), tuple, std::forward<Rest>(rest)...);
            }
            
            // Type mismatch
            template <typename First, typename... Rest, typename std::enable_if<!can_match<First>::value>::type* = nullptr>
            constexpr static auto match(bool, const Tuple &, First &&, Rest &&...)
                -> typename types_not_comparable_for_argument_number<idx+1>::template in_switch<typename std::tuple_element<idx, Tuple>::type>::template in_case<First>
            {
                return {};
            }
            
            // Too few arguments
            constexpr static auto match(bool, const Tuple &)
                -> typename number_of_arguments_is_not_right::template required<size>::template provided<idx>
            {
                return {};
            }
        };
        
        template <typename Tuple, std::size_t idx>
        struct matcher<Tuple, idx, idx>
        {
            // Too many arguments
            template <typename... ExtraArgs>
            static constexpr auto match(bool, const Tuple &, ExtraArgs &&...)
                -> typename number_of_arguments_is_not_right::template required<idx>::template provided<idx+sizeof...(ExtraArgs)>                       
            {
                return {};    
            }
            
            // Success                
            constexpr static bool match(bool good_so_far, const Tuple &)
            {
                return good_so_far;
            }
        };
        
        template <typename T>
        using copy_or_ref_t = typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type;
        
    }

    template <typename T>
    constexpr detail::copy_or_ref_t<T> switch_value(T && t)
    {
        return std::forward<T>(t);
    }
    
    template <typename T, typename U, typename... Xs>
    constexpr std::tuple<detail::copy_or_ref_t<T>, detail::copy_or_ref_t<U>, detail::copy_or_ref_t<Xs>...> switch_value(T && t, U && u, Xs &&... xs)
    {
        return {std::forward<T>(t), std::forward<U>(u), std::forward<Xs>(xs)...};
    }
    
    template <typename T>
    constexpr bool match(const T &switch_value, const T &case_value)
    {
        return switch_value == case_value;
    }
    
    template <typename T, typename X>
    constexpr auto match(const T &switch_value, X && case_value) -> decltype(switch_value == std::forward<X>(case_value))
    {
        return switch_value == std::forward<X>(case_value);
    }
    
    template <typename Tuple, typename T, typename U, typename... Xs, typename Matcher = typename detail::matcher<Tuple, 0, std::tuple_size<Tuple>::value>>
    constexpr auto match(const Tuple &switch_value, T && t, U && u, Xs &&... xs)
    {
        return Matcher::match(true, switch_value, std::forward<T>(t), std::forward<U>(u), std::forward<Xs>(xs)...);
    }
            
    constexpr struct any final
    {
        template <typename T>
        friend constexpr bool operator==(const any &, const T &) { return true; }
        
        template <typename T>
        friend constexpr bool operator==(const T &, const any &) { return true; }
        
        friend constexpr bool operator==(const any &, const any &) { return true; }
    } any;
    
    static_assert (any == any, "No warnings, please");
}

constexpr std::size_t uberswitch_next_nesting_level_ = 0;

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
#define uberswitch(...)                                                                                                  \
    for (bool uberswitch_var_init_ = true; uberswitch_var_init_; )                                                       \
        for (constexpr std::size_t uberswitch_nesting_level_ = uberswitch_next_nesting_level_;  uberswitch_var_init_;)   \
        for (constexpr std::size_t uberswitch_next_nesting_level_ = uberswitch_nesting_level_+1;  uberswitch_var_init_;) \
        for (uberswitch_counter_type_ uberswitch_counter_; uberswitch_var_init_;)                                        \
        for (bool uberswitch_matched_ = false; uberswitch_var_init_;)                                                    \
        for (auto uberswitch_value_ = uberswitch::switch_value(__VA_ARGS__); uberswitch_var_init_;)                      \
    for (;uberswitch_var_init_; uberswitch_var_init_ = false)                                                            \
    for (bool uberswitch_trying_match_ = false; (uberswitch_trying_match_ ^= true);)                                     \
    switch (uberswitch_counter_.idx)                                                                                     \
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
#define uberswitch_c(id, ...)                                                                             \
    if (constexpr std::size_t uberswitch_nesting_level_ = uberswitch_next_nesting_level_;  false); else   \
    if (constexpr std::size_t uberswitch_next_nesting_level_ = uberswitch_nesting_level_+1; false ); else \
    if (uberswitch_counter_type_ uberswitch_counter_; false); else                                        \
    if (bool uberswitch_matched_ = false; false); else                                                    \
    if (auto uberswitch_value_ = uberswitch::switch_value(__VA_ARGS__); false); else                      \
    uberswitch_label_##id##_:                                                                             \
    switch (uberswitch_counter_.idx)                                                                      \
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

