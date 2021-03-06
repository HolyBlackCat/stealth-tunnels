#pragma once

#include <cstddef>
#include <type_traits>

#include "meta/common.h"
#include "meta/type_info.h"
#include "program/errors.h"
#include "program/platform.h"

#ifndef IMP_TYPE_HASH_CHECKS_ENABLED
#  if IMP_PLATFORM_IS(prod)
#    define IMP_TYPE_HASH_CHECKS_ENABLED 0
#  else
#    define IMP_TYPE_HASH_CHECKS_ENABLED 1
#  endif
#endif

namespace Meta
{
    // Lets you augument classes with a hidden "type hash" field.
    // You can check it at runtime to make sure a pointer/reference you have really points to a valid instance.

    namespace impl
    {
        // A class wrapper to make friend declarations easier.
        struct TypeId
        {
            ~TypeId() = default;

            // Affects class hashes.
            // If you feel like you got a hash collision (invalid casts are not being detected), try changing this value.
            static constexpr Meta::hash_t hash_seed = 123;

            class Support
            {
                constexpr Support(Meta::hash_t hash) : hash(hash) {}

                volatile Meta::hash_t hash;
              public:
                const volatile Meta::hash_t &Get() const {return hash;}

                // cvref-qualifiers on T are ignored.
                template <typename T>
                [[nodiscard]] static Support Make()
                {
                    return Support(Meta::TypeHash<std::remove_cvref_t<T>>(hash_seed));
                }

                Support(const Support &other) : hash(other.hash) {}
                Support &operator=(const Support &) {return *this;} // This does nothing intentionally.

                ~Support()
                {
                    hash = 0;
                }
            };

            #if IMP_TYPE_HASH_CHECKS_ENABLED == 1
            // Can't use `Meta::is_detected` because the variable is not necessarily public.
            // Note that we not only check the validity of the member pointer, but also its type.
            // This ensures that inherited class members don't count.
            template <typename T, typename = void> struct stores_hash : std::false_type {};
            template <typename T> struct stores_hash<T,
                std::enable_if_t<std::is_same_v<Support T::*, decltype(&T::_type_hash_storage_)>>> : std::true_type {};
            #endif

            template <typename T> [[nodiscard]] static constexpr bool CheckHash(const T &object)
            {
                #if IMP_TYPE_HASH_CHECKS_ENABLED == 1
                static_assert(std::is_class_v<T>, "The parameter must have a class type.");
                static_assert(impl::TypeId::stores_hash<T>::value, "This type doesn't store its hash. Add `IMP_STORE_TYPE_HASH` to the class body.");
                return object._type_hash_storage_.Get() == TypeHash<T>(hash_seed);
                #else
                (void)object;
                return true;
                #endif
            }
        };
    }

    // In release builds does nothing (if `IMP_TYPE_HASH_CHECKS_ENABLED == 1` is not defined).
    // Otherwise, triggers an assertion if `object` doesn't refer to an actual object of type `T`.
    // If it's not possible to check this object (class lacks `IMP_STORE_TYPE_HASH` in its body), a `static_assert` is triggered.
    template <typename T>
    void AssertTypeHash(const T &object)
    {
        (void)object;
        ASSERT(impl::TypeId::CheckHash(object), "Type hash check failed.");
    }
}

#if IMP_TYPE_HASH_CHECKS_ENABLED == 1
// Add this to a class definition to enable member downcast support for it.
// In debug builds this increases class size by 4.
#define IMP_STORE_TYPE_HASH \
    friend ::Meta::impl::TypeId; \
    ::Meta::impl::TypeId::Support _type_hash_storage_ = ::Meta::impl::TypeId::Support::Make<decltype(*this)>();
#else
#define IMP_STORE_TYPE_HASH
#endif
