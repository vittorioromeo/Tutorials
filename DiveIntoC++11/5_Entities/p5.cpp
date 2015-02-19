// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

// To implement our component-based entity system, we need 
// a `Manager` class that helps us manage components and entities,
// an `Entity` class that acts as a collection of components,
// and a `Component` base class from which components will inherit.

// We will implement a component-based entity system where
// components contain not only data, but also logic. 

// This kind of system is easier to implement and it's a good
// starting point to move away from inheritance-based game design.

// In a future video, I may cover a more flexible and elegant 
// entity system where components do not contain logic, but only
// data.

// Here's a simple diagram of our design:

/*
                                          
	[ Manager ]                          /-`update()`
		 |             /-[ Component ]---|
		 \-[ Entity ]--|                 \-`draw()`
         |             \-[ Component ]    
		 |
         |             /-[ Component ]
		 \-[ Entity ]--|
                       \-[ Component ]	
*/

namespace CompositionArkanoid
{
	// Forward-declaration of the `Entity` class.
	class Entity;

	struct Component
	{
		// We begin by defining a base `Component` class.
		// Game components will inherit from this class.
		
		// We will use a pointer to store the parent entity.
		Entity* entity{nullptr};

		// Usually a game component will have:
		// * Some data
		// * Update behavior
		// * Drawing behavior

		// Therefore we define two virtual methods that
		// will be overridden by game component types.
		virtual void update(float mFT) { }
		virtual void draw() { }

		// As we'll be using this class polymorphically, it requires
		// a virtual destructor.
		virtual ~Component() { }
	};

	class Entity 
	{
		// Next, we define an Entity class. 
		// It will basically be an aggregate of components,
		// with some methods that help us update and draw
		// all of them.

		private:
			// We'll keep track of whether the entity is alive or dead
			// with a boolean and we'll store the components in a private
			// vector of `std::unique_ptr<Component>`, to allow polymorphism.
			bool alive{true};
			std::vector<std::unique_ptr<Component>> components;

		// Now we will define some public methods to update and
		// draw, to add components and to destroy the entity.

		public:
			// Updating and drawing simply consists in updating and drawing
			// all the components.
			void update(float mFT) 	{ for(auto& c : components) c->update(mFT); }
			void draw() 			{ for(auto& c : components) c->draw(); }

			// We will also define some methods to control the lifetime
			// of the entity.
			bool isAlive() const 	{ return alive; }
			void destroy() 			{ alive = false; }

			// Now, we'll define a method that allows us to add components
			// to our entity.
			// We'll take advantage of C++11 variadic templates and emplacement
			// to directly construct our components in place.
			// `T` is the component type. `TArgs` is a parameter pack of 
			// types used to construct the component.
			template<typename T, typename... TArgs> 
			T& addComponent(TArgs&&... mArgs)
			{
				// We begin by allocating the component of type `T`
				// on the heap, by forwarding the passed arguments
				// to its constructor.
				T* c(new T(std::forward<TArgs>(mArgs)...));

				// We set the component's entity to the current
				// instance.
				c->entity = this;

				// We will wrap the raw pointer into a smart one,
				// so that we can emplace it into our container and 
				// to make sure we do not leak any memory.
				std::unique_ptr<Component> uPtr{c};

				// Now we'll add the smart pointer to our container:
				// `std::move` is required, as `std::unique_ptr` cannot
				// be copied.
				components.emplace_back(std::move(uPtr));

				// ...and we will return a reference to the newly added
				// component, in case the user wants to do something
				// with it.
				return *c;
			}	
	};

	// Even if the `Entity` class may seem complex, conceptually it is
	// very simple. Just think of an entity as a container for components,
	// with syntatic sugar methods to quicky add/update/draw components.

	// If `Entity` is an aggregate of components, `Manager` is an aggregate
	// of entities. Implementation is straightforward, and resembles the 
	// previous one.

	struct Manager
	{
		private:
			std::vector<std::unique_ptr<Entity>> entities;

		public:
			void update(float mFT) 	
			{ 
				// We will start by cleaning up "dead" entities.

				entities.erase(
					std::remove_if(std::begin(entities), std::end(entities), 
					[](const std::unique_ptr<Entity>& mEntity) 
					{ 
						return !mEntity->isAlive(); 
					}), 
					std::end(entities));

				// This algorithm closely resembles the one we used in 
				// the first episode of the series to delete "destroyed"
				// blocks. Basically, we're going through all entities and
				// erasing the "dead" ones.
				// This is known as the "erase-remove idiom".

				// We are sure we won't have memory leaks because entities
				// are wrapped into smart pointers.

				for(auto& e : entities) e->update(mFT); 
			}
			void draw() { for(auto& e : entities) e->draw(); }

			Entity& addEntity()
			{				
				Entity* e(new Entity());
				std::unique_ptr<Entity> uPtr{e};
				entities.emplace_back(std::move(uPtr));
				return *e;
			}	
	};

	// Now that we implemented our small (and naive) component-based
	// entity system, let's test it before going back to our arkanoid
	// example.
}

// The following example will demonstrate how an entity can be created
// by putting togheter different components. In this case, we have
// a `CounterComponent` component which increases an internal `counter`
// float value every update, and a `KillComponent` that, after being
// constructed with a reference to a `CounterComponent`, destroys the
// parent entity when the `counter` float value reaches 100.

using namespace CompositionArkanoid;

struct CounterComponent : Component
{
	float counter;
	void update(float mFT) override 
	{ 
		counter += mFT;
		std::cout << counter << std::endl;
	}
};

struct KillComponent : Component
{
	CounterComponent& cCounter;

	KillComponent(CounterComponent& mCounterComponent) 
		: cCounter(mCounterComponent) { }

	void update(float mFT) override 
	{ 
		if(cCounter.counter >= 100) entity->destroy();
	}		
};

int main()
{
	Manager manager;

	// We create an entity and get a reference to it:
	auto& entity(manager.addEntity());

	// We create components:
	auto& cCounter(entity.addComponent<CounterComponent>());
	auto& cKill(entity.addComponent<KillComponent>(cCounter));

	// And here we simulate a game loop:
	for(auto i(0u); i < 1000; ++i) 
	{
		manager.update(1.f);
		manager.draw();
	}
}

// The above example works, but there is one major issue:
// `CounterComponent` and `KillComponent` and tightly coupled.

// We need to figure out an efficient way to check if a certain
// entity has a certain component type, and, if so, retrieve
// a reference. 

// In this way, we can avoid passing a reference in 
// `KillComponent`'s constructor, and also have a way of 
// getting/checking components at runtime.

// Let's see what we can do in the next code segment.