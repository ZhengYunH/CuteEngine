#pragma once
#include "Common/Config.h"
#include <vector>
#include <functional>

/*
	void Test()
	{
		class A
		{
		public:
			Event<void, int> TestEvent;

			void Run() { TestEvent.BoardCast(1); }
		};

		class B
		{
		public:
			void Recall(int i) { std::cout << i << std::endl; }
		};

		A a;
		B b;
		BIND_EVENT(a.TestEvent, b, Recall);

		a.Run();

		std::cout << std::endl;
	}
*/

namespace zyh
{
	template<typename RetType, typename... ArgsType>
	class Event
	{
	public:
		using BindFuncType = std::function<RetType(ArgsType...)>;

	public:
		virtual void BoardCast(ArgsType... args)
		{
			for (auto& func : BindingFuncs)
			{
				(func)(args...);
			}
		}
		
		virtual void Bind(BindFuncType BindFunc)
		{
			BindingFuncs.push_back(BindFunc);
		}

	protected:
		std::vector<BindFuncType> BindingFuncs;
	};
}
