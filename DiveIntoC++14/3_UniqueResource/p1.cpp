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

// We're going to implement our own generic "unique resource" class, with
// semantics to `std::unique_ptr`.
// We're also going to implement a "scope guard".

// So, what's a "resource" and why do we need to "manage" it?

// From Wikipedia:
// "A resource, or system resource, is any physical or virtual component of
// limited availability within a computer system. Virtual system resources
// include files, network connections, and memory areas."

// Real-life examples include: files, sockets, heap-allocated memory locations,
// game resources (models, sounds, textures, ...).

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
    Resource()
    {
        std::cout << "Acquire.\n";
    }
    ~Resource()
    {
        std::cout << "Release.\n";
    }
};

void unique_ptr_example()
{
    // "Empty" resource handle.
    std::unique_ptr<Resource> uptr0;

    // Acquire resource.
    uptr0 = std::make_unique<Resource>();

    // Transfer ownership.
    // (The explicit `std::move` is required.)
    std::unique_ptr<Resource> uptr1{std::move(uptr0)};

    // ...
    // Release resource.
    // (The resource is automatically released.)
}

void shared_ptr_example()
{
    // "Empty" resource handle.
    std::shared_ptr<Resource> external_sptr;

    {
        // Acquire resource.
        std::shared_ptr<Resource> sptr0{std::make_shared<Resource>()};

        // Share ownership.
        std::shared_ptr<Resource> sptr1{sptr0};
        std::shared_ptr<Resource> sptr2{sptr0};
        external_sptr = sptr0;

        // ...
        // `sptr0`, `sptr1`, `sptr2` lose ownership.
        // `external_sptr` still has ownership.
    }

    // ...
    // `external_sptr` loses ownership.
    // Release resource.
    // (The resource is automatically released.)
}

// C++11 smart pointers are great and should always be used when dealing with
// memory allocation. But, how do they work? How are they implemented?

// And... could they be used for other resource types?
// We can actually define and use custom release mechanisms for standard smart
// pointers, but they're still restricted to pointer-like resource handles.

// In this tutorial we'll implement our own "unique" generic resource wrapper
// that will support arbitrary handle and resource types.

// In a future tutorial, we'll expand upon this implementation, adding "shared"
// generic resource wrappers that behave like `std::shared_ptr`.

// We'll also see a generic way of wrapping any resource in a convenient modern
// interface.

int main()
{
    unique_ptr_example();
    std::cout << "\n";
    // Prints:
    // "Acquire."
    // "Release."

    shared_ptr_example();
    std::cout << "\n";
    // Prints:
    // "Acquire."
    // "Release."

    return 0;
}

// Let's begin our journey in the next code segment!

// TODO:
// * implement scope_guard with unique resource