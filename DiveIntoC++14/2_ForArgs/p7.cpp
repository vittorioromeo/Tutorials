// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <utility>
#include <iostream>
#include <tuple>
#include <unordered_map>

// WIP, TODO

// We're going to implement a `make_unordered_map` function in
// this code segment, similar to the previous `make_vector`.

// As `std::unordered_map` is an associative container, we
// will use `forNArgs<2>` to pass "key-value pairs".

// ----------------------------------------------------------------

template<typename, typename> struct forNArgsImpl;

template<std::size_t TArity, typename TF, typename... Ts>
void forNArgs(TF&& mFn, Ts&&... mXs)
{
	constexpr auto numberOfArgs(sizeof...(Ts));

	static_assert(numberOfArgs % TArity == 0, 
		"Invalid number of arguments");

	forNArgsImpl
	<
		std::make_index_sequence<numberOfArgs / TArity>,
		std::make_index_sequence<TArity>
	>
	::exec
	(
		mFn,
		std::forward_as_tuple(std::forward<Ts>(mXs)...)
	);
}

template<std::size_t... TNCalls, std::size_t... TNArity>
struct forNArgsImpl
<
	std::index_sequence<TNCalls...>,
	std::index_sequence<TNArity...>
>
{
	template<typename TF, typename... Ts>
	static void exec(TF&& mFn, const std::tuple<Ts...>& mXs)
	{
		constexpr auto arity(sizeof...(TNArity));
		using swallow = bool[];

		(void) swallow
		{
			(execN<TNCalls * arity>(mFn, mXs), true)...
		};
	}

	template<std::size_t TNBase, typename TF, typename... Ts>
	static void execN(TF&& mFn, const std::tuple<Ts...>& mXs)
	{
		mFn
		(
			std::get<TNBase + TNArity>(mXs)... 
		);
	}
};

// ----------------------------------------------------------------

// Example use case: `make_unordered_map` function.

// `make_unordered_map` will take arguments in groups of two and
// and return an `std::unordered_map` having the first arguments
// of every group as keys and the second arguments of every group
// as values.

template<typename... TArgs>
auto make_unordered_map(TArgs&&... mArgs);

// Our first job is defining an helper that will allow us
// to deduce the common type for all keys and the common type
// for all values of the `std::unordered_map`.

template<typename TIdxs, typename... Ts>
struct MakeUnorderedMapHelper2;

template<std::size_t... TIs, typename... Ts>
struct MakeUnorderedMapHelper2
<
	std::index_sequence<TIs...>,
	Ts...
>
{
	static_assert(sizeof...(Ts) % 2 == 0, "");

	template<std::size_t TS> 
	using TypeAt = std::tuple_element_t<TS, std::tuple<Ts...>>;

	using KeyType = std::common_type_t<TypeAt<TIs * 2>...>;
	using ValueType = std::common_type_t<TypeAt<(TIs * 2) + 1>...>;
};

template<typename TK, typename TV, typename... Ts>
struct MakeUnorderedMapHelper
{
	using NextKeyType = typename MakeUnorderedMapHelper<Ts...>::KeyType;
	using NextValueType = typename MakeUnorderedMapHelper<Ts...>::ValueType;

	using KeyType = std::common_type_t<TK, NextKeyType>;
	using ValueType = std::common_type_t<TV, NextValueType>;
};

template<typename TK, typename TV>
struct MakeUnorderedMapHelper<TK, TV>
{
	using KeyType = TK;
	using ValueType = TV;
};

template<typename... Ts> using HelperFor = MakeUnorderedMapHelper2
<
	std::make_index_sequence<sizeof...(Ts) / 2>,
	Ts...
>;

template<typename... Ts> 
using CommonKeyType = typename HelperFor<Ts...>::KeyType;
// using CommonKeyType = typename MakeUnorderedMapHelper<Ts...>::KeyType;

template<typename... Ts> 
using CommonValueType = typename HelperFor<Ts...>::ValueType;
// using CommonValueType = typename MakeUnorderedMapHelper<Ts...>::ValueType;

static_assert(std::is_same
<
	CommonKeyType<std::string, int>, 
	std::string
>(), "");

static_assert(std::is_same
<
	CommonValueType<std::string, int>, 
	int
>(), "");

static_assert(std::is_same
<
	CommonKeyType
	<
		std::string, int, 
		std::string, float
	>, 
	std::string
>(), "");

static_assert(std::is_same
<
	CommonValueType
	<
		std::string, int, 
		std::string, float
	>, 
	float
>(), "");


template<typename... TArgs>
auto make_unordered_map(TArgs&&... mArgs)
{
	using KeyType = CommonKeyType<TArgs...>;
	using ValueType = CommonValueType<TArgs...>;

    std::unordered_map<KeyType, ValueType> result;
    result.reserve(sizeof...(TArgs) / 2);

    forNArgs<2>
    (
        [&result](auto&& k, auto&& v)
        {
            result.emplace
            (
                std::forward<decltype(k)>(k),
                std::forward<decltype(v)>(v)
            );
        },

        std::forward<TArgs>(mArgs)...
    );

    return result;
}


int main()
{
	// Prints "012".

	using namespace std::literals;

	auto m = make_unordered_map
    (
        "zero"s, 0,
        "one"s, 1,
        "two", 2.f
    );

    static_assert(std::is_same
    <
    	decltype(m),
    	std::unordered_map<std::string, float>
    >(), "");

    std::cout << m["zero"] << m["one"] << m["two"];

    std::cout << "\n";
	return 0;
}
