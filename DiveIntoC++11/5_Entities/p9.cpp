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

// We will need some additional includes for frametime handling
// and callbacks.
#include <chrono>
#include <functional>

// And we'll use SFML for gfx and input management.
#include <SFML/Graphics.hpp>

namespace CompositionArkanoid
{
	struct Component;
	class Entity;
	class Manager;

	using ComponentID = std::size_t;
	using Group = std::size_t;

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
		static_assert(std::is_base_of<Component, T>::value,
			"T must inherit from Component");

		static ComponentID typeID{Internal::getUniqueComponentID()};
		return typeID;
	}

	constexpr std::size_t maxComponents{32};
	using ComponentBitset = std::bitset<maxComponents>;
	using ComponentArray = std::array<Component*, maxComponents>;	

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
			Manager& manager;

			bool alive{true};
			std::vector<std::unique_ptr<Component>> components;
			ComponentArray componentArray;
			ComponentBitset componentBitset;

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

			bool hasGroup(Group mGroup) const noexcept 
			{ 
				return groupBitset[mGroup]; 
			}

			void addGroup(Group mGroup) noexcept;
			void delGroup(Group mGroup) noexcept 
			{ 
				groupBitset[mGroup] = false; 
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
			std::array<std::vector<Entity*>, maxGroups> groupedEntities;

		public:
			void update(float mFT) 	{ for(auto& e : entities) e->update(mFT); }
			void draw() 			{ for(auto& e : entities) e->draw(); }

			void addToGroup(Entity* mEntity, Group mGroup)
			{
				groupedEntities[mGroup].emplace_back(mEntity);
			}

			std::vector<Entity*>& getEntitiesByGroup(Group mGroup)
			{
				return groupedEntities[mGroup];
			}

			void refresh()
			{
				for(auto i(0u); i < maxGroups; ++i)
				{
					auto& v(groupedEntities[i]);

					v.erase(
						std::remove_if(std::begin(v), std::end(v), 
						[i](Entity* mEntity) 
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

	void Entity::addGroup(Group mGroup) noexcept
	{ 
		groupBitset[mGroup] = true; 
		manager.addToGroup(this, mGroup);
	}

	// Let's re-implement our arkanoid clone using components:

	using namespace std;
	using namespace sf;
	using FrameTime = float;

	constexpr int windowWidth{800}, windowHeight{600};
	constexpr float	ballRadius{10.f}, ballVelocity{0.8f};
	constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{0.6f};
	constexpr float blockWidth{60.f}, blockHeight{20.f};
	constexpr int countBlocksX{11}, countBlocksY{4};
	constexpr float ftStep{1.f}, ftSlice{1.f};

	struct Game;

	// Entities can have a position in the game world.
	struct CPosition : Component
	{
		Vector2f position;

		CPosition() = default;
		CPosition(const Vector2f& mPosition) : position{mPosition} { }

		float x() const noexcept { return position.x; }
		float y() const noexcept { return position.y; }
	};

	// Entities can have a physical body and a velocity.
	struct CPhysics : Component
	{
		CPosition* cPosition{nullptr};
		Vector2f velocity, halfSize;

		// We will use a callback to handle the "out of bounds" event.
		std::function<void(const Vector2f&)> onOutOfBounds;

		CPhysics(const Vector2f& mHalfSize) : halfSize{mHalfSize} { }

		void init() override
		{	
			// A requirement for `CPhysics` is obviously `CPosition`.
			cPosition = &entity->getComponent<CPosition>();
		}

		void update(float mFT) override
		{
			cPosition->position += velocity * mFT;

			if(onOutOfBounds == nullptr) return;

			if(left() < 0) onOutOfBounds(Vector2f{1.f, 0.f});
			else if(right() > windowWidth) onOutOfBounds(Vector2f{-1.f, 0.f});

			if(top() < 0) onOutOfBounds(Vector2f{0.f, 1.f});
			else if(bottom() > windowHeight) onOutOfBounds(Vector2f{0.f, -1.f});
		}

		float x() 		const noexcept { return cPosition->x(); }
		float y() 		const noexcept { return cPosition->y(); }
		float left() 	const noexcept { return x() - halfSize.x; }
		float right() 	const noexcept { return x() + halfSize.x; }
		float top() 	const noexcept { return y() - halfSize.y; }
		float bottom() 	const noexcept { return y() + halfSize.y; }
	};

	// An entity can have a circular shape.
	struct CCircle : Component
	{
		Game* game{nullptr};
		CPosition* cPosition{nullptr};
		CircleShape shape;
		float radius;

		CCircle(Game* mGame, float mRadius) 
			: game{mGame}, radius{mRadius} { }

		void init() override
		{	
			cPosition = &entity->getComponent<CPosition>();

			shape.setRadius(radius);
			shape.setFillColor(Color::Red);
			shape.setOrigin(radius, radius);
		}

		void update(float mFT) override
		{
			// The shape will automatically get its position from
			// `CPosition`.
			shape.setPosition(cPosition->position);
		}
		void draw() override;
	};

	// An entity can have a rectangular shape.
	struct CRectangle : Component
	{
		Game* game{nullptr};
		CPosition* cPosition{nullptr};
		RectangleShape shape;
		Vector2f size;
		
		CRectangle(Game* mGame, const Vector2f& mHalfSize) 
			: game{mGame}, size{mHalfSize * 2.f} { }
		
		void init() override
		{	
			cPosition = &entity->getComponent<CPosition>();

			shape.setSize(size);
			shape.setFillColor(Color::Red);
			shape.setOrigin(size.x / 2.f, size.y / 2.f);
		}

		void update(float mFT) override
		{
			shape.setPosition(cPosition->position);
		}
		void draw() override;
	};

	// The user-controlled paddle needs a special component to handle
	// keyboard input.
	struct CPaddleControl : Component
	{
		CPhysics* cPhysics{nullptr};

		void init() override
		{	
			cPhysics = &entity->getComponent<CPhysics>();
		}

		void update(FrameTime mFT) 
		{ 			
			if(Keyboard::isKeyPressed(Keyboard::Key::Left) && 
				cPhysics->left() > 0) cPhysics->velocity.x = -paddleVelocity;
			else if(Keyboard::isKeyPressed(Keyboard::Key::Right) && 
				cPhysics->right() < windowWidth) cPhysics->velocity.x = paddleVelocity;
			else cPhysics->velocity.x = 0;
		}	
	};

	template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB) noexcept
	{
		return mA.right() >= mB.left() && mA.left() <= mB.right() 
				&& mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
	}

	void testCollisionPB(Entity& mPaddle, Entity& mBall) noexcept
	{	
		auto& cpPaddle(mPaddle.getComponent<CPhysics>());
		auto& cpBall(mBall.getComponent<CPhysics>());

		if(!isIntersecting(cpPaddle, cpBall)) return;

		cpBall.velocity.y = -ballVelocity;
		if(cpBall.x() < cpPaddle.x()) cpBall.velocity.x = -ballVelocity;
		else cpBall.velocity.x = ballVelocity;
	}

	void testCollisionBB(Entity& mBrick, Entity& mBall) noexcept
	{
		auto& cpBrick(mBrick.getComponent<CPhysics>());
		auto& cpBall(mBall.getComponent<CPhysics>());

		if(!isIntersecting(cpBrick, cpBall)) return;
		mBrick.destroy();

		float overlapLeft{cpBall.right() - cpBrick.left()};
		float overlapRight{cpBrick.right() - cpBall.left()};
		float overlapTop{cpBall.bottom() - cpBrick.top()};
		float overlapBottom{cpBrick.bottom() - cpBall.top()};

		bool ballFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));
		bool ballFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

		float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
		float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

		if(std::abs(minOverlapX) < std::abs(minOverlapY))
			cpBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
		else
			cpBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;	
	}

	struct Game
	{
		// We'll use groups to keep track of our entities.
		enum ArkanoidGroup : std::size_t
		{
			GPaddle,
			GBrick,
			GBall
		};

		RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - Components"};
		FrameTime lastFt{0.f}, currentSlice{0.f}; 
		bool running{false};
		Manager manager;

		// Creating entities can be done through simple "factory" functions.
		Entity& createBall()
		{
			auto& entity(manager.addEntity());
			
			entity.addComponent<CPosition>(Vector2f{windowWidth / 2.f, windowHeight / 2.f});
			entity.addComponent<CPhysics>(Vector2f{ballRadius, ballRadius});
			entity.addComponent<CCircle>(this, ballRadius);

			auto& cPhysics(entity.getComponent<CPhysics>());
			cPhysics.velocity = Vector2f{-ballVelocity, -ballVelocity};
			cPhysics.onOutOfBounds = [&cPhysics](const Vector2f& mSide)
			{
				if(mSide.x != 0.f) 
					cPhysics.velocity.x = std::abs(cPhysics.velocity.x) * mSide.x;

				if(mSide.y != 0.f) 
					cPhysics.velocity.y = std::abs(cPhysics.velocity.y) * mSide.y;
			};			

			entity.addGroup(ArkanoidGroup::GBall);

			return entity;
		}

		Entity& createBrick(const Vector2f& mPosition)
		{
			Vector2f halfSize{blockWidth / 2.f, blockHeight / 2.f};
			auto& entity(manager.addEntity());
			
			entity.addComponent<CPosition>(mPosition);
			entity.addComponent<CPhysics>(halfSize);
			entity.addComponent<CRectangle>(this, halfSize);

			entity.addGroup(ArkanoidGroup::GBrick);

			return entity;
		}

		Entity& createPaddle()
		{
			Vector2f halfSize{paddleWidth / 2.f, paddleHeight / 2.f};
			auto& entity(manager.addEntity());
			
			entity.addComponent<CPosition>(Vector2f{windowWidth / 2.f, windowHeight - 60.f});
			entity.addComponent<CPhysics>(halfSize);
			entity.addComponent<CRectangle>(this, halfSize);
			entity.addComponent<CPaddleControl>();

			entity.addGroup(ArkanoidGroup::GPaddle);

			return entity;
		}

		Game()
		{	
			window.setFramerateLimit(240);

			createPaddle();
			createBall();

			for(int iX{0}; iX < countBlocksX; ++iX)	
				for(int iY{0}; iY < countBlocksY; ++iY)		
					createBrick(Vector2f{(iX + 1) * (blockWidth + 3) + 22, 
										(iY + 2) * (blockHeight + 3)});					
		}

		void run()
		{
			running = true;

			while(running)
			{
				auto timePoint1(chrono::high_resolution_clock::now());
				
				window.clear(Color::Black);

				inputPhase();
				updatePhase();
				drawPhase();		

				auto timePoint2(chrono::high_resolution_clock::now());
				auto elapsedTime(timePoint2 - timePoint1);
				FrameTime ft{chrono::duration_cast<
					chrono::duration<float, milli>>(elapsedTime).count()};
				
				lastFt = ft;
				
				auto ftSeconds(ft / 1000.f);
				auto fps(1.f / ftSeconds);

				window.setTitle("FT: " + to_string(ft) + "\tFPS: " + to_string(fps));
			}	
		}

		void inputPhase()
		{
			Event event;
			while(window.pollEvent(event)) 
			{ 
				if(event.type == Event::Closed) 
				{
					window.close();
					break;
				}
			}

			if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) running = false;
		}
		void updatePhase()
		{
			currentSlice += lastFt;
			for(; currentSlice >= ftSlice; currentSlice -= ftSlice)
			{	
				manager.refresh();
				manager.update(ftStep);

				// We get our entities by group...
				auto& paddles(manager.getEntitiesByGroup(GPaddle));
				auto& bricks(manager.getEntitiesByGroup(GBrick));
				auto& balls(manager.getEntitiesByGroup(GBall));

				// ...and perform collision tests on them.
				for(auto& b : balls)
				{
					for(auto& p : paddles)
						testCollisionPB(*p, *b);

					for(auto& br : bricks)
						testCollisionBB(*br, *b);
				}
			}
		}
		void drawPhase() { manager.draw(); window.display(); }
		void render(const Drawable& mDrawable) { window.draw(mDrawable); }
	};

	void CCircle::draw() { game->render(shape); }
	void CRectangle::draw() { game->render(shape); }
}

int main() 
{
	CompositionArkanoid::Game{}.run(); return 0;
}