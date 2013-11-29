// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

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

	// Let's begin to dive even deeper into C++11...

	// A very important thing in C++ is const-correctness:
	// since "getter" functions are not modifying any
	// member of their caller, they should be marked as 
	// const - this allows to call them on `const&` and
	// `*const` parents.
	
	// It also expresses the intent that the function isn't
	// going to modify any member of its parent!

	// And we should also mark the functions with `noexcept`,
	// a new C++11 keyword that allows the programmer to help
	// the compiler optimize the code by marking certain functions
	// that will never throw an exception.

	// In this case, we are 100% sure that SFML's getters and 
	// additions/subtractions will not throw any exception.

	// In the unfortunate case where an exception will be thrown,
	// do not worry! The program will terminate.
	// In older standards of C++, incorrectly marking functions
	// could've caused run-time corruption and security issues,
	// but that's not the case in C++11! :)
	float x() 		const noexcept { return shape.getPosition().x; }
	float y() 		const noexcept { return shape.getPosition().y; }
	float left() 	const noexcept { return x() - shape.getRadius(); }
	float right() 	const noexcept { return x() + shape.getRadius(); }
	float top() 	const noexcept { return y() - shape.getRadius(); }
	float bottom() 	const noexcept { return y() + shape.getRadius(); }
};

// We can now refactor our code by creating a Rectangle class
// that encapsulates the common properties for `Brick` and `Paddle`.
// There is no run-time overhead when we avoid using `virtual` methods
// in hierarchies. 
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
	// I didn't talk about this earlier, but initializing
	// members in place as I did here with `destroyed` is
	// a new C++11 feature. It's an handy shortcut that
	// allows us to avoid writing constructor initialization
	// lists and helps remembering to initialize members.
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

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 10"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		// Event polling is a very important and useful SFML 
		// feature that I forgot to talk about in the previous video.
		// 1. It is, on some operating systems, required! Not using
		//    it may cause the window to freeze. 
		// 2. It allows us to react to common events, such as the
		//	  window being closed, dragged, resized, etc...
		// 3. It also allows us to get input directly from the OS,
		// 	  and immensely simplifies text entering (think about
		//    entering a name in an high score).
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
	}	

	return 0;
}