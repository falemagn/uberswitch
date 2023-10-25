// Author: Fabio Alemagna <personal@fabioalemagna.net>
// Source: https://github.com/falemagn/fameta-counter
// Inspired to Filip Roséen's work. See https://stackoverflow.com/questions/60082260/c-compile-time-counters-revisited

#ifndef FAMETA_COUNTER_HPP
#define FAMETA_COUNTER_HPP

#if !defined(FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN) && !defined(FAMETA_FRIEND_INJECTION_PRAGMA_END)
#	if defined(__INTEL_COMPILER)
#		define FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN _Pragma("warning push"); _Pragma("warning disable 1624");
#		define FAMETA_FRIEND_INJECTION_PRAGMA_END   _Pragma("warning pop");
#	elif defined(__clang__)
#		define FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN _Pragma("GCC diagnostic push"); _Pragma("GCC diagnostic ignored \"-Wundefined-internal\"");
#		define FAMETA_FRIEND_INJECTION_PRAGMA_END   _Pragma("GCC diagnostic pop");
#	elif defined(__GNUC__)
#		define FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN _Pragma("GCC diagnostic push"); _Pragma("GCC diagnostic ignored \"-Wnon-template-friend\"");
#		define FAMETA_FRIEND_INJECTION_PRAGMA_END   _Pragma("GCC diagnostic pop");
#	else
#		define FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN
#		define FAMETA_FRIEND_INJECTION_PRAGMA_END
#	endif
#endif

namespace fameta
{

// Anonymous namespace to avoid ODR violation
namespace {

template <typename Context, int Start = 0, int Step = 1>
class counter
{
public:
	template <typename Unique>
	static constexpr int next()
	{
		return next<Unique>(0);
	}

	template <unsigned long long UniqueValue>
	static constexpr int next()
	{
		struct Unique{};
		return next<Unique>(0);
	}

	template <typename Unique>
	static constexpr int current()
	{
		return current<Unique>(0);
	}

	template <unsigned long long UniqueValue>
	static constexpr int current()
	{
		struct Unique{};
		return current<Unique>(0);
	}

private:
	template <int I>
	struct slot
	{
		FAMETA_FRIEND_INJECTION_PRAGMA_BEGIN

			friend constexpr bool slot_allocated(slot<I>);

		FAMETA_FRIEND_INJECTION_PRAGMA_END
	};

	template <int I>
	struct allocate_slot {
		friend constexpr bool slot_allocated(slot<I>) {
			return true;
		}

		enum { value = I };
	};

	// If slot_allocated(slot<I>) has NOT been defined, then SFINAE will keep this function out of the overload set...
	template <typename Unique, int I = Start, bool = slot_allocated(slot<I>())>
	static constexpr int next(int)
	{
		return next<Unique, I+Step>(0);
	}

	// ...And this function will be used, instead, which will define slot_allocated(slot<I>) via allocate_slot<I>.
	template <typename Unique, int I = Start>
	static constexpr int next(double)
	{
		return allocate_slot<I>::value;
	}

	// If slot_allocated(slot<I>) has NOT been defined, then SFINAE will keep this function out of the overload set...
	template <typename Unique, int I = Start, bool = slot_allocated(slot<I>())>
	static constexpr int current(int)
	{
		return current<Unique, I+Step>(0);
	}

	// ...And this function will be used, instead, which will return the current counter, or assert in case next() hasn't been called yet.
	template <typename Unique, int I = Start>
	static constexpr int current(double)
	{
		static_assert(I != Start, "You must invoke next() first");

		return I-Step;
	}
};

}

}

#endif // FAMETA_COUNTER_HPP
