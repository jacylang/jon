#ifndef JON_JON_REF_H
#define JON_JON_REF_H

namespace jon::detail {
    template<class T>
    class jon_ref {
    public:

    private:
        mutable T ownedVal = nullptr;

    };
}

#endif // JON_JON_REF_H
