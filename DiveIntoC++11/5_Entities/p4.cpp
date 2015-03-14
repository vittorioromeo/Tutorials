// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>

// Now that we understand the basics of runtime polymorphism,
// let's return to our arkanoid clone example.

// We will define a base `GameElement` class, with a virtual
// update method and a virtual draw method. 

// Game element types will inherit from this class and override
// the virtual methods to define their own behavior.

namespace InheritanceArkanoid
{
	struct GameElement
	{
		virtual void update(float mFT) { }
		virtual void draw() { }
		virtual ~GameElement() { }
	};

	// Now that we have defined a base class, we can define
	// the classes that inherit from this `GameElement`,
	// as we did in the animal example.

	struct Ball : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	struct Brick : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	struct Paddle : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	struct Powerup : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	// In the game class we can now store game elements in 
	// the same container. Even if we'll be storing pointers
	// to `GameElement`, which is the base class, polymorphism
	// will make sure the correct overrides will be called for
	// each game element type.

	struct Game
	{
		std::vector<std::unique_ptr<GameElement>> elements;

		void update(float mFT)
		{
			for(auto& e : elements) e->update(mFT);
		}

		void draw()
		{
			for(auto& e : elements) e->draw();
		}
	};	

	// Much better! The game class doesn't care about the
	// game element types anymore. 

	// It is now much easier to implement a new game element:
	// modifying the game class is not required. Also, managing
	// interactions between elements and deleting "dead" elements
	// will be much easier with a single container.
}

// This approach, however, has a big drawback: this kind
// of design ("inheritance") does not allow "composition".

// By composition I mean begin able to create game objects 
// by putting togheter several small components.

// Using this method you will end up with a big inheritance 
// tree that makes sharing data and behavior between objects 
// very difficult. Code repetition will be an issue.
// Here's an example:

/*
					[ GameElement ]
					       |      
	    [ EnemyNPC ]----------------[ FriendlyNPC ]  
	    	  |							   |
	 [ EnemyArmoredNPC ]         [ FriendlyArmoredNPC ]		   
	    	  |							   |
  [ EnemyArmoredNPCWithGun ]  [ FriendlyArmoredNPCWithGun ]	
*/

// As you can see, even though both friendly and enemy NPCs 
// types may be armored or have a weapon, two separate 
// inheritance tree branches have to be constructed to allow
// different combinations.

// This becomes cumbersome very fast, especially when there
// can be a lot of combinations.

// Wouldn't it be better to separate common behaviors and data 
// in small components that objects could be made up of?

// Here's an example of a better design:

/*
	Components:
     	[ NPC ]    
     	[ Enemy ]    
     	[ Friendly ]    
     	[ Armored ]
     	[ WithGun ]
     	...

	Entities:
     	[[ Skeleton ]]	= Enemy + NPC
     	[[ Paladin ]] 	= Friendly + NPC + Armored + WithGun
     	[[ Player ]] 	= Friendly + Armored + WithGun
     	[[ Sniper ]] 	= Enemy + NPC + WithGun
     	...
*/

// Let's check out a possible implementation in the next
// code segment.