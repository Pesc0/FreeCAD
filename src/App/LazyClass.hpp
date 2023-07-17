#pragma once

#include <memory>

// The idea is similar to this: https://github.com/bitwizeshift/Lazy
// but insread of deferring object construction we are deferring copy.
// See also similar lazy evaluation: https://stackoverflow.com/a/17720749
//
// The object T wrapped by this class is referenced through a shared_ptr, 
// so that copies of Lazy just increment the refcount instead of preforming
// a full copy of the wrapped object. In case the object T needs to get modified a full copy  
// is performed, to avoid modifying the other references owned by the shared_ptr.
//
// The intention is to replicate the behavior of QByteArray, but for any generic type T.
// https://doc.qt.io/qt-6/qbytearray.html
//
// My hope was that the compiler was smart enough to deduce when it could call
// const methods and when it had no choice but to call the non-const variant.
// It is not. To work around this all access operators have been commented out, 
// leaving just two functions: asConst() and asMutable(). This is way more verbose
// to type, but also makes the behavior of the class extremely explicit and transparent. 
// There's no guessing when a copy will be made.

template <typename T>
class Lazy {
public:
    Lazy() : handle(std::make_shared<T>()) {};

    template<typename... Args>
    Lazy(Args&&... args) : handle(std::make_shared<T>(std::forward<Args>(args)...)) {}

    template<typename... Args>
    Lazy& operator=(Args&&... args) 
    {
        handle = std::make_shared<T>(std::forward<Args>(args)...);
        return *this;
    }

    Lazy(const Lazy& other) = default;
    Lazy& operator=(const Lazy& other) = default;

    /* Non-const copy constructor needed otherwise arg-forwarding constructor gets selected */
    Lazy(Lazy& other) = default;
    Lazy& operator=(Lazy& other) = default;

    Lazy(Lazy&& other) noexcept = default;
    Lazy& operator=(Lazy&& other) noexcept = default;

    ~Lazy() = default;

    void ensureUnshared()
    {
        if (handle.use_count() != 1)
        {
            handle = std::make_shared<T>(*handle);
        }
    }

    bool isUnshared() const { return handle.use_count() == 1; }

    // Access the wrapped object in a non-modifying fashion. No copy will be made (unless
    // a local copy was already made beforehand)
    const T& asConst() const { return *handle; }

    // Access the wrapped object to modify it. Ensures that a local copy has been
    // made beforehand, so that the other references owned do not get modified.
    T& asMutable() { ensureUnshared(); return *handle; }

/*
    const T& get() const { return *handle; }
    T& get() { ensureUnshared(); return *handle; }

    const T& operator*() const { return *handle; }
    T& operator*() { ensureUnshared(); return *handle; }

    const T* operator->() const { return &(*handle); }
    T* operator->() { ensureUnshared(); return &(*handle); }

    operator const T&() const { return *handle; }
    operator T&() { ensureUnshared(); return *handle; }
*/

private:
    std::shared_ptr<T> handle;
};
