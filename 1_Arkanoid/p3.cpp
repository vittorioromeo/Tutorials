// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

constexpr int windowWidth{800}, windowHeight{600};

// Let's create some constants for the Ball class.
constexpr float	ballRadius{10.f}, ballVelocity{8.f};

struct Ball
{
	CircleShape shape;

	// 2D vector that stores the Ball's velocity.
	Vector2f velocity{-ballVelocity, -ballVelocity};

	Ball(float mX, float mY) 
	{
		shape.setPosition(mX, mY);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}	

	// Let's "update" the ball: move its shape
	// by the current velocity.
	void update() { shape.move(velocity); }
};

int main() 
{
	Ball ball{windowWidth / 2, windowHeight / 2};

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 3"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		// Every loop iteration, we need to update the ball.
		ball.update();

		window.draw(ball.shape);
		window.display();
	}	

	return 0;
}