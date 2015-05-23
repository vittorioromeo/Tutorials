// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

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

// This code segments shows another interesting use case:
// iteration over `std::tuple` elements.

// Example use case: `forTuple` function.

// We can use `forArgs` as a building block for an `std::tuple`
// element iteration function. 

// To do so, we require an helper function that expands the
// elements of the `std::tuple` into a function call.

// The following helper function is taken from paper N3802,
// proposed by Peter Sommerlad. 

// You can find the paper at the following address:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3802.pdf

// Many similar implementations of the same function can be
// easily found online.

// ----------------------------------------------------------------

// We do not really need to know how this function works for the
// scope of this video. 

template<typename F, typename Tuple, size_t... I>
decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
	return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template<typename F, typename Tuple>
decltype(auto) apply(F&& f, Tuple&& t)
{
	using Indices =
		std::make_index_sequence<std::tuple_size<
			std::decay_t<Tuple>>::value>;

	return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), 
		Indices{});
}

// ----------------------------------------------------------------

// `forTuple` is a function that takes a callable object and
// and `std::tuple` as its parameters.

// It then calls the passed function individually passing every
// element of the tuple as its argument.

template<typename TFn, typename TTpl>
void forTuple(TFn&& mFn, TTpl&& mTpl)
{
	// We basically expand the tuple into a function call to
	// a variadic polymorphic lambda with `apply`, which in
	// turn passes the expanded tuple elements to `forArgs`,
	// one by one... which in turn calls `mFn` with every
	// single tuple element individually.

	apply
	(
		// The callable object we will pass to `apply` is
		// a generic variadic lambda that forwards its
		// arguments to `forArgs`.
		[&mFn](auto&&... xs)
		{
			// The `xs...` parameter pack contains the 
			// `mTpl` tuple elements, expanded thanks
			// to `apply`.

			// We will call the `mFn` unary function
			// for each expanded tuple element, thanks
			// to `forArgs`.
			forArgs
			(
				mFn,
				std::forward<decltype(xs)>(xs)...
			);
		},

		std::forward<TTpl>(mTpl)
	);
}

int main()
{
	// Prints "10 hello 15 c".
	forTuple
	(
		[](const auto& x){ std::cout << x << " "; },
		std::make_tuple(10, "hello", 15.f, 'c')
	);	

	// This is roughly equivalent to writing:
	/*
		forArgs
		(
			[](const auto& x){ std::cout << x << " "; },
			
			10,
			"hello",
			15.f,
			'c'
		);
	
		// ...which, in turn, is roughly equivalent to:
	
		std::cout << 10 << " ";
		std::cout << "hello" << " ";
		std::cout << "15.f" << " ";
		std::cout << 'c' << " ";
	*/

	std::cout << "\n";
	return 0;
}

// All of this is extremely cool and useful - but we're
// limited to unary functions.

// What if we want to take arguments two by two?
// Or three by three?

// It is actually possible to create a generic version
// of `forArgs` that takes the arity of the passed callable
// object as a template parameter.

// Let's see an implementation of that...