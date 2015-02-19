// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <chrono>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

// This is a modern C++11 `typedef` replacement:
// we just defined an alias for `float`, called `FrameTime`
using FrameTime = float;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{8.f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{6.f};
constexpr float blockWidth{60.f}, blockHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};

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
	
	void update() 
	{ 
		shape.move(velocity); 

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

	void update() 
	{ 
		shape.move(velocity); 
		
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

// Our code has a critical issue: it is framerate dependent!
// We set the framerate to 60 to prevent frame-dependent behavior,
// but it's not a good solution at all.

// We should, instead, pass the time the last frame took to update
// in our `update()` methods, so that we can "scale" our movements
// and actions with the frame time, achieving framerate indepedent
// behavior.

// Let's begin by getting the time a frame takes to update/draw,
// using C++11's fantastic `std::chrono` library.

int main() 
{
	Ball ball{windowWidth / 2, windowHeight / 2};
	Paddle paddle{windowWidth / 2, windowHeight - 50};
	vector<Brick> bricks;

	for(int iX{0}; iX < countBlocksX; ++iX)	
		for(int iY{0}; iY < countBlocksY; ++iY)		
			bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, 
								(iY + 2) * (blockHeight + 3));	

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 11"};
	
	// Let's comment out the frame rate limit for now.
	// window.setFramerateLimit(60);
	
	while(true)
	{
		// Start of our time interval.
		auto timePoint1(chrono::high_resolution_clock::now());
		
		window.clear(Color::Black);

		Event event;
		while(window.pollEvent(event)) 
		{ 
			if(event.type == Event::Closed) 
			{
				window.close();
				break;
			}
		}

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		ball.update();
		paddle.update();
		testCollision(paddle, ball);
		for(auto& brick : bricks) testCollision(brick, ball);
		bricks.erase(remove_if(begin(bricks), end(bricks), 
			[](const Brick& mBrick){ return mBrick.destroyed; }), 
			end(bricks));

		window.draw(ball.shape);
		window.draw(paddle.shape);
		for(auto& brick : bricks) window.draw(brick.shape);
		window.display();

		// End of our time interval.
		auto timePoint2(chrono::high_resolution_clock::now());

		// Let's calculate the frame time in milliseconds, 
		// and "print it out" as the window's title.

		// Subtracting two std::chrono::time_point objects
		// returns an `std::chrono::duration` object, which 
		// represents a time period.
		auto elapsedTime(timePoint2 - timePoint1);

		// We want to get the frametime in milliseconds, so we can
		// just use the safe `std::chrono::duration_cast` function.
		// To convert the duration to a `float`, we will use `.count()`
		// at the end.
		FrameTime ft{chrono::duration_cast<
			chrono::duration<float, milli>>(elapsedTime).count()};

		// We can approximate fps by dividing 1.f by the
		// elapsed seconds, calculated converting ft 
		// to seconds (ms / 1000.f).
		auto ftSeconds(ft / 1000.f);
		auto fps(1.f / ftSeconds);

		// std::to_string is another very useful C++11 function,
		// that transforms many different types to an std::string.
		window.setTitle("FT: " + to_string(ft) + "\tFPS: " + to_string(fps));
	}	

	return 0;
}

// The game should now run insanely fast... let's see what
// we can do about it in the next code segment.