// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>
#include <cassert>
#include <type_traits>

namespace CompositionArkanoid
{
	struct Component;
	class Entity;

	using ComponentID = std::size_t;

	// Let's hide implementation details into an "Internal" namespace:
	namespace Internal
	{
	 	ComponentID getUniqueComponentID() noexcept
		{
			static ComponentID lastID{0u};
			return lastID++;
		}
	}

	template<typename T> ComponentID getComponentTypeID() noexcept
	{
		// We an use a `static_assert` to make sure this function
		// is only called with types that inherit from `Component`.
		static_assert(std::is_base_of<Component, T>::value
			, 
			"T must inherit from Component");

		static ComponentID typeID{Internal::getUniqueComponentID()};
		return typeID;
	}

	// Let's define a maximum number of components:
	constexpr std::size_t maxComponents{32};

	// Let's typedef an `std::bitset` for our components:
	using ComponentBitset = std::bitset<maxComponents>;

	// And let's also typedef an `std::array` for them:
	using ComponentArray = std::array<Component*, maxComponents>;	

	struct Component
	{
		Entity* entity;

		virtual void update(float mFT) { }
		virtual void draw() { }

		virtual ~Component() { }
	};

	class Entity 
	{
		private:
			bool alive{true};
			std::vector<std::unique_ptr<Component>> components;

			// Let's add an array to quickly get a component with 
			// a specific ID, and a bitset to check the existance of
			// a component with a specific ID.
			ComponentArray componentArray;
			ComponentBitset componentBitset;

		public:
			void update(float mFT) 	{ for(auto& c : components) c->update(mFT); }
			void draw() 			{ for(auto& c : components) c->draw(); }

			bool isAlive() const 	{ return alive; }
			void destroy() 			{ alive = false; }

			// To check if this entity has a component, we simply
			// query the bitset.
			template<typename T> bool hasComponent() const
			{
				return componentBitset[getComponentTypeID<T>()];
			}

			template<typename T, typename... TArgs> 
			T& addComponent(TArgs&&... mArgs)
			{
				// Before adding a component, we make sure it doesn't
				// already exist by using an assertion.
				assert(!hasComponent<T>());

				T* c(new T(std::forward<TArgs>(mArgs)...));
				c->entity = this;
				std::unique_ptr<Component> uPtr{c};
				components.emplace_back(std::move(uPtr));

				// When we add a component of type `T`, we add it to 
				// the bitset and to the array.
				componentArray[getComponentTypeID<T>()] = c;
				componentBitset[getComponentTypeID<T>()] = true;

				return *c;
			}

			template<typename T> T& getComponent() const
			{
				// To retrieve a specific component, we get it from
				// the array. We'll also assert its existance.

				assert(hasComponent<T>());
				auto ptr(componentArray[getComponentTypeID<T>()]);
				return *reinterpret_cast<T*>(ptr);
			}
	};

	struct Manager
	{
		private:
			std::vector<std::unique_ptr<Entity>> entities;

		public:
			void update(float mFT) 	{ for(auto& e : entities) e->update(mFT); }
			void draw() 			{ for(auto& e : entities) e->draw(); }

			// We will create a `refresh` method that uses STL algorithms to
			// clean up "dead" entities.
			void refresh()
			{
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

				// We are sure we won't have memory leaks because entities
				// are wrapped into smart pointers.
			}

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
		manager.refresh();
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