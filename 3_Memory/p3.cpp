// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <vector>

// "Dynamic storage" refers to the dynamic allocation/deallocation
// of objects on the free-store.

// To allocate/deallocate objects dynamically, in C++ we use the
// `new` and `delete` keywords.

int main()
{
	// Allocating an object dynamically practically means  
	// grabbing and using an available piece of the 
	// "free-store" (available memory storage) at run-time.

	// To allocate an object dynamically, we use the `new`
	// keyword, which returns the address in which the 
	// object was allocated.
	int* dynamicNumber{new int{1}};

	// Dynamic deallocation IS NOT AUTOMATIC. `dynamicNumber`
	// won't be destroyed at the end of the block: its 
	// destructor won't be called and memory won't be freed!

	// Dynamically allocated objects must be deallocated 
	// manually with the `delete` keyword. Forgetting to do 
	// so is the primary cause of "memory leaks".

	// To deallocate a dynamically-allocated object, we
	// use the `delete` keyword.
	delete dynamicNumber;

	// After calling delete, the dynamically-allocated object
	// destructor will be called, and memory will be freed.
	// The `dynamicNumber` pointer will become invalid,
	// as it now points to a freed memory location.

	// It is good practice to set the pointer to `nullptr`,
	// a new C++11 keyword symbolizing a "pointer to null",
	// when dealing with manual dynamic object management.
	dynamicNumber = nullptr;

	// Obviously, you could also re-use the pointer to allocate a new
	// `int` on the free-store, or to point to an existing `int` 
	// instance.



	// Let's see a more complex example. We'll use the
	// class we used in the previous code segment.

	struct Example 
	{
		int id;
		Example(int mId) : id{mId}	{ std::cout << "CTOR " << id << std::endl; }
		~Example() 					{ std::cout << "DTOR " << id << std::endl; }
	};

	{
		Example* ex1{new Example{1}};
		// `ex1` is allocated and constructed, "CTOR 1" will be printed.

		Example* ex2{nullptr};
		// `ex2` is currently only a pointer: it does not point 
		// to any dynamically-allocated object, yet. As before, it is
		// good practice to use `nullptr` to initialize it.

		{
			Example* ex3{new Example{3}};
			ex2 = ex3;

			// `ex3` is allocated and constructed, "CTOR 3" will be printed.

			// We also assign `ex2` to `ex3`: this only assigns the address! 
			
			// You can read `ex2 = ex3;` as: 
			// "ex2 and ex3 now point to the same address".

			// What I want to be clear is that the contents 
			// of `ex2` and `ex3` are unaffected by the assignment.
			// We're only dealing with the addresses here, not with
			// the contents.
		}

		// Uh-oh. Even if we reached the end of the block, "DTOR 3" 
		// was not printed! And ex3 went out of scope.

		// This is a clear demonstration that dynamically-allocated objects 
		// are not deallocated/destroyed at the end of a block, 
		// like automatic-storage variables.

		// Fortunately, `ex2` now points to the same dynamic `Example`
		// instance `ex3` pointed to in the nested block. We can avoid
		// a memory leak by calling `delete` on `ex2`.
		// This will free the memory location that `ex3` was pointing to
		// before `ex3` went out of scope.

		// We will also delete the other dynamically-allocated
		// `Example` instance, `ex1`.

		delete ex1; // This prints "DTOR 1"!
		delete ex2; // This prints "DTOR 3"!

		// Notice how the destructors were called in a non-ordered
		// manner - we aren't following the LIFO principle anymore.
	}

	// So... why are dynamically-allocated objects useful? 

	// Well, the word "dynamic" should be a good hint. Dynamically
	// allocating an object allows us to specify how much memory
	// we want to allocate at run-time.

	// Automatic-storage, on the other hand, requires to know
	// how much memory to allocate at compile-time.

	// Even if you haven't used arrays before, this example may 
	// clarify the above statements.
	// An array is a contiguous memory block that can contain
	// a specific number of objects of the same type.

	// Creating an automatic-storage array requires a compile-time
	// constant size. The compiler needs to know how much memory to
	// allocate!

	// Valid, 10 is a compile-time constant. (int literal)
	int automaticArray1[10];		

	// `constexpr` is a new C++11 keyword. Watch the earlier "Dive into C++11"
	// videos for an in-depth explanation.
	constexpr int arraySize{5 + 5};

	// Valid, `arraySize` is a compile-time constant. (constexpr)
	int automaticArray2[arraySize];	

	/*
	{
		int runtimeSize;
		std::cin >> runtimeSize;

		// Invalid! `runtimeSize` is not a compile-time constant.
		int invalidArray[runtimeSize]; 

		// This commented code may compile and work on your machine.
		// Runtime-sized array, however, are not standardized.
		// Using the `-pedantic` compiler flag will in fact show
		// a warning.
	}
	*/
	
	// To allocate an array with a size known only at run-time, we need to
	// use dynamic allocation.

	int runtimeSize;
	std::cin >> runtimeSize;

	int* pointerToArray;
	pointerToArray = new int[runtimeSize];

	// To deallocate a dynamically-allocated array, we must use the `delete[]`
	// operator. 

	// CAUTION! Using `delete` and not `delete[]` will not free all the memory
	// and may be source of bugs and undefined behavior.

	// delete pointerToArray; <-- INVALID!
	
	delete[] pointerToArray; // Correct!



	// Ever wondered how `std::vector<T>` allows an user to store an indefinite
	// amount of objects?

	// Well, now you know! Interally, it uses a dynamically-allocated array.

	// Now you may ask.. why should we use an `std::vector<T>` instead of a
	// dynamically-allocated array?

	// Well, apart from the nice object-oriented interface, the most important
	// advantage is SAFETY.

	// Let's create our own naive "vector", that will only store `int` values.

	struct NaiveVector
	{
		// Capacity of the vector. (how many objects it can hold)
		int capacity{2};	
			
		// Pointer to the dynamically-allocated array.
		int* ptrToArray{new int[capacity]};

		// Size of the vector. (how many objects it is currently storing)
		int size{0};

		void push_back(int mValue)
		{
			// Set the last element to the array to the new value.
			ptrToArray[size] = mValue;

			// Increase the size: the vector is now storing an additional object.
			++size;

			// If the current capacity is not enough for the new size, 
			// we must reallocate the dynamic array with a bigger capacity.
			// Doubling the capacity sounds good.
			if(capacity < size)
			{
				std::cout << "Reallocating internal array!" << std::endl;

				// We create a dynamically-allocated array with double
				// the capacity of the previous one.
				capacity *= 2;
				int* ptrToNewArray{new int[capacity]};

				// We copy the current values of the vector into the new array.
				for(int i{0}; i < size; ++i) ptrToNewArray[i] = ptrToArray[i];

				// We delete the current array, and set the array pointer 
				// to the new one.
				delete[] ptrToArray;
				ptrToArray = ptrToNewArray;				
			}
		}		

		// The `printValues()` method prints information for testing purposes.
		void printValues() 
		{ 
			std::cout << std::endl;

			for(int i{0}; i < size; ++i)  std::cout << ptrToArray[i] << ", ";

			std::cout << std::endl << "Size: " << size << std::endl;
			std::cout << "Capacity: " << capacity << std::endl << std::endl; 
		}
	};

	// Let's test our horrible and naive vector implementation!

	NaiveVector nv;		// Size: 0	|	Capacity: 2

	nv.push_back(1);	// Size: 1	|	Capacity: 2
	nv.printValues();

	nv.push_back(2);	// Size: 2	|	Capacity: 4
	nv.push_back(3);	// Size: 3	|	Capacity: 4
	nv.push_back(4);	// Size: 4	|	Capacity: 4
	nv.printValues();

	nv.push_back(5);	// Size: 5	|	Capacity: 8
	nv.push_back(6);	// Size: 6	|	Capacity: 8
	nv.push_back(7);	// Size: 7	|	Capacity: 8
	nv.push_back(8);	// Size: 8	|	Capacity: 8	
	nv.push_back(9);	// Size: 9	|	Capacity: 16
	nv.push_back(10);	// Size: 10	|	Capacity: 16
	nv.push_back(11);	// Size: 11	|	Capacity: 16
	nv.push_back(12);	// Size: 12	|	Capacity: 16
	nv.push_back(13);	// Size: 13	|	Capacity: 16
	nv.printValues();	
}

// Dynamic memory management can be tedious and error-prone.
// Thankfully, C++11 offers some new features that make it much
// easier and safer. We will take a look at those features in
// the next tutorial, along with "references" and the differences
// between references and pointers.



// Thank you very much for watching!
// Hope you found the interesting.

// You can fork/look at the full source code on my GitHub page:
// http://github.com/SuperV1234/

// Check out my website for more tutorials and to personally
// get in touch with me.

// http://vittorioromeo.info