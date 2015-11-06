// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>
#include <memory>

// Welcome to "Dive into C++14", part 3.
// http://vittorioromeo.info

// In this tutorial we'll cover a very important topic: safe resource
// management.

// So, what's a "resource" and why do we need to "manage" it?

// From Wikipedia:
// "A resource, or system resource, is any physical or virtual component of
// limited availability within a computer system. Virtual system resources
// include files, network connections, and memory areas."

// You can think of files, sockets, heap-allocated pointers, game
// textures/sounds, and much more as resources.

// All resources have something in common: they need to be "acquired" and
// "released".

// Since resources are limited, we need to carefully think about their
// acquisition and release, making sure we do not run out of resource types we
// will need in the future and that we do not forget to release resources we do
// not need anymore.

// In other words, we need to think about the "ownership" and the "lifetime" of
// a resource.

// Fortunately, thanks to modern C++ generic programming, it is possible to
// abstract common ownership/lifetime semantics for resources.

// You've already seen, in previous "Dive into C++11/14" episodes, how
// heap-allocated memory can be automatically and safely managed using
// `std::unique_ptr` and `std::shared_ptr`.

struct Resource
{
    Resource() { std::cout << "Acquire.\n"; }
    ~Resource() { std::cout << "Release.\n"; }
};

void unique_ptr_example()
{
    // "Empty" resource handle.
    std::unique_ptr<Resource> uptr0;

    // Acquire resource.
    uptr0.reset(new Resource);

    // Transfer ownership.
    std::unique_ptr<Resource> uptr1{std::move(uptr0)};

    // ...
    // Release resource.
}

void shared_ptr_example()
{
    // "Empty" resource handle.
    std::shared_ptr<Resource> external_sptr;

    {
        // Acquire resource.
        std::shared_ptr<Resource> sptr0{new Resource};

        // Share ownership.
        std::shared_ptr<Resource> sptr1{sptr0};
        std::shared_ptr<Resource> sptr2{sptr0};
        external_sptr = sptr0;

        // ...
        // `sptr0`, `sptr1`, `sptr2` lose ownership.
    }

    // ...
    // `external_sptr` loses ownership.
    // Release resource.
}

// C++11 smart pointers are great and should always be used when dealing with
// memory allocation. But, how do they work? How are they implemented?

// And... could they be used for other resource types?
// We can actually define and use custom release mechanisms for standard smart
// pointers, but they're still restricted to pointer-like resource handles.

// In this tutorial we'll implement our own `unique` and `shared` resource
// management facilities, that can support arbitrary handle and resource types.

// We'll also see a generic way of wrapping any resource in a convenient modern
// interface.

int main()
{
    unique_ptr_example();
    shared_ptr_example();
    return 0;
}

// Let's begin our journey in the next code segment!