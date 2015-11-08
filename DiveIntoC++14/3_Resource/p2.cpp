// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>

// Let's take a look at possible resource types. We'll create "fake" versions of
// commonly used resource handles and put them into a dedicated namespace for
// readability.

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
        return ptr;
    }

    // Releasing is very simple too - we'll just use the `delete` keyword.

    template <typename T>
    void free_store_delete(T* ptr)
    {
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
    void glGenBuffers(GLsizei, GLuint* ptr) { *ptr = 1; }
    void glDeleteBuffers(GLsizei, GLuint* ptr)
    {
        if(*ptr == 0)
        {
            // Do nothing.
        }
        else
        {
            // Free buffer memory.
        }
    }



    // Example: non-pointer file resource handle.
    // Some APIs may return a non-pointer object, such as an `int`, to track a
    // specific resource.

    // Let's create a fake file management API with these semantics.

    int open_file() { return 1; }
    void close_file(int id)
    {
        if(id == -1)
        {
            // Do nothing.
        }
        else
        {
            // Close file.
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

        handle_type null_handle() { return nullptr; }

        template <typename... Ts>
        handle_type acquire(Ts&&... xs)
        {
            return legacy::free_store_new<T>(FWD(xs)...);
        }

        void release(const handle_type& handle)
        {
            legacy::free_store_delete(handle);
        }
    };

    struct vbo_b
    {
        using handle_type = legacy::GLuint;

        handle_type null_handle() { return 0; }

        handle_type acquire(std::size_t n)
        {
            handle_type result;
            legacy::glGenBuffers(n, &result);
            return result;
        }

        void release(const handle_type& handle)
        {
            legacy::free_store_delete(handle);
        }
    };
}

int main() { return 0; }

// TODO:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3949.pdf