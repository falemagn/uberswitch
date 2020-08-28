#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

#include <initializer_list>
#include <tuple>
#include <functional>

namespace uberswitch {

    namespace detail {
    
        template <typename T>
        struct is_initializer_list: std::false_type {};
        
        template <typename T>
        struct is_initializer_list<std::initializer_list<T>>: std::true_type {};
    
    }

    template <typename T = void>
    struct equal_to: std::equal_to<T>{};
    
    template <typename T, std::enable_if_t<!detail::is_initializer_list<T>::value>* = nullptr>
    constexpr bool match(const T &switchVal, const T &caseVal) {        
        return equal_to<T>()(switchVal, caseVal);
    }

    template <typename T, typename U>
    constexpr bool match(const T &switchVal, const U &caseVal) {
        return equal_to<>()(switchVal, caseVal);
    }
    
    template <typename T, typename U>
    constexpr bool match(const std::initializer_list<T> &switchVal, const std::initializer_list<U> &caseVal) {
        return std::equal(std::begin(switchVal), std::end(switchVal), std::begin(caseVal), std::end(caseVal));
    }
    
    template <typename T, template <typename...> typename Tuple, typename... Us, std::size_t... Is>
    constexpr bool match(const std::initializer_list<T> &switchVal, const Tuple<Us...> &caseVal, std::index_sequence<Is...>) {
        return sizeof...(Us) == switchVal.size() && ((switchVal.begin()[Is] == std::get<Is>(caseVal)) && ...);        
    }
    
    template <typename T, typename... Us>
    constexpr bool match(const std::initializer_list<T> &switchVal, const Us &... caseVal) {
        auto s = switchVal.begin();
        return sizeof...(Us) == switchVal.size() && ((*(s++) == caseVal) && ...);        
    }
    
    template <typename T, template <typename...> typename Tuple, typename... Us>
    constexpr bool match(const std::initializer_list<T> &switchVal, const Tuple<Us...> &caseVal) {
        return match(switchVal, caseVal, std::index_sequence_for<Us...>());        
    }
    
    inline constexpr struct any final {
        template <typename T>
        friend constexpr bool operator==(const any &, const T &) { return true; }
        
        template <typename T>
        friend constexpr bool operator==(const T &, const any &) { return true; }
    } any;    
    
}

#define uberswitch_GCC_VERSION_ (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#define uberswitch_cat_(x,y) uberswitch_cat_2(x,y)
#define uberswitch_cat_2(x,y) x##y

#define uberswitch_label_(name,tag) uberswitch_cat_(uberswitch_cat_(uberswitch_cat_(uberswitch_label_,name),_),tag)
#define uberswitch_match_ uberswitch_label_(match,__LINE__)

#if uberswitch_GCC_VERSION_ >= 70000 && !defined(Q_CREATOR_RUN)
    // This apparently unexplicable wrapping is due to a GCC bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85153
#   define uberswitch_PragmaWrap_(x) x
#   define uberswitch_BeginIgnoreWarning_ uberswitch_PragmaWrap_(_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wswitch-unreachable\""))
#   define uberswitch_EndIgnoreWarning_ uberswitch_PragmaWrap_(_Pragma("GCC diagnostic pop"))
#   define uberswitch_goto_(x) uberswitch_BeginIgnoreWarning_ goto x; uberswitch_EndIgnoreWarning_
#else
#   define uberswitch_goto_(x) goto x;
#endif

#define uberswitch(...)                                           \
    if (const auto &uberswitch_val_ = __VA_ARGS__; false); else   \
    if (std::size_t uberswitch_state_ = __COUNTER__; false); else \
    if (bool uberswitch_matched_ = false; false); else            \
    while (!uberswitch_matched_)                                  \
    switch(++uberswitch_state_)                                   \
/***/

#define case(...)                                                                                   \
                      uberswitch_goto_(uberswitch_match_)                                           \
    case __COUNTER__: if (!(uberswitch_matched_ = uberswitch::match(uberswitch_val_, __VA_ARGS__))) \
                          break;                                                                    \
    uberswitch_match_                                                                               \
/***/
#endif //!UBERSWITCH_HPP_
