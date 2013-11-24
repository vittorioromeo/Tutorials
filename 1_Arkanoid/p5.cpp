// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{8.f};

// Let's create some constants for the paddle
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{6.f};

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

// Let's create a `Paddle` class, similar to `Ball`
struct Paddle
{
	// RectangleShape is an SFML class that defines
	// a renderable rectangular shape
	RectangleShape shape;
	Vector2f velocity;

	// As with the ball, we construct the paddle with
	// arguments for initial position and pass the values
	// to the SFML `shape`
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

		// To move the paddle, we check if the user is pressing
		// the left or right arrow key: if so, we change the velocity		

		// To keep the paddle "inside the window", we change the velocity
		// only if its position is inside the window
		if(Keyboard::isKeyPressed(Keyboard::Key::Left) 
			&& left() > 0) velocity.x = -paddleVelocity;
		else if(Keyboard::isKeyPressed(Keyboard::Key::Right) 
			&& right() < windowWidth) velocity.x = paddleVelocity;

		// If the user isn't pressing anything, stop moving
		else velocity.x = 0;
	}	

	float x() 		{ return shape.getPosition().x; }
	float y() 		{ return shape.getPosition().y; }
	float left() 	{ return x() - shape.getSize().x / 2.f; }
	float right() 	{ return x() + shape.getSize().x / 2.f; }
	float top() 	{ return y() - shape.getSize().y / 2.f; }
	float bottom() 	{ return y() + shape.getSize().y / 2.f; }
};

int main() 
{
	Ball ball{windowWidth / 2, windowHeight / 2};

	// Let's create a `Paddle` instance
	Paddle paddle{windowWidth / 2, windowHeight - 50};

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 5"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		ball.update();

		// Let's update our `paddle`...
		paddle.update();

		window.draw(ball.shape);

		// ...and draw its shape on the window
 		window.draw(paddle.shape);
		window.display();
	}	

	return 0;
}