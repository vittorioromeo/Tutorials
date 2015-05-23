// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <string>
#include <iostream>
#include <type_traits>

namespace CPP14LanguageFeatures
{
	// `decltype(auto)`
	// 
	//    * `auto` will always deduce a non-reference type.
	//    
	//    * `auto&&` will always deduce a reference type.
	//    
	//    * `decltype(auto)` deduces a non-reference or 
	//      reference type depending upon the value category
	//      and the nature of a particular expression.
	// 

	auto func3()
	{
		static std::string test{"bye!"};
		
		auto& result(test);
		return result;
	}

	static_assert(std::is_same<decltype(func3()), 
		std::string>(), "");



	decltype(auto) func4()
	{
		static std::string test{"bye again!"};
		
		auto& result(test);
		return result;
	}

	static_assert(std::is_same<decltype(func4()), 
		std::string&>(), "");



	decltype(auto) func5()
	{
	 	std::string test{"bye one more time!"};			
		return std::move(test);

		// By the way, do not `std::move` things out 
		// of a function.
		// This actually returns a reference to a 
		// local object, which is not right!
		// Rely on the compiler's RVO, instead.
	}

	static_assert(std::is_same<decltype(func5()), 
		std::string&&>(), "");



	// If you want to know more about type deduction
	// in the latest standard, check out this
	// C++Now 2015 talk by David Stone:

	// "Type Deduction in C++14"

	// All slides from the conference are available here:
	// https://github.com/boostcon/cppnow_presentations_2015/
}	

int main()
{
	return 0;
}