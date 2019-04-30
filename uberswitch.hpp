#ifndef UBERSWITCH_HPP_
#define UBERSWITCH_HPP_

#include <map>

#if __cplusplus >= 201103L
#   include <unordered_map>
#endif

namespace uberswitch {

    template <typename T>
    struct hash;

    template <typename T = void>
    struct equal_to: std::equal_to<T>{};

    template <typename T>
    struct less: std::less<T>{};

    #if __cplusplus <= 201103L
        template <>
        struct equal_to<void> {
            template <typename T1, typename T2>
            bool operator ()(const T1 &lhs, const T2 &rhs) const {
                return lhs == rhs;
            }
        };
    #else
        template <typename T>
        struct hash: std::hash<T>{};
    #endif

    namespace impl {
        template <typename T, bool HasHash = false>
        struct SwitchMap {
            typedef std::map< T, int, less<T> > type;
        };

        #if __cplusplus >= 201103L
            // From: https://stackoverflow.com/a/37161919/566849
            template<class F, class...Args>
            struct is_callable
            {
                template<class U> static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
                template<class U> static auto test(...) -> decltype(std::false_type());

                static constexpr bool value = decltype(test<F>(0))::value;
            };

            template <typename T>
            struct has_hash {
                enum { value = is_callable<hash<T>, T>::value };
            };

            template <typename T>
            struct SwitchMap<T, true> {
                typedef std::unordered_map<T, int, hash<T>, equal_to<T>> type;
            };
        #else
            template <typename T>
            struct has_hash {
                enum { value = false };
            };
        #endif
    }

    template <typename T>
    struct Traits {
        typedef typename impl::SwitchMap<T, impl::has_hash<T>::value>::type map_type;
    };

    template <typename T>
    class Switch {
    public:
        Switch(int initialState):
            m_state(initialState)
        {}

        int matchedState(const T &) const {
            return m_state;
        }

        bool match(const T &switchVal, const T &caseVal, int &state) {
            m_state = state += 2;

            return m_equalTo(switchVal, caseVal);
        }

        template <typename T2>
        bool match(const T &switchVal, const T2 &caseVal, int &state) {
            m_state = state += 2;

            return m_equalTo(switchVal, caseVal);
        }

    protected:
        int m_state;
        equal_to<T> m_equalTo;
    };

    template <typename T>
    class MappedSwitch: Switch<T> {
        typedef typename Traits<T>::map_type Map;
    public:
        MappedSwitch(int initialState):
            Switch<T>(initialState),
            m_map()
        {}

        int matchedState(const T &switchVal) const {
            typename Map::const_iterator it = m_map.find(switchVal);
            if (it != m_map.end())
                return it->second;

            return Switch<T>::matchedState(switchVal);
        }

        bool match(const T &switchVal, const T &caseVal, int &state) {
            #if __cplusplus >= 201103L
                m_map.emplace(caseVal, state+1);
            #else
                m_map.insert(typename std::map<T, int>::value_type(caseVal, state));
            #endif

            return Switch<T>::match(switchVal, caseVal, state);
        }

    private:
        Map m_map;
    };
}

#define uberswitch_GCC_VERSION_ (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#define uberswitch_cat_(x,y) uberswitch_cat_2(x,y)
#define uberswitch_cat_2(x,y) x##y

#ifndef uberswitch_context
#    define uberswitch_context()
#endif

#define uberswitch_label_(name,tag) uberswitch_cat_(uberswitch_cat_(uberswitch_cat_(uberswitch_label_,name),_),tag)
#define uberswitch_match_ uberswitch_label_(match,__LINE__)
#define uberswitch_begin_ uberswitch_label_(begin,uberswitch_context())

#if __cplusplus >= 201406L
#   define uberswitch_var_begin_
#   define uberswitch_var_(...) if (__VA_ARGS__;false); else
#   define uberswitch_var_end_
#   define uberswitch_switch_(x) uberswitch_begin_: switch (x)
#   define uberswitch_switch_redo_() goto uberswitch_begin_
#else
#   define uberswitch_var_begin_ for (bool uberswitch_var_continue_ = true; uberswitch_var_continue_;)
#   define uberswitch_var_(...) for (__VA_ARGS__; uberswitch_var_continue_;)
#   define uberswitch_var_end_ for (; uberswitch_var_continue_; uberswitch_var_continue_ = false)
#   define uberswitch_switch_(x) for (bool uberswitch_switch_continue_ = true; uberswitch_switch_continue_;  uberswitch_switch_continue_ = !uberswitch_switch_continue_) switch (x)
#   define uberswitch_switch_redo_() { uberswitch_switch_continue_ = false; continue; }
#endif

