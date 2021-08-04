#ifndef JON_JON_REF_H
#define JON_JON_REF_H

#include <type_traits>

namespace jon::detail {
    template<class T>
    class jon_ref {
    public:
        jon_ref(T && val) : owned(std::move(val)) {}
        jon_ref(const T & val) : ref(&val) {}
        jon_ref(std::initializer_list<jon_ref> init) : owned(init) {}

        template<class ...Args, std::enable_if<std::is_constructible_v<T, Args...>, int> = 0>
        jon_ref(Args && ...args) : owned(std::forward<Args>(args)...) {}

        jon_ref(jon_ref&&) noexcept = default;

        jon_ref(const jon_ref&) = delete;
        jon_ref & operator=(const jon_ref&) = delete;
        jon_ref & operator=(jon_ref&&) = delete;

        ~jon_ref() = default;

        T get() const {
            if (ref == nullptr) {
                return std::move(owned);
            }
            return *ref;
        }

        const T & operator*() const {
            return ref ? *ref : owned;
        }

        const T * operator->() const {
            return &**this;
        }

    private:
        mutable T owned = nullptr;
        const T * ref = nullptr;
    };
}

#endif // JON_JON_REF_H
