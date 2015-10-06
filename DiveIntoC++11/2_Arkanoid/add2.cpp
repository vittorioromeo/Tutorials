// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>

// Some "uniform initialization syntax" thoughts.

struct Example
{
    Example(int mX) { std::cout << "Constructor!" << std::endl; }
    Example& operator=(int mX)
    {
        std::cout << "Operator=!" << std::endl;
        return *this;
    }
};

int main()
{
    // Even though this looks like an assignment,
    // it's actually a declaration, therefore it
    // calls Example::Example(int),
    // printing "Constructor!".
    Example example1 = 10;

    // This is an assignment, which prints "Operator=!",
    // calling Example::operator=(int).
    example1 = 5;



    // Using "uniform initialization syntax".

    // We are obviously calling the constructor here.
    Example example2{10};

    // example2{10}; <- this will not compile.

    // We are obviously assigning here.
    example2 = 10;

    // Other benefits:
    // * It avoids the most vexing parse.
    // * It's cohesive: it uses the same syntax as aggregate
    //	 initialization and to deal with array initialization
    //	 and initializer-lists.
    // * Prevents data-loss implicit conversions.

    return 0;
}

// Another example:
struct Vector2
{
    float x, y;
};

// These two functions are equivalent.
// By using {...} syntax we can avoid repeating Vector2.
Vector2 getMyVector1() { return Vector2(5.f, 5.f); }
Vector2 getMyVector2() { return {5.f, 5.f}; }

// IMPORTANT NOTE:
// When using `auto` to let the compiler deduce the type,
// never use `{...}` syntax, as it is always deduced to
// be an `std::initializer_list`. When using `auto`,
// always use the `(...)` initialization syntax.

auto x = 5; // (fine, x is an `int`)
auto x(5);  // (fine, x is an `int`)
auto x{5};  // (misleading, x is an `std::initializer_list<int>`)
