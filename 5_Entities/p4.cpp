// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>

// Now that we understand the basics of runtime polymorphism,
// let's return to our arkanoid clone example.

namespace InheritanceArkanoid
{
	struct GameElement
	{
		virtual void update(float mFT) { }
		virtual void draw() { }
	};

	// Now that we have defined a base class, we can define
	// the classes that inherit from this `GameElement`,
	// as we did in the animal example.

	struct Ball : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	struct NormalBrick : GameElement
	{ 
		void update(float mFT) override { /* ... */ } 
		void draw() override { /* ... */ } 
	};

	struct SpecialBrick : GameElement
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

	// And here's the new game class:

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

// Using inheritance, you end up with a big inheritance tree
// that makes sharing data and behavior between objects very 
// difficult. Here's an example:

/*
					[ GameElement ]
					       |      
	    [ EnemyNPC ]----------------[ FriendlyNPC ]  
	    	  |							   |
	 [ EnemyArmoredNPC ]         [ FriendlyArmoredNPC ]		   
*/

// Wouldn't it be better to have something like this?

/*
	 Components:

     [ Enemy ]    [ NPC ]    [ Friendly ]    [ Armored ]


	 Entities:

     [[ Skeleton ]]	= Enemy + NPC
     [[ Paladin ]] 	= Friendly + NPC + Armored
     [[ Player ]] 	= Friendly + Armored
*/