// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Let's define an example class, `Resource`, that we'll use in
// our examples.
struct Resource { };

void passByValue(std::unique_ptr<Resource> mResPtr) 			{ /*...*/ }
void passByConstRef(const std::unique_ptr<Resource>& mResPtr) 	{ /*...*/ }
void passByRawPtr(Resource* mResPtr) 							{ /*...*/ }

void whatShouldIPassByToMaintainOriginalOwnership()
{
	// Example situation: after we acquire a resource, we need to
	// refer to its pointer, maintaining ownership.	
	std::unique_ptr<Resource> resPtr{new Resource};  



	// Compile-time error! `std::unique_ptr` cannot be copied!
	/* passByValue(resPtr); */

	// `std::unique_ptr` cannot be copied because otherwise we would end
	// up in a situation where two `std::unique_ptr` instances "own" the
	// same free-store-allocated object. It makes no sense because, as
	// the class's name implies, `std::unique_ptr` is the "unique" owner,
	// there is no shared ownership.



	// The following code is fine, and I personally use this solution.
	passByConstRef(resPtr);

	// We're not transferring ownership here, we're just referring to the
	// `std::unique_ptr` indirectly, and by doing so we can easily access
	// the smart pointer's contents.



	// This is another solution, it works properly, but I still prefer
	// passing by const reference.
	passByRawPtr(resPtr.get());

	// Remember: `std::unique_ptr::get()` returns a raw pointer to the
	// smart pointer's contents.
}

void iHaveToTransferOwnershipWhatNow()
{
	// Example situation: after we acquire a resource, we need to
	// transfer ownership from a smart pointer to another.
	std::unique_ptr<Resource> resPtr{new Resource};  

	// `std::unique_ptr` cannot be copied, but it can be "moved".

	// "Move semantics" are a new feature introduced in C++11, 
	// that basically allows library developers to write efficient
	// assignments/constructions that avoid expensive copies, or,
	// as with `std::unique_ptr`'s case, can have a different meaning
	// than traditional copying. Example:

	{
		std::string source{"hello!"};
		std::string target{std::move(source)};

		// After constructing target by "moving `source` into it",
		// we probably avoided a (possibly expensive) copy.

		// `source` is now in an undefined state - what we did was
		// basically rip `source`'s contents and put them into 
		// `target`. Using `source` after moving it could result
		// in errors on unexpected results.
	}

	// `std::move()` helps us to transfer ownership between
	// smart pointers. It explictly expresses the intent of "moving"
	// ownership.

	std::unique_ptr<Resource> newOwner{std::move(resPtr)};

	// After moving `resPtr`, using `resPtr` may result in unexpected
	// results. `newOwner` is now the only owner of the resource, and
	// you should use it instead of `resPtr`.

	// Continuously transferring ownership can become confusing
	// and expensive. Only transfer ownership when it's really required,
	// otherwise simply refer to the smart pointer's contents either by
	// const reference or by "raw" pointer.

	// Notice that by using `std::move()`, we can now use the `passByValue`
	// function:

	passByValue(std::move(newOwner));

	// After this call, the resource will be owned by the `passByValue` 
	// parameter called `mPtr`, and after the function finishes the
	// `std::unique_ptr` will go out-of-scope and the memory previously
	// allocated for the resource will be freed.
}

int main() { return 0; }

// Here's a very interesting and well-written article about smart pointers parameters
// by Herb Sutter. I strongly reccommend you to read it!
// http://herbsutter.com/2013/06/05/gotw-91-solution-smart-pointer-parameters/

// In the next code segment we'll take a look at shared ownership.
