#pragma once
#ifdef NDEBUG

#else
	#define ZYH_DEBUG
#endif // !NDEBUG


#ifdef ZYH_DEBUG
#include <assert.h>
#define HYBRID_CHECK(X, ...) assert((X))
#define DEBUG_RUN(X) X
#else
#define HYBRID_CHECK(X, ...)
#define DEBUG_RUN(X)
#endif

#define Unimplement(...) HYBRID_CHECK(0, ##__VA_ARGS__)
#define UNEXPECTED(X) (X)
#define SafeDestroy(X) if(X){ delete X; X=nullptr;}
#define SafeDestroyArray(X) if(X){delete[] X; X=nullptr;}



#include <vector>
#include <map>
#include <functional>
#include <stdint.h>
#include <time.h>
#include <chrono>
#include <vector>


enum RenderSet : uint32_t
{
	NONE = 0,
	SCENE = 1,
	SHADOW = 2
};
typedef std::vector<RenderSet> TRenderSets;

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


template<size_t N> struct unroll
{
	template<typename F, typename... Args> static void call(F const& f, Args... args)
	{
		f(args...);
		unroll<N - 1>::call(f, std::forward<Args>(args)...);
	}
};

template<> struct unroll<0>
{
	template<typename F, typename... Args> static void call(F const& f, Args... args) {}
};