#pragma once
#ifdef NDEBUG

#else
	#define ZYH_DEBUG
#endif // !NDEBUG


#ifdef ZYH_DEBUG
#define HYBRID_CHECK(X, ...) assert(X)
#else
#define HYBRID_CHECK(X, ...)
#endif


#define SafeDestroy(X) if(X){ delete X; X=nullptr;}