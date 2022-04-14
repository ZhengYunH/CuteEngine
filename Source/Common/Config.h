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



#include<vector>
#include<map>

enum RenderSet : uint32_t
{
	NONE = 0,
	SCENE = 1,
	SHADOW = 2
};

#include <vector>
typedef std::vector<bool> BitArray;

#define BIND_EVENT(Event, Object, FuncName) Event.Bind(std::bind(&decltype(b)::FuncName, &b, std::placeholders::_1));