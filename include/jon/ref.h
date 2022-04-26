#ifndef JON_JON_REF_H
#define JON_JON_REF_H

#include <type_traits>

namespace jacylang::detail {
    template<class T>
    class jon_ref {
    public:
        jon_ref(T && val) noexcept : owned(std::move(val)) {}
        jon_ref(const T & val) noexcept : ref(&val) {}
        jon_ref(std::initializer_list<jon_ref> init) noexcept : owned(init) {}

        template<class JCT>
        jon_ref(JCT && val) noexcept : owned(std::move(val)) {}

        template<class JCT>
        jon_ref(const JCT & val) noexcept : owned(val) {}

        template<class ...Args, std::enable_if_t<std::is_constructible<T, Args...>::value, int> = false>
        jon_ref(Args && ...args) noexcept : owned(std::forward<Args>(args)...) {}

        jon_ref(jon_ref&&) noexcept = default;
        ~jon_ref() = default;

        jon_ref(const jon_ref&) = delete;
        jon_ref & operator=(const jon_ref&) = delete;
        jon_ref & operator=(jon_ref&&) = delete;

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
