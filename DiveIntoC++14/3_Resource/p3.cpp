// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>

#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

// Let's implement a generic resource class that will have "unique ownership"
// semantics.

// That means that there will always be only a single owner for a resource.
// Ownership can be moved from one object to another, but can never be shared.

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

// We'll create the class in the `resource` namespace.
namespace resource
{
    // Forward-declaration.
    // Resource classes will take the behavior as a template parameter.
    template <typename TBehavior>
    class unique;

    // We will not store an instance of `TBehavior` inside the class - we will
    // actually inherit from it.

    // Why?

    // Most of the time, behavior classes will be stateless and won't contain
    // any field. Theorically, their size should be exactly zero.

    // However, a zero-sized member stored inside a class, will actually be
    // handled as if it were of size 1. In short, this is done to prevent
    // problems with pointer arithmetic and struct memory layout.

    // By using inheritance, we allow the compiler to perform the "empty base
    // optimization". This optimization will avoid any unnecessary memory
    // overhead, and will actually count the size of empty classes as zero.

    // More information:
    // en.cppreference.com/w/cpp/language/ebo
    // en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Empty_Base_Optimization

    // In practice, using this optimization allows us to create an `unique_ptr`
    // "clone" whose size is exactly equivalent to its stored pointer.

    template <typename TBehavior>
    class unique : TBehavior
    {
        // Let's start by defining the interface of `unique`.
        // We will follow `unique_ptr`'s interface as closely as possible.

    public:
        using behavior_type = TBehavior;
        using handle_type = typename behavior_type::handle_type;

    private:
        handle_type _handle;

        // For convenience and readability, we'll write some methods that cast
        // `*this` to `TBehavior` and return the result of the cast.
        auto& as_behavior() noexcept;
        const auto& as_behavior() const noexcept;

    public:
        // The default constructor will initialize `_handle` with a null handle.
        unique() noexcept;

        // The destructor will take care of releasing the resource.
        ~unique() noexcept;

        // We don't want to share ownership - let's prevent copies.
        unique(const unique&) = delete;
        unique& operator=(const unique&) = delete;

        // We may want to create an unique resource from an existing handle.
        // Using `explicit` will prevent implicit conversions.
        explicit unique(const handle_type& handle) noexcept;

        // To transfer ownership between `unique` instances, we will use move
        // operations.
        unique(unique&& rhs) noexcept;
        auto& operator=(unique&&) noexcept;

        // `release` will not call `TBehavior::deinit` - it will simply stop the
        // current `unique` instance from being the owner of the current handle.
        // This is done by returning the current handle, then setting it to a
        // null handle.
        auto release() noexcept;

        // `reset` will actually call `TBehavior::deinit`.
        // By default the handle will be replaced with a null one.
        // An overload to replace the handle with an existing one is provided.
        void reset() noexcept;
        void reset(const handle_type& handle) noexcept;

        // Swapping two `unique` instances will simply swap their handles.
        void swap(unique& rhs) noexcept;

        // `get` will return the current handle, without changing it.
        auto get() const noexcept;

        // An explicit `bool` conversion will allow us to conveniently check the
        // validity of the handle.
        explicit operator bool() const noexcept;

        // We're going to define equality operators and a global `swap` - since
        // they're going to access private members, we're marking them as
        // `friend`.
        friend bool operator==(const unique& lhs, const unique& rhs) noexcept;
        friend bool operator!=(const unique& lhs, const unique& rhs) noexcept;
        friend void swap(unique& lhs, unique& rhs) noexcept;
    };

    // One missing piece of the `std::unique_ptr` interface is an `std::hash`
    // specialization, which is often used in hash-based containers like
    // `std::unordered_map`.

    // We're not implementing it to make the code simpler - one possible
    // approach would be defining an `hash` function inside every behavior
    // class.

    // We can now define the previously declared `unique` methods.

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

    // Default constructor: null handle initialization.
    template <typename TBehavior>
    unique<TBehavior>::unique() noexcept : _handle{as_behavior().null_handle()}
    {
    }

    // Upon destruction, we will call `reset` - it will take of calling
    // `TBehavior::deinit`.
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


    // When move-constructing, we simply set the current handle to `rhs`'s
    // handle, then call `rhs.release()`.
    template <typename TBehavior>
    unique<TBehavior>::unique(unique&& rhs) noexcept : _handle{rhs._handle}
    {
        rhs.release();
    }

    // When move-assigning, the current instance may already be the owner of
    // some resource. Therefore, we must call `reset()` before setting the new
    // handle.
    template <typename TBehavior>
    auto& unique<TBehavior>::operator=(unique&& rhs) noexcept
    {
        reset();

        _handle = rhs._handle;
        rhs.release();

        return *this;
    }

    template <typename TBehavior>
    auto unique<TBehavior>::release() noexcept
    {
        auto temp_handle(_handle);
        _handle = as_behavior().null_handle();
        return temp_handle;
    }

    // Both overloads of `reset` will immediately call `TBehavior::deinit` and
    // replace the current handle. The implementation of `TBehavior::deinit`
    // needs to handle null handles correctly.
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
        // When implementing member `swap` functions for user-defined classes,
        // it is important to enable correct ADL lookup by `using std::swap`.

        // More information here:
        // en.cppreference.com/w/cpp/algorithm/swap
        // stackoverflow.com/questions/6380862

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

// To quickly test our `resource::unique` implementation, we'll write a
// `real_unique_ownership` function that should have the same semantics as the
// "fake" one.

// We will then compare the output and the generated assembly to detect eventual
// implementation mistakes and/or run-time overhead.

void simulate_unique_ownership();
void real_unique_ownership();

int main()
{
    simulate_unique_ownership();
    real_unique_ownership();

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

void real_unique_ownership()
{
    // `h0` is the current unique owner.
    resource::unique<behavior::file_b> h0(legacy::open_file());

    // ... use `h0` ...

    // `h1` is the current unique owner.
    auto h1 = std::move(h0);

    // ... use `h1` ...

    // OK - `h0` is a null handle.
    // (nothing to do)

    // ... use `h1` ...

    // Resource released. `h1` points to an invalid handle.
    // (nothing to do)

    // Optional safety measure.
    // (nothing to do)
}

// Good news everyone!

// The output is exactly what we expected.
// The code we have to write is a lot shorter and much much safer - there is no
// risk of forgetting to release a resource and causing a leak!

// Also, generated assembly with `-O3` is identical for both
// `simulate_unique_ownership` and `real_unique_ownership`.

// This is an amazing "cost-free abstraction".
// We achieved safety, readability and convenience... 

// ...without any run-time overhead.

// TODO: 
// In the next code segment...
