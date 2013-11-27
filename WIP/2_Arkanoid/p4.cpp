// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <chrono>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

// To prevent behavior and precision from changing with
// framerate, let's instead use a constant `mFT` value, 
// and not call `update(mFT)` on every frame.
// We will divide our time in different slices:

// |............|............|............|..........
// |            |            |            |
// \-> slice    \-> slice    \-> slice    \-> slice


// And, every frame, we will call `update(mFT)` "n" times,
// where "n" is the number of slices we passed through
// with our last frametime. Example:
 
//        v-----+frametime---+--v
// |............|............|............|..........
// |            |            |            |
// \-> slice    \-> slice    \-> slice    \-> slice

// In the above example, in the time it took to process
// a single frame, we went over two different slices.
// We will then call `update(mFT)` twice in the same frame.

// No matter how big or how small or last `frametime` was,
// we will always call `update(mFT)` in fixed intervals (our
// slices). If the game goes extremely fast, we will completely
// skip the updating phase during certain frames. If the game
// goes extremely slow, we will probably need to update multiple
// times during a single frame.
// This method allows fine control over our movement precision,
// and guarantees the same behavior with any FPS.

using namespace std;
using namespace sf;

using FrameTime = float;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{0.8f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{0.6f};
constexpr float blockWidth{60.f}, blockHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};

// Let's define a step and a slice constant
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

int main() 
{
	Ball ball{windowWidth / 2, windowHeight / 2};
	Paddle paddle{windowWidth / 2, windowHeight - 50};
	vector<Brick> bricks;

	for(int iX{0}; iX < countBlocksX; ++iX)	
		for(int iY{0}; iY < countBlocksY; ++iY)		
			bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, 
								(iY + 2) * (blockHeight + 3));	

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 13"};
	
	FrameTime lastFt{0.f};
	
	// Let's define a variable to accumulate the current frametime slice.
	// If the game runs fast, it will take some frames before
	// `currentSlice >= ftSlice`.
	// If the game runs slow, it will often take a single frame for
	// `currentSlice >= ftSlice * n` where `n > 1`.
	FrameTime currentSlice{0.f}; 
	
	window.setFramerateLimit(240);
	// window.setFramerateLimit(120);
	// window.setFramerateLimit(60);
	// window.setFramerateLimit(30);
	// window.setFramerateLimit(15);

	while(true)
	{
		auto timePoint1(chrono::high_resolution_clock::now());
		
		window.clear(Color::Black);

		// Events and input should be processed every frame
		// to ensure maximum responsiveness
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

		// The update phase begins here.
		// We start by accumulating frametime into `currentSlice`.
		currentSlice += lastFt;

		// If `currentSlice` is greater or equal than `ftSlice`,
		// we update our game logic and decrease `currentSlice` by
		// `ftSlice` until `currentSlice` becomes less than `ftSlice`.
		// Basically, if `currentSlice` is three times as big as `ftSlice`,
		// we update our game logic three times.
		for(; currentSlice >= ftSlice; currentSlice -= ftSlice)
		{		
			// Important: we need to pass a fixed `ftStep` constant
			// value to our game logic update, not `lastFt`.
			ball.update(ftStep);
			paddle.update(ftStep);

			testCollision(paddle, ball);
			for(auto& brick : bricks) testCollision(brick, ball);
			bricks.erase(remove_if(begin(bricks), end(bricks), 
				[](const Brick& mBrick){ return mBrick.destroyed; }), 
				end(bricks));
		}

		// Drawing is done every frame, instead. Even if we didn't
		// update our game logic, we still need to draw things on
		// the screen.
		window.draw(ball.shape);
		window.draw(paddle.shape);
		for(auto& brick : bricks) window.draw(brick.shape);
		window.display();

		auto timePoint2(chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		FrameTime ft{chrono::duration_cast<
			chrono::duration<float, milli>>(elapsedTime).count()};
		
		lastFt = ft;
		
		auto ftSeconds(ft / 1000.f);
		auto fps(1.f / ftSeconds);

		window.setTitle("FT: " + to_string(ft) + "\tFPS: " + to_string(fps));
	}	

	return 0;
}

// This method works quite well. It has some minor drawbacks, though:
// * On a fast machine, the precision of game logic updates
//   is less than it could be. Usually it's not an issue.
// * On a slow machine, the number of updates in a single frame can 
//   become very big, making the game unplayable and delaying the 
//   drawing phase. It's a good idea to limit the number of updates
//   using a `maxLoops` constant.
// * With big time slices, objects will appear to jump around - a 
//   solution, albeit difficult in its implementation, could be interpolating
//   the objects' positions in the drawing phase, so that movement looks
//   smooth even during frames where updates didn't occur. 

// Good resources:
// * http://www.koonsolo.com/news/dewitters-gameloop/
// * http://fabiensanglard.net/timer_and_framerate/

// The code is now quite messy. Let's clean it up a bit in
// the final segment.