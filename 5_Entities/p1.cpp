// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>

// Welcome to "Dive into C++11", part 5.
// http://vittorioromeo.info

// In this tutorial we're gonna cover a common game-development
// topic: entity management.

// I highly reccommend watching part 1 and part 4
// before watching this video.

// Let's dive in!

// Let's say we're making an arkanoid/breakout clone.
// There are various game elements that the player interacts with:
// * The ball
// * Normal bricks
// * Some powerups
// * The paddle
// * Special bricks
// * ...

// A possible way of dealing with all these elements is creating
// a class for every element, with an `update` method and a `draw` 
// method, then use `std::vectors` of `std::unique_ptr` to manage
// their lifetime. Here's an example:

namespace VeryNaiveArkanoid
{
	// Here are the game element classes:

	struct Ball 
	{ 
		void update(float mFT) { /* ... */ } 
		void draw() { /* ... */ } 
	};

	struct NormalBrick 
	{ 
		void update(float mFT) { /* ... */ } 
		void draw() { /* ... */ } 
	};

	struct SpecialBrick 
	{ 
		void update(float mFT) { /* ... */ } 
		void draw() { /* ... */ } 
	};

	struct Paddle 
	{ 
		void update(float mFT) { /* ... */ } 
		void draw() { /* ... */ } 
	};

	struct Powerup 
	{ 
		void update(float mFT) { /* ... */ } 
		void draw() { /* ... */ } 
	};

	// And there is the "game" class itself:

	struct Game
	{
		std::vector<std::unique_ptr<Ball>> balls;
		std::vector<std::unique_ptr<NormalBrick>> normalBricks;
		std::vector<std::unique_ptr<SpecialBrick>> specialBricks;
		std::vector<std::unique_ptr<Paddle>> paddles;
		std::vector<std::unique_ptr<Powerup>> powerups;

		void update(float mFT)
		{
			for(auto& b : balls) b->update(mFT);
			for(auto& nb : normalBricks) nb->update(mFT);
			for(auto& sb : specialBricks) sb->update(mFT);
			for(auto& p : paddles) p->update(mFT);
			for(auto& p : powerups) p->update(mFT);
		}

		void draw()
		{
			for(auto& b : balls) b->draw();
			for(auto& nb : normalBricks) nb->draw();
			for(auto& sb : specialBricks) sb->draw();
			for(auto& p : paddles) p->draw();
			for(auto& p : powerups) p->draw();
		}
	};	

	// As you can probably see, this approach is very difficult
	// to maintain and expand. For `n` game element types, you need
	// `n` containers, and `n` function calls. 

	// Adding another game element would require the developer
	// to modify the game class itself. 

	// This approach is not scalable, confusing and error-prone.
}

// Let's move on, and check out a (possibly) better implementation.