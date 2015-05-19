// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <string>
#include <iostream>
#include <type_traits>
#include <vector>

// Welcome to "Dive into C++14", part 1.
// http://vittorioromeo.info

// C++11 is awesome, but C++14 is even better.

// C++14-compliant compilers are becoming more widespread.
// It is time to leave C++11 behind and move onto the 
// newest standard of the C++ language.

// I'm, therefore, introducing a new YouTube video series,
// dedicated to C++14.

// The videos will share the same format and topics as the
// previous ones: 
// * Chronologically sequential commented code segments.
// * Screencasting + live compilation and execution.
// * Various topics: game development, language features,
//   interesting code snippets, best practices, etc...

// Compiler support:
// * "clang++ 3.5" has full C++14 support.
// * "g++ 5.0" has full C++14 support.
 
// Refer to the following links for more information:
// * http://en.cppreference.com/w/cpp/compiler_support
// * http://clang.llvm.org/cxx_status.html
// * https://gcc.gnu.org/projects/cxx1y.html

// Let's take a quick look at some of my favorite new 
// core language features of the C++14 standard:
// * Function return type deduction.
// * `decltype(auto)`.
// * Relaxed constexpr restrictions.
// * Variable templates.
// * Generic lambdas.

// If you are familiar with the features listed above,
// feel free to skip to the next video in the series,
// which shows a very interesting C++14 code snippet.

namespace CPP14LanguageFeatures
{
	// Function return type deduction.
	// * This feature allows the use of the keyword `auto`
	//   in place of a function's return type.
	// * You can qualify the `auto` keyword with `const`,
	//   `*`, `&`, etc...
	// * It follows the same rules as `auto` variable 
	//   type deduction.

	auto func0()
	{
		return 0;
	}	

	static_assert(std::is_same<decltype(func0()), 
		int>(), "");



	auto func1()
	{
		std::string test{"hello!"};
		return test;
	}

	static_assert(std::is_same<decltype(func1()), 
		std::string>(), "");



	const auto& func2()
	{
		static std::string test{"hello again!"};
		return test;
	}
 
	static_assert(std::is_same<decltype(func2()), 
		const std::string&>(), "");


	// Here are some additional examples:
	template<typename T> 
	struct SomeContainerWrapper
	{
		std::vector<const T*> vec;

		typename std::vector<const T*>::iterator
			beginCPP03() const
		{
			return std::begin(vec);
		}

		auto beginCPP11() const 
			-> decltype(std::begin(vec))
		{
			return std::begin(vec);
		}

		auto beginCPP14() const { return std::begin(vec); }
	};



	template<typename T1, typename T2, typename T3>
	auto complicatedFuncCPP11(T1 x, T2 y, T3 z)
		-> decltype((x * y) - (y * z))
	{
		return (x * y) - (y * z);
	}

	template<typename T1, typename T2, typename T3>
	auto complicatedFuncCPP14(T1 x, T2 y, T3 z)	
	{
		return (x * y) - (y * z);
	}
}

int main()
{
	return 0;
}