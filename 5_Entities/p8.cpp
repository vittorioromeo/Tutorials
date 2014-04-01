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
#include <chrono>
#include <functional>
#include <SFML/Graphics.hpp>

struct Component;
class Entity;

using ComponentID = std::size_t;

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
		bool alive{true};
		std::vector<std::unique_ptr<Component>> components;
		ComponentArray componentArray;
		ComponentBitset componentBitset;

	public:
		void update(float mFT) 	{ for(auto& c : components) c->update(mFT); }
		void draw() 			{ for(auto& c : components) c->draw(); }

		bool isAlive() const 	{ return alive; }
		void destroy() 			{ alive = false; }

		template<typename T> bool hasComponent() const
		{
			return componentBitset[getComponentTypeID<T>()];
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

// TODO:
// complete

using namespace std;
using namespace sf;
using FrameTime = float;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{0.8f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{0.6f};
constexpr float blockWidth{60.f}, blockHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};
constexpr float ftStep{1.f}, ftSlice{1.f};

struct CPosition : Component
{
	Vector2f position;
};

struct CPhysics : Component
{
	CPosition* cPosition{nullptr};
	Vector2f velocity;

	std::function<void(float)> onOutOfBounds;
	std::function<void(CPhysics&)> onCollision;

	void init() override
	{	
		cPosition = &entity->getComponent<CPosition>();
	}

	void update(float mFT) override
	{
		cPosition.position += velocity * mFT;

		if(left() < 0) velocity.x = ballVelocity;
		else if(right() > windowWidth) velocity.x = -ballVelocity;

		if(top() < 0) velocity.y = ballVelocity;
		else if(bottom() > windowHeight) velocity.y = -ballVelocity;
	}
};

struct CRenderCircle
{
	CPosition* cPosition{nullptr};
	CircleShape shape;
	float radius;

	CRenderCircle(float mRadius) : radius{mRadius} { }

	void init() override
	{	
		cPosition = &entity->getComponent<CPosition>();

		shape.setRadius(radius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(radius, radius);
	}

	void update(float mFT) override
	{
		shape.setPosition(cPosition.position);
	}

	void draw() override
	{

	}
};

struct CRenderRectangle
{
	CPosition* cPosition{nullptr};
	RectangleShape shape;
	Vector2f size;
	
	CRenderRectangle(const Vector2f& mSize) : size{mSize} { }

	void init() override
	{	
		cPosition = &entity->getComponent<CPosition>();

		shape.setSize(size);
		shape.setFillColor(Color::Red);
		shape.setOrigin(size.x / 2.f, size.y / 2.f);
	}

	void update(float mFT) override
	{
		shape.setPosition(cPosition.position);
	}

	void draw() override
	{

	}
};

struct Ball
{
	CircleShape shape;
	Vector2f velocity{-ballVelocity, -ballVelocity};

	Ball(float mX, float mY) 
	{
		shape.setPosition(mX, mY);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}	

	void update(FrameTime mFT) 
	{ 
		shape.move(velocity * mFT); 

		if(left() < 0) velocity.x = ballVelocity;
		else if(right() > windowWidth) velocity.x = -ballVelocity;

		if(top() < 0) velocity.y = ballVelocity;
		else if(bottom() > windowHeight) velocity.y = -ballVelocity;
	}

	float x() 		const noexcept { return shape.getPosition().x; }
	float y() 		const noexcept { return shape.getPosition().y; }
	float left() 	const noexcept { return x() - shape.getRadius(); }
	float right() 	const noexcept { return x() + shape.getRadius(); }
	float top() 	const noexcept { return y() - shape.getRadius(); }
	float bottom() 	const noexcept { return y() + shape.getRadius(); }
};

struct Rectangle
{
	RectangleShape shape;
	float x() 		const noexcept { return shape.getPosition().x; }
	float y() 		const noexcept { return shape.getPosition().y; }
	float left() 	const noexcept { return x() - shape.getSize().x / 2.f; }
	float right() 	const noexcept { return x() + shape.getSize().x / 2.f; }
	float top() 	const noexcept { return y() - shape.getSize().y / 2.f; }
	float bottom() 	const noexcept { return y() + shape.getSize().y / 2.f; }
};

struct Paddle : public Rectangle
{
	Vector2f velocity;

	Paddle(float mX, float mY) 
	{ 
		shape.setPosition(mX, mY);
		shape.setSize({paddleWidth, paddleHeight});
		shape.setFillColor(Color::Red);
		shape.setOrigin(paddleWidth / 2.f, paddleHeight / 2.f);
	}

	void update(FrameTime mFT) 
	{ 
		shape.move(velocity * mFT); 
		
		if(Keyboard::isKeyPressed(Keyboard::Key::Left) && 
			left() > 0) velocity.x = -paddleVelocity;
		else if(Keyboard::isKeyPressed(Keyboard::Key::Right) && 
			right() < windowWidth) velocity.x = paddleVelocity;
		else velocity.x = 0;
	}	
};

struct Brick : public Rectangle
{
	bool destroyed{false};

	Brick(float mX, float mY) 
	{ 
		shape.setPosition(mX, mY);
		shape.setSize({blockWidth, blockHeight});
		shape.setFillColor(Color::Yellow);
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	}
};

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB) noexcept
{
	return mA.right() >= mB.left() && mA.left() <= mB.right() 
			&& mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Paddle& mPaddle, Ball& mBall) noexcept
{
	if(!isIntersecting(mPaddle, mBall)) return;

	mBall.velocity.y = -ballVelocity;
	if(mBall.x() < mPaddle.x()) mBall.velocity.x = -ballVelocity;
	else mBall.velocity.x = ballVelocity;
}

void testCollision(Brick& mBrick, Ball& mBall) noexcept
{
	if(!isIntersecting(mBrick, mBall)) return;
	mBrick.destroyed = true;

	float overlapLeft{mBall.right() - mBrick.left()};
	float overlapRight{mBrick.right() - mBall.left()};
	float overlapTop{mBall.bottom() - mBrick.top()};
	float overlapBottom{mBrick.bottom() - mBall.top()};

	bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));
	bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

	float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
	float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

	if(abs(minOverlapX) < abs(minOverlapY))
		mBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
	else
		mBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;	
}

struct Game
{
	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 14"};
	FrameTime lastFt{0.f}, currentSlice{0.f}; 
	bool running{false};

	Ball ball{windowWidth / 2, windowHeight / 2};
	Paddle paddle{windowWidth / 2, windowHeight - 50};
	vector<Brick> bricks;

	Game()
	{	
		window.setFramerateLimit(240);

		for(int iX{0}; iX < countBlocksX; ++iX)	
				for(int iY{0}; iY < countBlocksY; ++iY)		
					bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, 
										(iY + 2) * (blockHeight + 3));	
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

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) 
			running = false;
	}

	void updatePhase()
	{
		currentSlice += lastFt;
		for(; currentSlice >= ftSlice; currentSlice -= ftSlice)
		{		
			ball.update(ftStep);
			paddle.update(ftStep);

			testCollision(paddle, ball);
			for(auto& brick : bricks) testCollision(brick, ball);
			bricks.erase(remove_if(begin(bricks), end(bricks), 
				[](const Brick& mBrick){ return mBrick.destroyed; }), 
				end(bricks));
		}
	}

	void drawPhase()
	{
		window.draw(ball.shape);
		window.draw(paddle.shape);
		for(auto& brick : bricks) window.draw(brick.shape);
		window.display();
	}
};

int main() 
{
	Game{}.run(); return 0;
}
