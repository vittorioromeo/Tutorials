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

	float x() 		{ return shape.getPosition().x; }
	float y() 		{ return shape.getPosition().y; }
	float left() 	{ return x() - shape.getRadius(); }
	float right() 	{ return x() + shape.getRadius(); }
	float top() 	{ return y() - shape.getRadius(); }
	float bottom() 	{ return y() + shape.getRadius(); }
};

struct Paddle
{
	RectangleShape shape;
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

	float x() 		{ return shape.getPosition().x; }
	float y() 		{ return shape.getPosition().y; }
	float left() 	{ return x() - shape.getSize().x / 2.f; }
	float right() 	{ return x() + shape.getSize().x / 2.f; }
	float top() 	{ return y() - shape.getSize().y / 2.f; }
	float bottom() 	{ return y() + shape.getSize().y / 2.f; }
};

struct Brick
{
	RectangleShape shape;
	bool destroyed{false};

	Brick(float mX, float mY) 
	{ 
		shape.setPosition(mX, mY);
		shape.setSize({blockWidth, blockHeight});
		shape.setFillColor(Color::Yellow);
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	}

	float x() 		{ return shape.getPosition().x; }
	float y() 		{ return shape.getPosition().y; }
	float left() 	{ return x() - shape.getSize().x / 2.f; }
	float right() 	{ return x() + shape.getSize().x / 2.f; }
	float top() 	{ return y() - shape.getSize().y / 2.f; }
	float bottom() 	{ return y() + shape.getSize().y / 2.f; }
};

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left() && mA.left() <= mB.right() &&
			mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Paddle& mPaddle, Ball& mBall)
{
	if(!isIntersecting(mPaddle, mBall)) return;

	mBall.velocity.y = -ballVelocity;
	if(mBall.x() < mPaddle.x()) mBall.velocity.x = -ballVelocity;
	else mBall.velocity.x = ballVelocity;
}

// Here's the most complex part of our game: ball-brick collision.
void testCollision(Brick& mBrick, Ball& mBall)
{
	// If there's no intersection, get out of the function.
	if(!isIntersecting(mBrick, mBall)) return;

	// Otherwise, the brick has been hit!
	mBrick.destroyed = true;

	// Let's calculate how much the ball intersects the brick
	// in every direction.
	float overlapLeft{mBall.right() - mBrick.left()};
	float overlapRight{mBrick.right() - mBall.left()};
	float overlapTop{mBall.bottom() - mBrick.top()};
	float overlapBottom{mBrick.bottom() - mBall.top()};

	// If the magnitude of the left overlap is smaller than the
	// right one we can safely assume the ball hit the brick
	// from the left.
	bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));

	// We can apply the same idea for top/bottom collisions.
	bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

	// Let's store the minimum overlaps for the X and Y axes.
	float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
	float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

	// If the magnitude of the X overlap is less than the magnitude
	// of the Y overlap, we can safely assume the ball hit the brick
	// horizontally - otherwise, the ball hit the brick vertically.

	// Then, upon our assumptions, we change either the X or Y velocity
	// of the ball, creating a "realistic" response for the collision.
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

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 8"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		ball.update();
		paddle.update();
		testCollision(paddle, ball);

		// We use another C++11 foreach loop to test collisions
		// between the ball and EVERY brick.
		for(auto& brick : bricks) testCollision(brick, ball);

		// And we use a STL algorithm to remove all `destroyed` blocks
		// from the block vector - using a cool C++11 lambda!
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