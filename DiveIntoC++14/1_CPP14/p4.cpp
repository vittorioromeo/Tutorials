// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <string>
#include <iostream>
#include <type_traits>

namespace CPP14LanguageFeatures
{
	// Variable templates.
	// 
	//    * C++14 allows the creation of templated variables.
	// 
	//    * Variable templates can also be specialized.
	// 

	template<typename T> T pi{3.14159265359};

	void test0()
	{
		auto piInt(pi<int>);
		auto piFloat(pi<float>);
		auto piDouble(pi<double>);
	}	



	template<typename T> int typeID;
	template<> constexpr int typeID<int>{0};
	template<> constexpr int typeID<float>{1};
	template<> constexpr int typeID<double>{2};

	void test1()
	{
		static_assert(typeID<int> == 0, "");
		static_assert(typeID<float> == 1, "");
		static_assert(typeID<double> == 2, "");
	}
}	

int main()
{
	return 0;
}
