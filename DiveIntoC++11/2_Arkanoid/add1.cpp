// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>

// The `constexpr` keyword can also be applied to functions

// A big restriction (that will be lifted in C++14) is that
// `constexpr` functions can only be made of a `return`
// statement

// `constexpr` functions will be resolved at compile-time
// if the passed arguments are constant expressions, otherwise
// they may get resolved at run-time

constexpr int getSum(int mA, int mB, int mC)
{
	return mA + mB + mC;
}

int main()
{
	// getSum(1, 2, 3 + 5) will get resolved at compile-time
	// because `1`, `2`, `3 + 5` are `constexpr` expressions
	std::cout << getSum(1, 2, 3 + 5) << std::endl;

	int input;
	std::cin >> input;

	// getSum(input, 10) will get resolved at run-time
	// as `input` is not a `constexpr` and it cannot be
	// known at compile-time
	std::cout << getSum(input, 10, 1) << std::endl;

	// Since `constexpr` is resolved at compile-time,
	// we can use it, for example, to declare arrays
	int array[getSum(10, 10, 10)]; // int array[30];

	return 0;
}