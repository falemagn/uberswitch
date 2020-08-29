#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

#include <initializer_list>
#include <tuple>
#include <functional>

namespace uberswitch {

    namespace detail {
    
        template <typename T>
        using copy_or_ref_t = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;
    
    }

    template <typename T, typename... Us>
    struct state
    {
        constexpr state(std::size_t initial_switch, T t, Us ... u)
            : value_{t, u...}
            , switch_{initial_switch}
        {}
        
        template <typename... Xs>
        constexpr bool match(Xs &&... xs) {
            return matched_ = (value_ == std::forward_as_tuple(std::forward<Xs>(xs)...));
        }
        
        constexpr explicit operator bool() const {
            return !matched_;    
            }
        
        std::size_t next() {
            return ++switch_;
        }
        
    private:
        std::tuple<T, Us...> value_;
        std::size_t switch_;
        bool matched_{};
    };
    
    template <typename T, typename... Us>
    state(std::size_t initial_switch, T && t, Us &&... us) -> state<detail::copy_or_ref_t<T>, detail::copy_or_ref_t<Us>...>;
    
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

#define uberswitch(...)                                                                     \
    for (uberswitch::state uberswitch_state_(__COUNTER__, __VA_ARGS__); uberswitch_state_;) \
    switch(uberswitch_state_.next())                                                        \
/***/

#define case(...)                                                \
                      uberswitch_goto_(uberswitch_match_)        \
    case __COUNTER__: if (!uberswitch_state_.match(__VA_ARGS__)) \
                          break;                                 \
    uberswitch_match_                                            \
/***/
#endif //!UBERSWITCH_HPP_
