// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <vector>
#include <string>

// Before dealing with "dynamic storage", we need to learn
// what pointers are.

int main()
{
	// In C and C++, we can think of variables as entities
	// having a "type", a "name", a "value" and an "address".

	int i{10};
	float f{5.f};
	int k;

	//	| 	TYPE 	| 	NAME 	|	VALUE 	| 	ADDRESS 	|
	//  +-----------+-----------+-----------+---------------+
	//	| 	int 	| 	i 		|	10 		| 	&i 			|
	//	| 	float 	| 	f 		|	5.f 	| 	&f 			|
	//	| 	int 	| 	k 		|	??? 	| 	&k 			|

	// What does it mean for variables to have an "address"?
	
	// The address of a variable is the location in memory
	// where the variable is allocated.
	
	// We can get the address of a variable with the `&` unary
	// operator. Getting the address of a variable returns a 
	// pointer of the type of the variable.

	int* 	pointerToI{&i};	// `pointerToI` is a "pointer to int" 	(int*)
	float* 	pointerToF{&f};	// `pointerToF` is a "pointer to float"	(float*)
	int* 	pointerToK{&k};	// `pointerToK` is a "pointer to int" 	(int*)

	// A pointer is basically a memory address.
	// We can access the value contained in that address 
	// (or, "the value the pointer points to") by using the unary
	// operator `*` on the pointer variable.

	std::cout << *pointerToI << std::endl; 	// Prints "10".
	std::cout << *pointerToF << std::endl; 	// Prints "5".

	// We can also modify a pointer's value.
	// Doing so modifies the original variable.
	*pointerToK = 15;

	std::cout << k << std::endl; 			// Prints "15".
	std::cout << *pointerToK << std::endl; 	// Prints "15".

	// The opposite also applies: modifying the original
	// variable will also be reflected when accessing
	// a pointer's contents.

	k = 20;

	std::cout << k << std::endl; 			// Prints "20".
	std::cout << *pointerToK << std::endl; 	// Prints "20".

	// As you can see, since `pointerToK` points to `k`,
	// changing `k` or `*pointerToK` is the same thing.



	// Using pointers in this way does not alter in any way
	// the object's lifetime. Here's an example.

	int* pointerToNested;

	{
		int nestedNumber{42};
		pointerToNested = &nestedNumber;

		// `nestedNumber` "dies" at the end of the block.
	}

	// Even if we `pointerToNested` seems to point to
	// `nestedNumber`, the truth is that `nestedNumber`
	// got deallocated and destroyed at the end of its block,
	// and now `pointerToNested` points to an invalid memory
	// location!

	// Accessing the contents of `pointerToNested` is "undefined
	// behavior". It means that, basically, anything can happen.
	// Usually you get "garbage" values or what is left in memory.

	std::cout << *pointerToNested << std::endl;	// Undefined behavior!

	// It "should" print "42" because it is the value that remained in 
	// memory after destroying `nestedNumber`. Let's see a more explicit
	// example.

	std::vector<int>* pointerToVec;

	{
		std::vector<int> vec{1, 2, 3, 4};
		pointerToVec = &vec;

		// The "arrow" operator (`->`) is basically syntactic
		// sugar for `(*ptr).member`.

		// `pointerToVec->size()` is exactly the same as
		// `(*pointerToVec).size()`.

		std::cout << vec.size() << std::endl;			// Prints 4
		std::cout << pointerToVec->size() << std::endl;	// Prints 4

		pointerToVec->push_back(1);

		std::cout << vec.size() << std::endl;			// Prints 5
		std::cout << pointerToVec->size() << std::endl;	// Prints 5

		// `vec` "dies" at the end of the block.
	}

	pointerToVec->push_back(1);						// Undefined behavior!
	std::cout << pointerToVec->size() << std::endl;	// Undefined behavior!

	pointerToVec->clear();							// Undefined behavior!
	std::cout << pointerToVec->size() << std::endl;	// Undefined behavior!
}

// Notice: even if the "Undefined behavior!" code may seem to work,
// do not be fooled! The code "working" is one possible outcome of
// undefined behavior. Relying on undefined behavior is the worst
// thing you could do - your code will not be portable, optimizations
// may break it, and it may simply fail to work.

// Now that we have a basic knowledge about pointers, let's move
// to the next segment, where we deal with "dynamic object lifetime".

// We will also create our own version of `std::vector<T>`.