#if __cplusplus >= 201103L
#   define uberswitch_typeof_(x) std::remove_reference<std::remove_cv<decltype(x)>::type>::type
#   define uberswitch_decay_(x) std::decay<decltype(x)>::type
#   define uberswitch_thread_local_(T, v, ...) static thread_local T v(__VA_ARGS__)
#else
    #include <pthread.h>

    namespace uberswitch {
        namespace oldcxxhacks {
            template <typename T>
            struct fakeremovecv { typedef T type; };
            template <typename T>
            struct fakeremovecv<const T> { typedef T type; };
            template <typename T>
            struct fakeremovecv<volatile T> { typedef T type; };
            template <typename T>
            struct fakeremovecv<volatile const T> { typedef T type; };

            template <typename T>
            struct fakedecay { typedef typename fakeremovecv<T>::type type; };
            template <typename T>
            struct fakedecay<T&> { typedef typename fakeremovecv<T>::type type; };
            template <typename T, size_t N>
            struct fakedecay<T(&)[N]> { typedef typename fakeremovecv<T>::type *type; };

            /***/
            template<std::size_t _Len, std::size_t _Align>
            struct aligned_storage
            {
                union type
                {
                    unsigned char __data[_Len];
                    struct __attribute__((__aligned__((_Align)))) { } __align;
                };
            };

            // Inspired by https://stackoverflow.com/a/12053862/566849
            // Use a template to leverage the One Definition Rule, so that we don't need to put the static data into a cpp file of its own.
            template <typename T>
            struct thread_local_impl_ {
                static pthread_key_t key;
                static pthread_once_t once_control;

                struct destructor_list
                {
                    void (*destructor)(void*);
                    void* param;
                    destructor_list* next;
                };

                static void execute_destructor_list(void* v)
                {
                    for (destructor_list* p = (destructor_list*) v; p != 0; p = p->next)
                        p->destructor(p->param);
                }

                static void create_key()
                {
                    pthread_key_create(&key, execute_destructor_list);
                }

                static void add_destructor(destructor_list* p)
                {
                    pthread_once(&once_control, create_key);

                    p->next = (destructor_list*) pthread_getspecific(key);
                    pthread_setspecific(key, p);
                }

                template<class U> static void placement_delete(void* t) { reinterpret_cast<U*>(t)->~U(); }
            };


            template <typename T>
            pthread_key_t thread_local_impl_<T>::key;
            template <typename T>
            pthread_once_t thread_local_impl_<T>::once_control = PTHREAD_ONCE_INIT;

            typedef thread_local_impl_<void> thread_local_;
        }
    }
#   define uberswitch_typeof_(x) __typeof__(x)
#   define uberswitch_decay_(x) uberswitch::oldcxxhacks::fakedecay<uberswitch_typeof_(x)>::type
#   define uberswitch_thread_local_(T, t, ...)                     \
    T& t = *reinterpret_cast<T*>(({                                \
        using namespace uberswitch::oldcxxhacks;                   \
        typedef aligned_storage<sizeof(T),                         \
            __alignof__(T)>::type Storage;                         \
        static __thread bool allocated = false;                    \
        static __thread Storage storage;                           \
        static __thread thread_local_::destructor_list dlist;      \
                                                                   \
        if (!allocated)                                            \
        {                                                          \
            new (&storage) T(__VA_ARGS__);                         \
            allocated = true;                                      \
            dlist.destructor = thread_local_::placement_delete<T>; \
            dlist.param = &storage;                                \
            thread_local_::add_destructor(&dlist);                 \
        }                                                          \
                                                                   \
        &storage;                                                  \
    }))                                                            \
    /***/
#endif

#if uberswitch_GCC_VERSION_ >= 70000 && !defined(Q_CREATOR_RUN)
#   define uberswitch_mayfallthrough_ __attribute__((fallthrough));
    // This apparently unexplicable wrapping is due to a GCC bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85153
#   define uberswitch_PragmaWrap_(x) x
#   define uberswitch_BeginIgnoreWarning_ uberswitch_PragmaWrap_(_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wswitch-unreachable\""))
#   define uberswitch_EndIgnoreWarning_ uberswitch_PragmaWrap_(_Pragma("GCC diagnostic pop"))
#   define uberswitch_goto_(x) uberswitch_BeginIgnoreWarning_ goto x; uberswitch_EndIgnoreWarning_
#else
#   define uberswitch_mayfallthrough_
#   define uberswitch_goto_(x) goto x;
#endif

#define uberswitch(switchVal) uberswitch_impl_(uberswitch_mapclass_, switchVal)
#define dynamic_uberswitch(switchVal)  uberswitch_impl_(uberswitch_class_, switchVal)

#define uberswitch_mapclass_(ValType, varName, initState) uberswitch_thread_local_(uberswitch::MappedSwitch<ValType>, varName, initState)
#define uberswitch_class_(ValType, varName, initState) uberswitch::Switch<ValType> varName(initState)

#define uberswitch_impl_(USwitchClass, switchVal)                                                     \
    uberswitch_var_begin_                                                                             \
        uberswitch_var_(const uberswitch_typeof_(switchVal) &uberswitch_val_ = switchVal)             \
        uberswitch_var_(USwitchClass(uberswitch_decay_(uberswitch_val_), uberswitch_, __COUNTER__+1)) \
        uberswitch_var_(int uberswitch_state_ = uberswitch_.matchedState(uberswitch_val_))            \
    uberswitch_var_end_                                                                               \
    uberswitch_switch_(uberswitch_state_)                                                             \
/***/

#define case(...)                                                                        \
                      uberswitch_goto_(uberswitch_match_)                                    \
    case __COUNTER__: if (!uberswitch_.match(uberswitch_val_, __VA_ARGS__, uberswitch_state_)) { \
                          uberswitch_switch_redo_();                                         \
                      }                                                                      \
    uberswitch_mayfallthrough_                                                               \
    uberswitch_match_:                                                                       \
    case __COUNTER__                                                                         \
/***/

#endif //!UBERSWITCH_HPP_

