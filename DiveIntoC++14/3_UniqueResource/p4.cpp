// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>
#include <memory>

namespace legacy
{
    template <typename T>
    auto free_store_new(T* ptr)
    {
        std::cout << "free_store_new\n";
        return ptr;
    }

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



    using GLsizei = std::size_t;
    using GLuint = int;

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
            std::cout << "glDeleteBuffers(" << n << ", " << *ptr << ")\n";
        }
    }



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
            std::cout << "close_file(" << id << ")\n";
        }
    }
}

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

        handle_type init(T* ptr)
        {
            return legacy::free_store_new<T>(ptr);
        }

        void deinit(const handle_type& handle)
        {
            legacy::free_store_delete(handle);
        }
    };

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

namespace resource
{
    template <typename TBehavior>
    class unique : TBehavior
    {
    public:
        using behavior_type = TBehavior;
        using handle_type = typename behavior_type::handle_type;

    private:
        handle_type _handle;

        auto& as_behavior() noexcept;
        const auto& as_behavior() const noexcept;

    public:
        unique() noexcept;
        ~unique() noexcept;

        unique(const unique&) = delete;
        unique& operator=(const unique&) = delete;

        explicit unique(const handle_type& handle) noexcept;

        unique(unique&& rhs) noexcept;
        auto& operator=(unique&&) noexcept;

        auto release() noexcept;

        void reset() noexcept;
        void reset(const handle_type& handle) noexcept;

        void swap(unique& rhs) noexcept;

        auto get() const noexcept;

        explicit operator bool() const noexcept;

        friend bool operator==(const unique& lhs, const unique& rhs) noexcept;
        friend bool operator!=(const unique& lhs, const unique& rhs) noexcept;
        friend void swap(unique& lhs, unique& rhs) noexcept;
    };

    template <typename TBehavior>
    auto& unique<TBehavior>::as_behavior() noexcept
    {
        return static_cast<behavior_type&>(*this);
    }

    template <typename TBehavior>
    const auto& unique<TBehavior>::as_behavior() const noexcept
    {
        return static_cast<const behavior_type&>(*this);
    }

    template <typename TBehavior>
    unique<TBehavior>::unique() noexcept : _handle{as_behavior().null_handle()}
    {
    }

    template <typename TBehavior>
    unique<TBehavior>::~unique() noexcept
    {
        reset();
    }

    template <typename TBehavior>
    unique<TBehavior>::unique(const handle_type& handle) noexcept
        : _handle{handle}
    {
    }

    template <typename TBehavior>
    unique<TBehavior>::unique(unique&& rhs) noexcept : _handle{rhs._handle}
    {
        rhs.release();
    }

    template <typename TBehavior>
    auto& unique<TBehavior>::operator=(unique&& rhs) noexcept
    {
        reset(rhs.release());
        return *this;
    }

    template <typename TBehavior>
    auto unique<TBehavior>::release() noexcept
    {
        auto temp_handle(_handle);
        _handle = as_behavior().null_handle();
        return temp_handle;
    }

    template <typename TBehavior>
    void unique<TBehavior>::reset() noexcept
    {
        as_behavior().deinit(_handle);
        _handle = as_behavior().null_handle();
    }

    template <typename TBehavior>
    void unique<TBehavior>::reset(const handle_type& handle) noexcept
    {
        as_behavior().deinit(_handle);
        _handle = handle;
    }

    template <typename TBehavior>
    void unique<TBehavior>::swap(unique& rhs) noexcept
    {
        using std::swap;
        swap(_handle, rhs._handle);
    }

    template <typename TBehavior>
    auto unique<TBehavior>::get() const noexcept
    {
        return _handle;
    }

    template <typename TBehavior>
    unique<TBehavior>::operator bool() const noexcept
    {
        return _handle != as_behavior().null_handle();
    }

    template <typename TBehavior>
    bool operator==(
        const unique<TBehavior>& lhs, const unique<TBehavior>& rhs) noexcept
    {
        return lhs._handle == rhs._handle;
    }

