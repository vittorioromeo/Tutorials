// Copyright (c) 2013-2015 Vittorio Romeo
// http://vittorioromeo.info | vittorio.romeo@outlook.com
// License: Academic Free License ("AFL") v. 3.0
//          http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <random>
#include <memory>
#include <algorithm>

// Hello everyone!
// Today we'll learn about entity storage and handles.

// In games and many other applications it's common to
// deal with "entity"-like classes.

// What defines an "entity"? Personally, I think:
// * Stores data and/or logic
// * Tied to a specific concept (game object? UI element?)
// * Object that we need to keep track of
// * Can be either be alive or dead
// * Rarely used on its own - we usually deal with many entities

// For instance, all of our game objects can be "entities".
// All of our UI widgets can be "entities".
// In general:
// 1. We need to be able to keep track of particular instances
//    of these objects.
// 2. We need to iterate and perform actions on all of these
//    objects at once.

// What is the easiest/best way to solve the first problem?
// --------------------------------------------------------
// Allocate entities dynamically. Objects on the heap are very
// easy to keep track of. Just pass a pointer around - it will
// always point to the correct instance until the entity is
// destroyed.
// We can create an `std::vector` of dynamically allocated
// entities and not worry about pointers being invalidated.

// What is the easiest/best way to solve the second problem?
// ---------------------------------------------------------
// Store entities contiguously. Having entities laid out in memory
// in a cache-friendly way greatly improves iteration efficiency.
// Unfortunately, if we store entities contiguously, keeping track
// of them becomes hard.

// Let's see an implementation of the first approach.

// Let's use C++11 pseudo-random generators to help us in
// these examples.
std::minstd_rand rndEngine;

// This is our `Entity` class. It stores some data can perform
// some actions. Its state can either be alive or dead.
struct Entity
{
    // The current state.
    bool alive{true};

    // Example stored data.
    int health;

    // Let's construct the entity by giving it a random `health`
    // value.
    Entity() : health(10 + (rndEngine() % 50)) {}

    // Updates the entity.
    // Continuously decreases `health`. When `health` is 0,
    // alive is set to false.
    void update()
    {
        if(--health <= 0) alive = false;
    }
};

// We will manage entities in a `Manager` class.
// It will help us deal with entity creation, destruction,
// and iteration.
// The `Manager` will have two "main" user-called methods:
// * `update`: iterates over all entities and updates them.
// * `refresh`: destroys all "dead" entities and adds all "new"
//              entities to the main container.
class Manager
{
public:
    // Typedef for the internal storage type.
    using Storage = std::vector<std::unique_ptr<Entity>>;

    // Typedef for handles that keep track of entities.
    using Handle = Entity*;

private:
    // Main storage of all current entities.
    Storage entities;

    // Storage of entities waiting to be added to the main
    // storage on the next `refresh()` call.
    Storage toAdd;

public:
    // Updates all entities. Entities can "die" during
    // the update.
    void update()
    {
        for(auto& e : entities) e->update();
    }

    // Gets rid of all "dead" entities and adds the newly
    // created ones to the main storage.
    // We add entities in the `refresh()` step and not
    // instantly as the vector could be resized during
    // update, invalidating the iterators.
    void refresh()
    {
        // Destroy all "dead" entities.
        // Memory is automatically freed.
        entities.erase(std::remove_if(std::begin(entities), std::end(entities),
                           [](const auto& mE)
                           {
                               return !mE->alive;
                           }),
            std::end(entities));

        // Add all new entities and clear `toAdd`.
        for(auto& e : toAdd) entities.emplace_back(std::move(e));
        toAdd.clear();
    }

    // Create a new entity and return an `Handle` pointing to it.
    Handle create()
    {
        toAdd.emplace_back(std::make_unique<Entity>());
        return toAdd.back().get();
    }
};

// Let's try out this implementation.

int main()
{
    Manager m;

    // Create two entities and get their handles.
    auto h1(m.create());
    auto h2(m.create());

    // Add the entities to the main storage.
    m.refresh();

    while(h1 != nullptr || h2 != nullptr)
    {
        // Update...
        m.update();

        // ...entities can now be marked as "dead".
        // If so, set their specific handles to `nullptr`.
        if(h1 != nullptr && !h1->alive)
        {
            std::cout << "h1 died\n";

            // We need to invalidate the handles ourselves...
            h1 = nullptr;
        }
        if(h2 != nullptr && !h2->alive)
        {
            std::cout << "h2 died\n";
            h2 = nullptr;
        }

        // Refresh (memory can be freed).
        m.refresh();
    }
}

// This method allows us to iterate and keep track of entities.
// However, it has some huge issues:
// * It's inefficient. Entity iteration is cache-unfriendly.
// * Handles point to deleted memory when the entity is "killed".
//   We have to invalidate handles ourselves.

// Good news everyone!

// We can solve these problems with the next approach.
// Unfortunately, the implementation is not as straightforward.