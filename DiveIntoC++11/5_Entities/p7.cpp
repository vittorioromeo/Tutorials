// Copyright (c) 2013-2015 Vittorio Romeo
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
	 	inline ComponentID getUniqueComponentID() noexcept
		{
			static ComponentID lastID{0u};
			return lastID++;
		}
	}

	template<typename T> inline ComponentID getComponentTypeID() noexcept
	{
		// We an use a `static_assert` to make sure this function
		// is only called with types that inherit from `Component`.
		static_assert(std::is_base_of<Component, T>::value,
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

		// Let's add a virtual `init` method to our Component
		// class that will be called after the component is
		// added to an entity.
		virtual void init() { }

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

				// We can now call `Component::init()`:
				c->init();

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

			void refresh()
			{
				entities.erase(
					std::remove_if(std::begin(entities), std::end(entities), 
					[](const std::unique_ptr<Entity>& mEntity) 
					{ 
						return !mEntity->isAlive(); 
					}), 
					std::end(entities));
			}

			Entity& addEntity()
			{				
				Entity* e(new Entity());
				std::unique_ptr<Entity> uPtr{e};
				entities.emplace_back(std::move(uPtr));
				return *e;
			}	
	};
}

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
	CounterComponent* cCounter{nullptr};

	// Instead of passing the entity as a parameter in the constructor,
	// we can now override the `init()` method and get the counter
	// component with our new methods.
	void init() override 
	{ 
		cCounter = &entity->getComponent<CounterComponent>();
	}

	void update(float mFT) override 
	{ 
		if(cCounter->counter >= 100) entity->destroy();
	}		
};

int main()
{
	Manager manager;
	
	auto& entity(manager.addEntity());

	// We can now avoid getting references to the components:
	entity.addComponent<CounterComponent>();
	entity.addComponent<KillComponent>();

	for(auto i(0u); i < 1000; ++i) 
	{
		manager.refresh();
		manager.update(1.f);
		manager.draw();
	}
}

// This approach, in my opinion, is cleaner and more efficient than
// the previous one. 

// We can avoid verbose constructors where we pass a lot of parameters
// by overriding the `init()` method.

// Additionally, we can avoid the `init()` method virtual overhead
// by using macros and templates, and statically calling it without dynamic
// dispatch. As this is an often unnecessary complication, I won't cover
// it in this tutorial, but you can check the implementation out in the
// SSVEntitySystem repository. 

// For the last code segment, I'll re-implement the arkanoid clone you
// saw in the first episode of the series using this component-based
// entity system. 

// Before that, however, we will add grouping to our entity manager.