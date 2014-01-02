// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <chrono>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

using FrameTime = float;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{0.8f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{0.6f};
constexpr float blockWidth{60.f}, blockHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};
constexpr float ftStep{1.f}, ftSlice{1.f};

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

// Let's create a class for our game.
struct Game
{
	// These members are related to the control of the game.
	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 14"};
	FrameTime lastFt{0.f}, currentSlice{0.f}; 
	bool running{false};

	// These members are game entities.
	Ball ball{windowWidth / 2, windowHeight / 2};
	Paddle paddle{windowWidth / 2, windowHeight - 50};
	vector<Brick> bricks;

	Game()
	{
		// On construction, we initialize the window and create
		// the brick wall. On a more serious implementation, it 
		// would be a good idea to have a `newGame()` method that
		// can be called at any time to restart the game.
		
		window.setFramerateLimit(240);

		for(int iX{0}; iX < countBlocksX; ++iX)	
				for(int iY{0}; iY < countBlocksY; ++iY)		
					bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, 
										(iY + 2) * (blockHeight + 3));	
	}

	void run()
	{
		// The `run()` method is used to start the game and
		// contains the game loop.

		// Instead of using `break` to stop the game, we will
		// use a boolean variable, `running`.
		running = true;

		while(running)
		{
			auto timePoint1(chrono::high_resolution_clock::now());
			
			window.clear(Color::Black);

			// It's not a bad idea to use methods to make the
			// code more organized. In this case, I've divided
			// the game loop in "input", "update" and "draw"
			// phases. It's one of many possible ways of tidying up
			// the code :) 			
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
	// Let's create a temporary variable of type `Game` and
	// run it, to start our game.
	Game{}.run(); return 0;
}

// Thanks for watching!

// You can fork/look at the full source code on my GitHub page:
// http://github.com/SuperV1234/

// Check out my website for more tutorials and to personally
// get in touch with me.

// http://vittorioromeo.info