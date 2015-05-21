// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <utility>
#include <iostream>
#include <tuple>

template<typename TF, typename... Ts>
void forArgs(TF&& mFn, Ts&&... mArgs)
{
	return (void) std::initializer_list<int>
	{
		(
			mFn(std::forward<Ts>(mArgs)),
			0
		)...
	};
}

// The implementation I'm about to show you uses a new library
// feature introduced in C++14: compile-time "integer sequences".

// Before moving on, let's see what they are and what they can do.

// Positive integer sequences using `std::size_t` as their underlying
// index type are called "index sequences".
// They can be generated using `std::make_index_sequence.

// `Seq0` is a compile-time index sequence.
// It goes from `0` (inclusive) to `10` (non-inclusive).
using Seq0 = std::make_index_sequence<10>;

// To retrieve the numbers in the sequence, we must match it
// using template specializations and expand it with `...`.

// Let's forward declare a `struct` that will print an index
// sequence to the standard output.
template<typename> 
struct SeqPrinter;

// Let's now specialize it to match an index sequence:
template<std::size_t... TIs> 
struct SeqPrinter<std::index_sequence<TIs...>>
{
	// And let's use our `forArgs` function to print
	// the indices:
	static void print()
	{
		forArgs
		(
			[](auto x){ std::cout << x << " "; },

			// We can expand the matched indices here:
			TIs...
		);
	}
};

int main()
{
	// Let's try it out now.

	// Prints "0 1 2 3 4 5 6 7 8 9".
	SeqPrinter<Seq0>::print();
	std::cout << "\n";

	// Prints "0 1 2 3 4".
	SeqPrinter<std::make_index_sequence<5>>::print();
	std::cout << "\n";

	return 0;
}