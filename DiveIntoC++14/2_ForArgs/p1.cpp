// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>

// Welcome to "Dive into C++14", part 2.
// http://vittorioromeo.info

// In this tutorial video we'll take a look at a very 
// interesting code snippet, originally posted on Twitter
// by Sean Parent: <tweet0.png>.

// We'll also cover a very useful C++14 standard library
// addition: "Compile time integer sequences".

// This tutorial video is a reviewed and improved version
// of my C++Now 2015 lightning talk:

// "`for_each_arg` explained and expanded"

// The original files can be found here:
// https://github.com/SuperV1234/cppnow2015

// Also, slides from the conference are available here:
// https://github.com/boostcon/cppnow_presentations_2015

// ----------------------------------------------------------------

// So, what does `for_each_argument` do?
// Well, the name is pretty self-explanatory...

// It invokes a callable object on every passed argument.

template <class F, class... Args>
void for_each_argument(F f, Args&&... args) {
    [](...){}((f(std::forward<Args>(args)), 0)...);
}

int main()
{
	// Prints "hello123".
	for_each_argument
	(
		[](const auto& x){ std::cout << x; },

		"hello",
		1,
		2,
		3
	);

	std::cout << "\n";
	return 0;
}

// That is cool. How does it work?
// Let's discover that in the next code segment.