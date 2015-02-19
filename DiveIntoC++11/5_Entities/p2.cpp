// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>

// Hmm... we have a lot of different game elements, but what
// do they have in common?

// Every game element has an `update` method and a `draw` 
// method.

// Using C++'s inheritance and runtime polymorphism features,
// we can create a `GameElement` class, that will be the base 
// of our game elements.

// This will allow us to abstract away the "type" of the game
// element, and store heterogeneous game elements in a single
// container.

namespace InheritanceArkanoid
{
	// Before defining the game element classes, we need
	// to define the `GameElement` base class.

	// What are the common features of the game elements?
	// The `update` and `draw` methods. We will declare them
	// as `virtual`, so that C++ runtime polymorphism may take
	// place.

	struct GameElement
	{
		virtual void update(float mFT) { }
		virtual void draw() { }
	};

	// The `virtual` keyword is fundamental. Without it, runtime
	// polymorphism won't take place.

	// ...what's this runtime polymorphism I've been talking about?
	// Let's see a quick example before moving on.

	// ...	
