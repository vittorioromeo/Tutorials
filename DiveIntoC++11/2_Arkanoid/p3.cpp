// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <cmath>
#include <chrono>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

using FrameTime = float;

// We will need to make the velocity constants a lot smaller
// to deal with the new frametime values
constexpr int windowWidth{800}, windowHeight{600};
constexpr float ballRadius{10.f}, ballVelocity{0.8f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{0.6f};
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

    // We somehow need to `scale` movements and actions
    // by the frametime - to do so, we must pass the
    // latest frametime as a parameter in the update methods
    void update(FrameTime mFT)
    {
        // Then we'll just scale our velocity by the frametime
        shape.move(velocity * mFT);

        if(left() < 0)
            velocity.x = ballVelocity;
        else if(right() > windowWidth)
            velocity.x = -ballVelocity;

        if(top() < 0)
            velocity.y = ballVelocity;
        else if(bottom() > windowHeight)
            velocity.y = -ballVelocity;
    }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float left() const noexcept { return x() - shape.getRadius(); }
    float right() const noexcept { return x() + shape.getRadius(); }
    float top() const noexcept { return y() - shape.getRadius(); }
    float bottom() const noexcept { return y() + shape.getRadius(); }
};

struct Rectangle
{
    RectangleShape shape;
    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float left() const noexcept { return x() - shape.getSize().x / 2.f; }
    float right() const noexcept { return x() + shape.getSize().x / 2.f; }
    float top() const noexcept { return y() - shape.getSize().y / 2.f; }
    float bottom() const noexcept { return y() + shape.getSize().y / 2.f; }
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

        if(Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0)
            velocity.x = -paddleVelocity;
        else if(Keyboard::isKeyPressed(Keyboard::Key::Right) &&
                right() < windowWidth)
            velocity.x = paddleVelocity;
        else
            velocity.x = 0;
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

template <class T1, class T2>
bool isIntersecting(T1& mA, T2& mB) noexcept
{
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
           mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Paddle& mPaddle, Ball& mBall) noexcept
{
    if(!isIntersecting(mPaddle, mBall)) return;

    mBall.velocity.y = -ballVelocity;
    if(mBall.x() < mPaddle.x())
        mBall.velocity.x = -ballVelocity;
    else
        mBall.velocity.x = ballVelocity;
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
            bricks.emplace_back(
                (iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));

    RenderWindow window{{windowWidth, windowHeight}, "Arkanoid - 12"};

    // Let's store the last frametime here
    FrameTime lastFt{0.f};

    // By commenting/uncommenting these lines, we can check
    // if the game behaves the same way at every framerate
    window.setFramerateLimit(240);
    // window.setFramerateLimit(120);
    // window.setFramerateLimit(60);
    // window.setFramerateLimit(30);
    // window.setFramerateLimit(15);

    while(true)
    {
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

        // Now we need to pass the latest frametime in all
        // our `update` calls
        ball.update(lastFt);
        paddle.update(lastFt);

        testCollision(paddle, ball);
        for(auto& brick : bricks) testCollision(brick, ball);
        bricks.erase(remove_if(begin(bricks), end(bricks),
                         [](const Brick& mBrick)
                         {
                             return mBrick.destroyed;
                         }),
            end(bricks));

        window.draw(ball.shape);
        window.draw(paddle.shape);
        for(auto& brick : bricks) window.draw(brick.shape);
        window.display();

        auto timePoint2(chrono::high_resolution_clock::now());
        auto elapsedTime(timePoint2 - timePoint1);
        FrameTime ft{
            chrono::duration_cast<chrono::duration<float, milli>>(elapsedTime)
                .count()};

        // Let's assign the new frametime here
        lastFt = ft;

        auto ftSeconds(ft / 1000.f);
        auto fps(1.f / ftSeconds);

        window.setTitle("FT: " + to_string(ft) + "\tFPS: " + to_string(fps));
    }

    return 0;
}

// You may have noticed that the game seems to behave in the same way
// at any framerate - however, this is not the case. At very low
// framerates something horrible can occur: velocities can be so high
// that certain collision checks get skipped, because objects completely
// jump over other objects!

// Let's say our ballVelocity constant is `0.8f`.
// If our FPS are 15, our frametime will be equal to `66`!

// Our final "movement step" will then be `66 * 0.8f`, which
// equals to `52.8f`. It is almost 3 times bigger than the
// height of our bricks! Our ball can easily skip an entire
// brick in a single movement step. Let's see how we can fix
// this in the next code segment.