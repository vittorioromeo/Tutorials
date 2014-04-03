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

// We will implement groups by giving a group bitset to every entity,
// and storing entity pointers in the entity manager.

namespace CompositionArkanoid
{
	struct Component;
	class Entity;

	using ComponentID = std::size_t;

	// Let's create a typedef for our group type...
	using Group = std::size_t;

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
		static_assert(std::is_base_of<Component, T>::value,
			"T must inherit from Component");

		static ComponentID typeID{Internal::getUniqueComponentID()};
		return typeID;
	}

	constexpr std::size_t maxComponents{32};
	using ComponentBitset = std::bitset<maxComponents>;
	using ComponentArray = std::array<Component*, maxComponents>;	

	// ...and one for the bitset.
	constexpr std::size_t maxGroups{32};
	using GroupBitset = std::bitset<maxGroups>;

	struct Component
	{
		Entity* entity;

		virtual void init() { }
		virtual void update(float mFT) { }
		virtual void draw() { }

		virtual ~Component() { }
	};

	class Entity 
	{
		private:
			// The entity will need a reference to its manager now.
			Manager& manager;

			bool alive{true};
			std::vector<std::unique_ptr<Component>> components;
			ComponentArray componentArray;
			ComponentBitset componentBitset;

			// Let's add a bitset to our entities.
			GroupBitset groupBitset;

		public:
			Entity(Manager& mManager) : manager(mManager) { } 

			void update(float mFT) 	{ for(auto& c : components) c->update(mFT); }
			void draw() 			{ for(auto& c : components) c->draw(); }

			bool isAlive() const 	{ return alive; }
			void destroy() 			{ alive = false; }

			template<typename T> bool hasComponent() const
			{
				return componentBitset[getComponentTypeID<T>()];
			}

			// Groups will be handled at runtime, not compile-time:
			// therefore we will pass groups as a function argument.
			bool hasGroup(Group mGroup) const noexcept 
			{ 
				return componentBitset[mGroup]; 
			}

			// To add/remove group we define some methods that alter
			// the bitset and tell the manager what we're doing,
			// so that the manager can internally store this entity 
			// in its grouped containers.
			void addGroup(Group mGroup) noexcept 
			{ 
				groupBitset[mGroup] = true; 
				manager.addToGroup(this, mGroup);
			}
			void delGroup(Group mGroup) noexcept 
			{ 
				groupBitset[mGroup] = false; 
				// We won't notify the manager that a group has been
				// removed here, as it will automatically remove 
				// entities from the "wrong" group containers during
				// refresh.
			}

			template<typename T, typename... TArgs> 
			T& addComponent(TArgs&&... mArgs)
			{
				assert(!hasComponent<T>());

				T* c(new T(std::forward<TArgs>(mArgs)...));
				c->entity = this;
				std::unique_ptr<Component> uPtr{c};
				components.emplace_back(std::move(uPtr));
				
				componentArray[getComponentTypeID<T>()] = c;
				componentBitset[getComponentTypeID<T>()] = true;
				
				c->init();
				return *c;
			}

			template<typename T> T& getComponent() const
			{
				assert(hasComponent<T>());
				auto ptr(componentArray[getComponentTypeID<T>()]);
				return *reinterpret_cast<T*>(ptr);
			}
	};

	struct Manager
	{
		private:
			std::vector<std::unique_ptr<Entity>> entities;

			// We store entities in groups by creating "group buckets" in an 
			// array. `std::vector<Entity*>` could be also replaced for 
			// `std::set<Entity*>`.
			std::array<std::vector<Entity*>, maxGroups> groupedEntities;

		public:
			void update(float mFT) 	{ for(auto& e : entities) e->update(mFT); }
			void draw() 			{ for(auto& e : entities) e->draw(); }

			// When we add a group to an entity, we just add it to the
			// correct "group bucket".
			void addToGroup(Entity* mEntity, Group mGroup)
			{
				// It would be wise to either assert that the bucket doesn't
				// already contain `mEntity`, or use a set to prevent duplicates
				// in exchange for less efficient insertion/iteration.

				groupedEntities[mGroup].emplace_back(mEntity);
			}

			// To get entities that belong to a certain group, we can simply
			// get one of the "buckets" from the array.
			std::vector<Entity*>& getEntitiesByGroup(Group mGroup)
			{
				return groupedEntities[mGroup];
			}

			void refresh()
			{
				// During refresh, we need to remove dead entities and entities
				// with incorrect groups from the buckets.
				for(auto i(0u); i < maxGroups; ++i)
				{
					auto& v(groupedEntities[i]);

					v.erase(
						std::remove_if(std::begin(v), std::end(v), 
						[](const std::unique_ptr<Entity>& mEntity) 
						{ 
							return !mEntity->isAlive() || !mEntity->hasGroup(i); 
						}), 
						std::end(v));
				}

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
				Entity* e(new Entity(*this));
				std::unique_ptr<Entity> uPtr{e};
				entities.emplace_back(std::move(uPtr));
				return *e;
			}	
	};
}

// TODO: test and example
// TODO: finish part 2

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
	entity.addComponent<CounterComponent>();
	entity.addComponent<KillComponent>();

	for(auto i(0u); i < 1000; ++i) 
	{
		manager.refresh();
		manager.update(1.f);
		manager.draw();
	}
}