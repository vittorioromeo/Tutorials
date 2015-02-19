// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// As in the previous segment, here we have our "expensive resource".
struct Resource { };

// Let's say we need to create a "game effect" class that plays an
// animation, a sound, and some particle effects.
struct NaiveGameEffect
{
	Resource animation;
	Resource backgroundTexture;
	Resource particleTexture;
	Resource sound;
};

// If `Resource` is expensive to copy, copying `NaiveGameEffect` would
// be madness! 

// There are many solutions: we could have a `ResourceManager` class
// that internally stores expensive resources by using `std::unique_ptr`
// and then gives out "raw" pointers or references to game effects.

// Or we could use shared ownership, so that game effects own the resources
// in a shared manner, and the resources are freed when there are no
// game effects using them. This is where `std::shared_ptr` comes into
// play.

struct GameEffect
{
	std::shared_ptr<Resource> animation;
	std::shared_ptr<Resource> backgroundTexture;
	std::shared_ptr<Resource> particleTexture;
	std::shared_ptr<Resource> sound;
};

// This kind of design basically says:
// * While there is at least a `GameEffect` alive, there will
//   at least be a `std::shared_ptr` alive, and memory for
//   the resource will be allocated.
// * While there are more `GameEffect` instances alive, 
//   `std::shared_ptr` keeps track of the number of alive 
//   instances. As long as there are more than zero instances,
//   memory for the resource will still be allocated.
// * If there are no more instances of `GameEffect` alive,
//   there will be no more instances of `std::shared_ptr` alive.
//   The "internal shared ownership counter" will then become 0,
//   and memory for the resource will be freed.
// Let's see an example:

int main() 
{ 
	struct TextureResource
	{
		TextureResource() 	{ std::cout << "CTOR" << std::endl; }	
		~TextureResource() 	{ std::cout << "DTOR" << std::endl; }	

		// Non-copyable (using C++11's new `= delete` feature).
		TextureResource(const TextureResource&) = delete;
		TextureResource& operator=(const TextureResource&) = delete;
	};

	// Let's acquire a texture resource:
	// "CTOR" will be printed, as we're constructing a resource.
	// Here the "internal shared ownership counter" becomes 1.
	std::shared_ptr<TextureResource> source{new TextureResource};
	

	// Here's an example of a class that uses shared ownership
	// to refer to a `TextureResource`.
	struct TexturedObject
	{
		std::shared_ptr<TextureResource> texture;
	};

	{
		// We instantiate a `TexturedObject`, and set its texture to
		// the shared pointer. Notice the lack of `std::move()`. 

		// Here the "internal shared ownership counter" becomes 2.

		TexturedObject to1; to1.texture = source;
	}

	// What happened? We went in and out-of-scope, but neither "CTOR"
	// nor "DTOR" were printed. That's because instantiating a 
	// `TexturedObject` simply increased the "internal shared ownership
	// counter" by one (it became 2), and after `to1` was destroyed it
	// went back to 1.



	// As with the previous situation, we aren't constructing/destroying
	// any resource here either:

	{
		TexturedObject to2; to2.texture = source;
		TexturedObject to3; to3.texture = source;
		TexturedObject to4; to4.texture = source;
		TexturedObject to5; to5.texture = source;
		TexturedObject to6; to6.texture = source;
	}

	// We can release ownership by using `std::shared_ptr::reset()`.
	std::shared_ptr<TextureResource> newOwner{source};
	source.reset(); 

	// Now `source` doesn't own the texture resource anymore, 
	// `newOwner` is currently the only owner.

	// We can also pass `std::shared_ptr` by value (if we want to
	// increase the "internal counter" by sharing ownership with
	// the function parameter), or by const reference, if we simply
	// want to refer to the smart pointer's contents.

	// We can also call `std::shared_ptr::get()` to get a "raw"
	// pointer to the smart pointer's contents.

	// Non-implemented examples:

	// Valid! "Ownership counter" will increase by 1 here...
	/* passByValue(newOwner); */ 
	// ...and decrease by 1 here. (Assuming the function does nothing).

	// Valid! "Ownership counter" will stay the same.
	/* passByConstRef(newOwner); */

	// As you can see, we've managed to instantiate `TexturedObject`
	// easily, avoid copying an expensive resource.

	// "CTOR" and "DTOR" will, in fact, be printed only once.

	return 0; 

	// Caution: `std::shared_ptr`, unlike `std::unique_ptr`, can introduce
	// a significant runtime overhead. Use unique pointers if you don't
	// need the shared ownership!
}

// Smart pointers are incredibly powerful, and have much more to offer.
// I suggest using the fantastic "cppreference.com" website to learn more
// about them...

// `std::unique_ptr`
// http://en.cppreference.com/w/cpp/memory/unique_ptr

// `std::shared_ptr`
// http://en.cppreference.com/w/cpp/memory/shared_ptr

// ...but don't forget that "raw" pointers and references are the best
// solution when "ownership" isn't needed!



// In this episode we've learned how C++11 solves the often tedious
// and error-prone process of dynamic memory management. Smart pointers
// should completely replace manual dynamic memory management: everywhere
// you see the `new` and `delete` keywords you should use a smart pointer,
// to avoid mistakes and improve the safety and readability of your program.

// We've barely scratched the surface though. Again, I suggest reading 
// the "cppreference.com" pages for more information. 

// Let me know what you think in the comments.


// Remember to check out:
// http://herbsutter.com/2013/06/05/gotw-91-solution-smart-pointer-parameters/


// Thank you very much for watching!
// Hope you found the tutorial interesting.

// You can fork/look at the full source code on my GitHub page:
// http://github.com/SuperV1234/

// Check out my website for more tutorials and to personally
// get in touch with me.

// http://vittorioromeo.info