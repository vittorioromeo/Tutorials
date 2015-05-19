// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <string>
#include <iostream>
#include <type_traits>

namespace CPP14LanguageFeatures
{
	// Generic lambdas.
	// * Lambda function parameters can now be `auto`.
	// * This effectively makes the lambda behave like
	//   a functor with a templated `operator()`.

	auto l0([](auto x){ return x * 2; });
	
	static_assert(std::is_same<decltype(l0(1)), 
		int>(), "");

	static_assert(std::is_same<decltype(l0(1.f)), 
		float>(), "");

	static_assert(std::is_same<decltype(l0(1.)), 
		double>(), "");



	// Lambdas can also take variadic parameter packs.
	auto l1([](auto... xs){ return sizeof...(xs); });



	// This lambda...
	auto l2([](auto x, auto y){ return x * y; });

	// ...gets converted to something like this during
	// compilation:
	struct CompilerGeneratedL2
	{
	  	template<typename T1, typename T2>
	    auto operator()(T1 x, T2 y) const 
	    {
	    	return x * y;
	    }
	};		
}	

int main()
{
	return 0;
}

// The C++14 standard has a lot more core language features
// and library additions - these are just some of my favorites!

// Here are some helpful links to explore what the new standard
// has to offer:

// * https://isocpp.org/wiki/faq/cpp14-language
// * http://en.wikipedia.org/wiki/C%2B%2B14
// * http://www.drdobbs.com/cpp/the-c14-standard-what-you-need-to-know

// Thank you very much for watching this brief introduction to some
// C++14 core language features.

// Check out the next tutorial for a very interesting code snippet
// implementation and explanation: `forArgs`.

// You can fork/look at the full source code on my GitHub page:
// http://github.com/SuperV1234/

// Check out my website for more tutorials and to personally
// get in touch with me.

// http://vittorioromeo.info