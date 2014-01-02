// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f};

// Let's create a class for our Ball.
// `struct` == `class` in C++.
struct Ball
{
	// CircleShape is an SFML class that 
	// defines a renderable circle.
	CircleShape shape;

	// Let's create the Ball constructor.
	// (argument mX -> starting x coordinate)
	// (argument mY -> starting y coordinate)
	Ball(float mX, float mY) 
	{
		// Apply position, radius, color and origin 		 
		// to the CircleShape `shape`.
		shape.setPosition(mX, mY);		
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}	
};

int main() 
{
	// Let's create an instance of `Ball`
	// positioned at the center of the window
	Ball ball{windowWidth / 2, windowHeight / 2};

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 2"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		// Let's render the Ball instance on the window
		window.draw(ball.shape);
		window.display();
	}	

	return 0;
}
