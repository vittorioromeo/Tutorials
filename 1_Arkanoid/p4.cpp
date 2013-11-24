#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

constexpr int windowWidth{800}, windowHeight{600};
constexpr float	ballRadius{10.f}, ballVelocity{8.f};

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
		
		// We need to keep the ball "inside the screen"

		// If it's leaving toward the left, we need to set
		// horizontal velocity to a positive value (towards the right)
		if(left() < 0) velocity.x = ballVelocity;

		// Otherwise, if it's leaving towards the right, we need to
		// set horizontal velocity to a negative value (towards the left)
		else if(right() > windowWidth) velocity.x = -ballVelocity;

		// The same idea can be applied for top/bottom collisions
		if(top() < 0) velocity.y = ballVelocity;
		else if(bottom() > windowHeight) velocity.y = -ballVelocity;
	}

	// We can also create "property" methods to easily
	// get commonly used values
	float x() 		{ return shape.getPosition().x; }
	float y() 		{ return shape.getPosition().y; }
	float left() 	{ return x() - shape.getRadius(); }
	float right() 	{ return x() + shape.getRadius(); }
	float top() 	{ return y() - shape.getRadius(); }
	float bottom() 	{ return y() + shape.getRadius(); }
};

int main() 
{
	Ball ball{windowWidth / 2, windowHeight / 2};

	RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 4"};
	window.setFramerateLimit(60);
	
	while(true)
	{
		window.clear(Color::Black);

		if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		ball.update();

		window.draw(ball.shape);
		window.display();
	}	

	return 0;
}