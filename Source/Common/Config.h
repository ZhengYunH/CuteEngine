#pragma once
#ifdef NDEBUG

#else
	#define ZYH_DEBUG
#endif // !NDEBUG


#ifdef ZYH_DEBUG
#include <assert.h>
#define HYBRID_CHECK(X, ...) assert((X))
#else
#define HYBRID_CHECK(X, ...)
#endif

#define UNEXPECTED(X) (X)
#define SafeDestroy(X) if(X){ delete X; X=nullptr;}
#define SafeDestroyArray(X) if(X){delete[] X; X=nullptr;}



#include <vector>
#include <map>
#include <functional>
#include <stdint.h>
#include <time.h>
#include <chrono>


enum RenderSet : uint32_t
{
	NONE = 0,
	SCENE = 1,
	SHADOW = 2
};

#include <vector>
typedef std::vector<bool> BitArray;


template<int N, int ...I>
struct MakeSeqs : MakeSeqs<N - 1, N - 1, I...> {};

template<int...I>
struct MakeSeqs<1, I...>
{
	template<class T, class _Fx>
	static auto bind(T&& _Obj, _Fx&& _Func)
	{
		return std::bind(std::forward<_Fx>(_Func), std::forward<T>(_Obj), std::_Ph<I>{}...);
	}
};

template <class T, typename RetType, typename...Args>
auto Bind(RetType(T::* f)(Args...), T* t)
{
	return MakeSeqs<sizeof...(Args) + 1>::bind(t, f);
}

#define BIND_EVENT(Event, Object, Func) Event.Bind(Bind(&Func, &Object));