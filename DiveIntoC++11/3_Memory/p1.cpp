// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <vector>

// Welcome to "Dive into C++11", part 3.
// http://vittorioromeo.info

// In this tutorial we're gonna take a look at pointers
// and basic memory management.

// We're gonna learn:
// * The difference between objects allocated on the stack 
//   and on the free-store (heap).
// * What pointers are and how to use them.
// * How to manage dynamic memory (objects on the free-store).

// Let's begin with object lifetime (storage).

int main()
{
	// In C and C++, all variables are allocated with a "storage method".
	// The default method in both languages is "automatic storage".

	// "Automatic storage" and "automatic lifetime" can be used
	// interchangeably.

	// A variable with automatic lifetime is allocated
	// at the beginning of a code block and deallocated
	// at the end of the same block.

	// A code block is a "portion of code" between curly braces.

	{
		// Hello! I'm a code block.
	}



	// When a variable reaches the end of its block, it 
	// is said to be "out-of-scope".

	{
		int var;
		var = 5;
	}

	// 'var' is now out-of-scope.
	// var = 5; <-- compile-time error.



	// Let's create a simple class with a constructor and a
	// destructor to make automatic storage easier to understand.

	// If you are unfamiliar with classes: roughly, a class is a "type" 
	// that provides "structure" and "behavior".

	// In this case, the type `Example` provides:
	// * `id`: an integer member variable
	// * `Example(int mId)`: a constructor taking an int as an argument
	// * `~Example()`: a destructor

	// The constructor gets called on variable allocation.
	// The destructor gets called on variable deallocation.

	// Our constructor and our destructor both print the object's `id`.
	// This will be very useful to understand when an object gets
	// allocated or deallocated.

	struct Example 
	{
		int id;
		Example(int mId) : id{mId}	{ std::cout << "CTOR " << id << std::endl; }
		~Example() 					{ std::cout << "DTOR " << id << std::endl; }
	};



	// Let's now create some instances of `Example`, and analyze
	// when they get allocated/deallocated.

	{
		Example ex1{1};
		Example ex2{2};

		// `ex1` is allocated and constructed, "CTOR 1" will be printed.
		// `ex2` is allocated and constructed, "CTOR 2" will be printed.

		// ...we reach the end of the block.	
	}
	
	// `ex2` and `ex1` are now out of scope.
	// `ex2` is deallocated and destroyed, "DTOR 2" will be printed.
	// `ex1` is deallocated and destroyed, "DTOR 1" will be printed.

	// As you can see, objects with automatic 
	// lifetime are allocated/deallocated in a 
	// LIFO order (last in, first out). 

	// Let's see an example with nested blocks.

	{
		Example ex1{1};
		
		// `ex1` is allocated and constructed, "CTOR 1" will be printed.

		{
			Example ex2{2};

			// `ex2` is allocated and constructed, "CTOR 2" will be printed.

			// ...we reach the end of the block.	
		}

		// `ex2` is now out of scope.
		// `ex2` is deallocated and destroyed, "DTOR 2" will be printed.



		Example ex3{3};

		// `ex3` is allocated and constructed, "CTOR 3" will be printed.

		// ...we reach the end of the block.
	}

	// `ex1` and `ex3` are now out of scope.
	// `ex3` is deallocated and destroyed, "DTOR 3" will be printed.
	// `ex1` is deallocated and destroyed, "DTOR 1" will be printed.
	

	
	// As said previously, the default storage mode
	// in C and C++ is "automatic storage". 

	// So, these variables have automatic storage.
	int intNumber{5};
	std::string str{"I will die at the end of the block :("};
	std::vector<int> vec{1, 2, 3, 4};

	// You can think about automatic storage as if it was
	// a stack. (LIFO order)

	// This is what happens when the above variables get
	// allocated and constrcuted:

	//	0(TOP):	[intNumber	]	[str		]	[vec		]	
	//	1:		[			]	[intNumber	]	[str		]	
	//	2:		[			]	[			]	[intNumber	]	
	//	3:		[			]	[			]	[			]



	// And this is what happens when they get deallocated
	// and destroyed:

	//	0(TOP):	[vec		]	[str		]	[intNumber	]	
	//	1:		[str		]	[intNumber	]	[			]	
	//	2:		[intNumber	]	[			]	[			]	
	//	3:		[			]	[			]	[			]



	// The simplicity of automatic storage make it incredibily
	// fast. You should always use automatic storage when possible,
	// to reduce code complexity and greatly improve performance.

	// Let's move on to the next code segment, where we'll deal
	// with pointers.
}