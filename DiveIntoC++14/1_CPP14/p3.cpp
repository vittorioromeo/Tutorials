// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <string>
#include <iostream>
#include <type_traits>

namespace CPP14LanguageFeatures
{
	// Relaxed constexpr restrictions.
	// * Allows `constexpr` functions to have multiple
	//   instructions and common language constructs
	//   such as branches and loops.

	constexpr int computeSomething(int mX)
	{
		int result{mX};

		for(int i{0}; i < 10; ++i)
			result += i;

		if(result > 5) result += 10;

		return result;
	}	

	template<int TValue> 
	struct Test
	{

	};

	// Compiles!
	Test<computeSomething(10)> instance;



	// For more `constexpr` coolness, check out this
	// C++Now 2015 talk by Scott Schurr:

	// "constexpr: C++ At Compile Time"

	// All slides from the conference are available here:
	// https://github.com/boostcon/cppnow_presentations_2015/
}	

int main()
{
	return 0;
}