    template <typename TBehavior>
    bool operator!=(
        const unique<TBehavior>& lhs, const unique<TBehavior>& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template <typename TBehavior>
    void swap(unique<TBehavior>& lhs, unique<TBehavior>& rhs) noexcept
    {
        lhs.swap(rhs);
    }
}

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

void example_free_store_ptr_vs_unique_ptr()
{
    using my_behavior = behavior::free_store_b<Resource>;
    using my_resource = resource::unique<my_behavior>;

    static_assert(                                                // .
        sizeof(my_resource) == sizeof(std::unique_ptr<Resource>), // .
        "");

    {
        my_resource r0{my_behavior{}.init(new Resource)};
        auto r1(std::move(r0));
    }
    // Prints:
    // "Acquire."
    // "free_store_new"
    // "free_store_delete"
    // "Release."


    {
        std::unique_ptr<Resource> r0{new Resource};
        auto r1(std::move(r0));
    }
    // Prints:
    // "Acquire."
    // "Release."

    // Possible improvements to our interface:
    // * Add `make_unique_resource<TBehavior>(...)` variadic function.
    // * Use SFINAE to enable "pointer propagation" `*` and `->` operators for
    // pointer-like types.

    // Again, the generated assembly for both "g++" and "clang++" (using `-O3`)
    // was identical.
}

void example_vbo()
{
    using my_behavior = behavior::vbo_b;
    using my_resource = resource::unique<my_behavior>;

    {
        my_resource r0{my_behavior{}.init(4)};
        auto r1(std::move(r0));
    }
    // Prints:
    // "glGenBuffers(4, ptr) -> 1"
    // "glDeleteBuffers(4, 1)"
}

// There is another way of dealing with "uniqueness semantics" in scopes.
// Ever heard of "scope guards"?

// They're a feature in some languages, such as "D", that allows users to
// conveniently write a piece of code anywhere in a scope, which will only be
// executed at the end of the scope.

// All "scope guards" will be executed in reverse order (guards that appear
// later in the scope will be executed first).

// Sounds familiar?
// We can implement "scope guards" as a resource.

struct scope_guard_behavior
{
    // Our handle type will be a simple pointer-to-function.
    // The target function has to return `void` and has to take zero arguments.
    using handle_type = void (*)();

    // A more efficient implementation would avoid the function pointer overhead
    // by passing the function to the handle type as a template parameter.

    // Check out N4189 for a "scope guard" and a generic "RAII wrapper"
    // proposal:
    // open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189.pdf

    handle_type null_handle()
    {
        return handle_type{};
    }

    handle_type init(void (*fn_ptr)())
    {
        return fn_ptr;
    }

    void deinit(const handle_type& handle)
    {
        (*handle)();
    }
};

template <typename TFunction>
auto make_scope_guard(TFunction f)
{
    using my_behavior = scope_guard_behavior;
    using my_resource = resource::unique<my_behavior>;
    return my_resource{my_behavior{}.init(f)};
}

void example_scope_guard_0()
{
    auto s0 = make_scope_guard([]
        {
            std::cout << "A\n";
        });

    std::cout << sizeof(s0) << "\n";

    auto s1 = make_scope_guard([]
        {
            std::cout << "B\n";
        });

    auto s2 = make_scope_guard([]
        {
            std::cout << "C\n";
        });

    // Prints:
    // "C"
    // "B"
    // "A"

    // The calls are executed backwards, as expected.
}

void example_scope_guard_1()
{
    // By explicitly writing scopes, we can change the order of the calls.

    {
        {
            auto s0 = make_scope_guard([]
                {
                    std::cout << "A\n";
                });
        }

        auto s1 = make_scope_guard([]
            {
                std::cout << "B\n";
            });
    }

    auto s2 = make_scope_guard([]
        {
            std::cout << "C\n";
        });

    // Prints:
    // "A"
    // "B"
    // "C"
}

// We can actually avoid manually specifying an unique name every time and an
// empty lambda capture list by using a macro.

#define DELAYED_CAT(a, b) a##b
#define CAT(a, b) DELAYED_CAT(a, b)

// The `SCOPE_GUARD` macro takes a variadic amount of tokens (our function
// body), and puts them inside a `make_scope_guard` call, adding the empty
// capture list. The result of the call is then assigned to a variable with an
// unique name, computed using a very unlikely name prefix and the current line
// number.

#define SCOPE_GUARD(...) \
    auto CAT(_strange_var_name_, __LINE__) = make_scope_guard([] __VA_ARGS__)

// The following examples are much easier to read and do not require the user to
// invent an unique name for every "scope guard".

void example_pretty_scope_guard_0()
{
    SCOPE_GUARD(
        {
            std::cout << "A\n";
        });

    SCOPE_GUARD(
        {
            std::cout << "B\n";
        });

    SCOPE_GUARD(
        {
            std::cout << "C\n";
        });

    // Prints:
    // "C"
    // "B"
    // "A"
}

void example_pretty_scope_guard_1()
{
    {
        {
            SCOPE_GUARD(
                {
                    std::cout << "A\n";
                });
        }

        SCOPE_GUARD(
            {
                std::cout << "B\n";
            });
    }

    SCOPE_GUARD(
        {
            std::cout << "C\n";
        });

    // Prints:
    // "A"
    // "B"
    // "C"
}

int main()
{
    example_free_store_ptr_vs_unique_ptr();
    std::cout << "\n";

    example_vbo();
    std::cout << "\n";

    example_scope_guard_0();
    std::cout << "\n";

    example_scope_guard_1();
    std::cout << "\n";

    example_pretty_scope_guard_0();
    std::cout << "\n";

    example_pretty_scope_guard_1();
    std::cout << "\n";

    return 0;
}

// Thank you very much for watching this video!
// I hope you found the covered topics interesting.

// You can fork/look at the full source code on GitHub:
// http://github.com/SuperV1234/Tutorials

// Check out my website for more tutorials/projects and to personally get in
// touch with me.

// http://vittorioromeo.info