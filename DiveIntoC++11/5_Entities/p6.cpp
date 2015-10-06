// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>

// Let's assume that an entity can only contain only one instance of a
// certain component type. Let's also assume every component type has an
// ID integer number. The first component type will have ID 0, and the
// other component types will have ID 1, 2, 3, ..., N.

// With these assumptions, we can create a sequence of bits that helps us
// check whether or not an entity has a certain component type.

/*  Component bitset:
    [ 0 0 0 0 0 0 0 1 0 0 1 ]
                    |     |
                    |	  \___ Component Type #0
                    |
                    \___ Component Type #3
*/

// With a simple and efficient "bitwise and", we can check if a certain
// entity has a component.

// Also, since we assign an ID number to every component type, we can
// store components in a linear array so that we can efficiently get
// a certain component type from an entity.

/*  Component bitset:
    [ 0 0 0 0 0 0 0 1 0 0 1 ]
                    |     |
                    |	  \___ Component Type #0
                    |
                    \___ Component Type #3

    Component array:
    [0] 	= (Component Type #0)*
    [1] 	= nullptr
    [2] 	= nullptr
    [3] 	= (Component Type #3)*
    [4] 	= nullptr
    [5] 	= nullptr
    [6] 	= nullptr
    [7] 	= nullptr
    [8] 	= nullptr
    [9] 	= nullptr
    [10] 	= nullptr
    [11] 	= nullptr
*/

// Therefore, we can easily say:
//
// if(entity.hasComponent<ComponentType3>())
//     entity.getComponent<ComponentType3>().doSomething();

// Let's implement everything we just talked about.

// The first step is figuring out a way to automatically give
// component types an unique ID.

// We don't want to force our user to manually assign an unique ID
// to every component.

// We will use a very simple "template trick" that guarantees an
// unique ID every time we call a function with a specific type.

namespace CompositionArkanoid
{
    // We define a typedef for the component ID type:
    using ComponentID = std::size_t;

    inline ComponentID getUniqueComponentID() noexcept
    {
        // We store a `static` lastID variable: static means
        // that every time we call this function it will refer
        // to the same `lastID` instance.

        // Basically, calling this function returns an unique ID
        // every time.

        static ComponentID lastID{0u};
        return lastID++;
    }

    // Now, some "template magic" comes into play.
    // We create a function that returns an unique ComponentID based
    // upon the type passed.
    template <typename T>
    inline ComponentID getComponentTypeID() noexcept
    {
        // Let's try to understand what happens here...

        // Every time we call this function with a specific type `T`,
        // we are actually calling an instantiation of this template,
        // with its own unique static `typeID` variable.

        // Upon calling this function for the first time with a specific
        // type `T1`, `typeID` will be initialized with an unique ID.
        // Subsequent calls with the same type `T1` will return the
        // same ID.

        static ComponentID typeID{getUniqueComponentID()};
        return typeID;
    }

    // Before applying this code to our component-based entity
    // system, let's run some tests and see how it works.
}

// Let's define some random types:
struct TypeA
{
};
struct TypeB
{
};
struct TypeC
{
};

int main()
{
    using namespace CompositionArkanoid;

    std::cout << "TypeA: " << getComponentTypeID<TypeA>() << "\n"
              << "TypeB: " << getComponentTypeID<TypeB>() << "\n"
              << "TypeC: " << getComponentTypeID<TypeC>() << "\n"

              << "TypeA: " << getComponentTypeID<TypeA>() << "\n"
              << "TypeA: " << getComponentTypeID<TypeA>() << "\n"
              << "TypeB: " << getComponentTypeID<TypeB>() << "\n"
              << "TypeB: " << getComponentTypeID<TypeB>() << "\n"
              << "TypeC: " << getComponentTypeID<TypeC>() << "\n"
              << "TypeC: " << getComponentTypeID<TypeC>() << std::endl;

    return 0;
}

// Running this code will demonstrate that we always get the same ID
// if we call `getComponentTypeID<T>` with the same type `T`.

// We can, therefore, use the return value of `getComponentTypeID<T>`
// to set a specific bit in our component bitset.

// Let's implement everything into our component-based entity system.