// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Welcome to "Dive into C++11", part 4.
// http://vittorioromeo.info

// In this tutorial we're gonna take a look at one of the
// best C++11 features: smart pointers.

// We're gonna learn:
// * What smart pointers are, and what problem they solve.
// * How to deal with ownership transfer.
// * The difference between `unique` and `shared` smart pointers.

// I highly reccommend watching part 3 before this video.
// Let's dive in!

int main()
{
	// As explained in part 3, we must be careful when dealing
	// with pointers that store addresses of objects allocated
	// on the free-store. Example:

	{
		int* correctUsage{new int{20}};
		delete correctUsage;
	}

	{
		int* memoryLeak{new int{50}};
	}

	// Whoops, we forgot to call `delete`. Great. 
	// Now our program leaks memory.

	// Hmm... wonder if we could create a simple wrapper
	// class that automatically called `delete` for us
	// on destruction.

	class FreeStoreInt
	{
		// FreeStoreInt is a class that wraps an `int`
		// instance allocated on the free-store.

		private:
			int* pointer; // Points to our free-store-allocated `int`.

		public:
			// The constructor allocated the `int` on the free-store,
			// using the value passed by the user.
			FreeStoreInt(int* mInt) : pointer{mInt} { }

			// The destructor calls delete on `pointer`. 
			// This ensures that the memory will be freed when an instance
			// of FreeStoreInt goes out-of-scope.
			~FreeStoreInt() { delete pointer; }

			// And this is a simple "getter" function that allows us
			// to access the internal pointer.
			int* get() { return pointer; }
	};

	// Great! Let's use our new class.

	{
		// Here `test` is constructed, with a value of 10.
		// Remember: `test.pointer`now points to a free-store-allocated `int`!
		FreeStoreInt test{new int{10}};

		// We DO NOT HAVE to call `delete`! 
		// When `test` goes out of scope, the destructor will be
		// automatically called, and `test.pointer` will be deleted
		// for us.
	}

	// Fantastic! We have no memory leaks.

	// Turns out our class has some problems, though:
	// * It is not generic. Can be easily fixed with templates
	//   and C++11 variadic templates.
	// * What happens if we try to copy our class? What if
	//   we try to move it? It would be a good idea to take a lot 
	//   of time to design and implement copy/move constructors,
	//   assignment operator overloads, et cetera...
	// * What if we need to store `FreeStoreInt` instances in a 
	//   container? What are the implications?
	// * And much more...

	// Turns out our life-changing class is not that great...
	// Well, fortunately the C++11 Standard Library introduces
	// a new class, `std::unique_ptr`, which basically solves the 
	// issue we were trying to solve with `FreeStoreInt`.

	// All smart pointers are in the `memory` header file.
	
	// Let's analyze the simplest (yet, usually most useful) type of
	// smart pointer: `std::unique_ptr`.

	{
		// `std::unique_ptr` is a clever well thought-out version
		// of our naive `FreeStoreInt`. On construction, it allocates
		// and constructs an object with the user specified parameters.
		std::unique_ptr<int> test{new int{10}};

		// And, like our `FreeStoreInt`, it will automatically free
		// memory on destruction (when it goes out-of-scope).

		// Also, it solves all the issues listed above!
	}

	{
		std::unique_ptr<double> smartPointers{new double{20.3}};
		std::unique_ptr<std::string> are{new std::string{"extremely"}};
		std::unique_ptr<char> versatile{new char{'!'}};

		// Do not worry! All memory will automatically be freed.
		// This is GUARANTEED, even in case of thrown exceptions.
		// `std::unique_ptr` and other smart pointers are extremely
		// safe and should always be used instead of "raw" pointers.
	}

	// <<That's great! I won't need "raw" pointers anymore, then!>>
	// Nope. "Raw" pointers are still very useful. Here's what you should do:

	// You need to store something on the free-store -> USE A SMART POINTER.
	// (Basically, when your pointer needs to "own" memory, USE A SMART POINTER.)

	// You need to refer to something, WITHOUT OWNING IT -> USE A RAW POINTER.

	// Examples:

	{
		std::size_t runtimeSize{10};

		// We need to allocate an array with a runtime-known size.
		// We need the free-store. We need a pointer to "own" the array,
		// therefore we use a smart pointer.

		std::unique_ptr<int[]> array{new int[runtimeSize]};

		// Maybe we now need to simply refer to `array`, without representing
		// ownership. Maybe we need to refer to `array` in a function, or
		// maybe we just need an alias that changes its pointee during the
		// program execution. Therefore, we use a "raw" pointer.

		// `std::unique_ptr::get()` returns a "raw" pointer to the stored
		// object.
		// (Rember to use round parenthesis with `auto`!) 
		auto pointerToArray(array.get());

		// Careful: if `pointerToArray` goes out of scope, `array` won't 
		// be deleted. Only the smart pointer frees memory on destruction!
	}

	{
		std::string testString{"this string is not on the free store"};

		// Again, we may need to refer to `testString` without owning it.
		// A "raw" pointer is what we need here.

		std::string* pointerToString{&testString};
	}

	// Also, `std::unique_ptr` should have zero overhead compared to "raw"
	// owning pointers. So feel free to use it without performance worries! 

	// Let's move on to the next part, and take a look at possible ownership
	// issues.

	return 0;
}
