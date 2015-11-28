// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>

// Let's take a look at possible resource types. We'll create "fake" versions of
// commonly used resource handles and put them into a dedicated namespace for
// readability.

// We're gonna do a lot of "perfect forwarding" later on - let's define a macro
// that reduces the amount of required boilerplate and prevents silly mistakes.
#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

namespace legacy
{
    // Example: free-store allocated pointers.
    // The most familiar and simple type of resource handles.

    // Acquiring a free-store allocated pointer requires no special
    // instructions.

    // The passed `ptr` argument will have to be either `nullptr` or be
    // allocated using the `new` keyword.

    template <typename T>
    auto free_store_new(T* ptr)
    {
        std::cout << "free_store_new\n";
        return ptr;
    }

    // Releasing is very simple too - we'll just use the `delete` keyword.

    template <typename T>
    void free_store_delete(T* ptr)
    {
        if(ptr == nullptr)
        {
            // Do nothing.
        }
        else
        {
            std::cout << "free_store_delete\n";
        }

        delete ptr;
    }



    // Example: OpenGL's "Vertex Buffer Objects".
    // "VBOs" are used to buffer and send vertex data to the GPU.

    // They need to be acquired and released using specific OpenGL global `void`
    // functions, which mutate the passed handle pointer.

    // OpenGL provides these and many more type aliases:
    using GLsizei = std::size_t;
    using GLuint = int;

    // The real OpenGL functions have the same name as signature as these fake
    // implementations:
    void glGenBuffers(GLsizei n, GLuint* ptr)
    {
        static GLuint next_id{1};

        std::cout << "glGenBuffers(" << n << ", ptr) -> " << next_id << "\n";

        *ptr = next_id++;
    }
    void glDeleteBuffers(GLsizei n, const GLuint* ptr)
    {
        if(*ptr == 0)
        {
            // Do nothing.
        }
        else
        {
            // Free buffer memory.
            std::cout << "glDeleteBuffers(" << n << ", " << *ptr << ")\n";
        }
    }



    // Example: non-pointer file resource handle.
    // Some APIs may return a non-pointer object, such as an `int`, to track a
    // specific resource.

    // Let's create a fake file management API with these semantics.

    int open_file()
    {
        static int next_id(1);

        std::cout << "open_file() -> " << next_id << "\n";

        return next_id++;
    }
    void close_file(int id)
    {
        if(id == -1)
        {
            // Do nothing.
        }
        else
        {
            // Close file.
            std::cout << "close_file(" << id << ")\n";
        }
    }
}

// We can see that every resource API has a pattern:
// * There's a function to "acquire".
// * There's a function to "release".
// * There's an "handle type" to track the resource.
// * There's a "null handle" value that can be safely deleted multiple times.

// Let's abstract these common features into types that will be used as template
// parameters for our generic resource types.

// We'll call these types "behaviors".

namespace behavior
{
    template <typename T>
    struct free_store_b
    {
        using handle_type = T*;

        handle_type null_handle()
        {
            return nullptr;
        }

        // To avoid confusing name clashes with generic resource class
        // implementations:
        // * Behavior "acquiring" will be called `init`.
        // * Behavior "releasing" will be called `deinit`.

        template <typename... Ts>
        handle_type init(Ts&&... xs)
        {
            return legacy::free_store_new<T>(FWD(xs)...);
        }

        void deinit(const handle_type& handle)
        {
            legacy::free_store_delete(handle);
        }
    };

    // TODO: mention other ways of dealing witn n
    // * could be a template parameter
    // * could be stored in the behavior (unsafe)
    struct vbo_b
    {
        struct vbo_handle
        {
            legacy::GLuint _id;
            legacy::GLsizei _n;
        };

        using handle_type = vbo_handle;

        handle_type null_handle()
        {
            return {0, 0};
        }

        handle_type init(std::size_t n)
        {
            handle_type result;

            legacy::glGenBuffers(n, &result._id);
            result._n = n;

            return result;
        }

        void deinit(const handle_type& handle)
        {
            legacy::glDeleteBuffers(handle._n, &handle._id);
        }
    };

    struct file_b
    {
        using handle_type = int;

        handle_type null_handle()
        {
            return -1;
        }

        handle_type init()
        {
            return legacy::open_file();
        }

        void deinit(const handle_type& handle)
        {
            legacy::close_file(handle);
        }
    };
}

void simulate_unique_ownership();
void simulate_shared_ownership();

int main()
{
    simulate_unique_ownership();
    simulate_shared_ownership();
    return 0;
}

void simulate_unique_ownership()
{
    behavior::file_b b;

    // `h0` is the current unique owner.
    auto h0 = b.init();

    // ... use `h0` ...

    // `h1` is the current unique owner.
    auto h1 = h0;
    h0 = b.null_handle();

    // ... use `h1` ...

    // OK - `h0` is a null handle.
    b.deinit(h0);

    // ... use `h1` ...

    // Resource released. `h1` points to an invalid handle.
    b.deinit(h1);

    // Optional safety measure.
    h1 = b.null_handle();
}

void simulate_shared_ownership()
{
    behavior::file_b b;

    // `h0` is one the current owners.
    // [`h0`]
    auto h0 = b.init();

    // ... use `h0` ...

    // `h1` is one the current owners.
    // [`h0`, `h1`]
    auto h1 = h0;

    // ... use `h0` ...
    // ... use `h1` ...

    // `h2` is one the current owners.
    // [`h0`, `h1`, `h2`]
    auto h2 = h0;

    // ... use `h0` ...
    // ... use `h1` ...
    // ... use `h2` ...

    // `h1` does not own the resource anymore.
    // [`h0`, `h2`]
    h1 = b.null_handle();

    // ... use `h0` ...
    // ... use `h2` ...

    // `h0` does not own the resource anymore.
    // [`h2`]
    h0 = b.null_handle();

    // ... use `h2` ...

    // `h2` does not own the resource anymore.
    // []
    // No more owners - resource will be released.
    b.deinit(h2);
    h2 = b.null_handle();
}

// In the next code segment, we'll implement an "unique" ownership resource
// class, that will have the same behavior as our `simulate_unique_ownership`
// test function.