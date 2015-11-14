// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <iostream>

#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

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

        handle_type acquire(std::size_t n)
        {
            handle_type result;

            legacy::glGenBuffers(n, &result._id);
            result._n = n;

            return result;
        }

        void release(const handle_type& handle)
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

        handle_type acquire()
        {
            return legacy::open_file();
        }

        void release(const handle_type& handle)
        {
            legacy::close_file(handle);
        }
    };
}

// Let's begin by implementing our "unique" resource class.
// TODO:

namespace resource
{
    template <typename TBehavior>
    class unique;

    // We'll make use of EBCO optimization... TODO:

    template <typename TBehavior>
    class unique : TBehavior
    {
    private:
        using behavior_type = TBehavior;
        using handle_type = typename behavior_type::handle_type;

        handle_type _handle;

        auto& as_behavior() noexcept;
        const auto& as_behavior() const noexcept;

    public:
        // Default ctor
        unique() noexcept;

        // Dtor
        ~unique() noexcept;

        // Prevent copies
        unique(const unique&) = delete;
        unique& operator=(const unique&) = delete;

        // Handle ctor
        explicit unique(const handle_type& handle) noexcept;

        // Move ctor
        unique(unique&& rhs) noexcept;

        // Move assign
        auto& operator=(unique&&) noexcept;

        // Ownership release
        auto release() noexcept;

        // Null reset
        void reset() noexcept;

        // Handle reset
        void reset(const handle_type& handle) noexcept;

        // Swap
        void swap(unique& rhs) noexcept;

        // Observe handle
        auto get() const noexcept;

        // Null handle check
        explicit operator bool() const noexcept;

        // Friend ops
        friend bool operator==(const unique& lhs, const unique& rhs) noexcept;
        friend bool operator!=(const unique& lhs, const unique& rhs) noexcept;
        friend void swap(unique& lhs, unique& rhs) noexcept;
    };

    // Mention hash specialization

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

    template <typename TBehavior>
    void unique<TBehavior>::reset() noexcept
    {
        as_behavior().release(_handle);
        _handle = as_behavior().null_handle();
    }

    template <typename TBehavior>
    void unique<TBehavior>::reset(const handle_type& handle) noexcept
    {
        as_behavior().release(_handle);
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

void simulate_unique_ownership();
void real_unique_ownership();


int main()
{
// Both gcc and clang produce same assembly, you can try on godbolt 
    simulate_unique_ownership();
    real_unique_ownership();
    return 0;
}

void simulate_unique_ownership()
{
    behavior::file_b b;

    // `h0` is the current unique owner.
    auto h0 = b.acquire();

    // ... use `h0` ...

    // `h1` is the current unique owner.
    auto h1 = h0;
    h0 = b.null_handle();

    // ... use `h1` ...

    // OK - `h0` is a null handle.
    b.release(h0);

    // ... use `h1` ...

    // Resource released. `h1` points to an invalid handle.
    b.release(h1);

    // Optional safety measure.
    h1 = b.null_handle();
}

void real_unique_ownership()
{
    using ur_type = resource::unique<behavior::file_b>;

    // `h0` is the current unique owner.
    ur_type h0(legacy::open_file());

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


// TODO: