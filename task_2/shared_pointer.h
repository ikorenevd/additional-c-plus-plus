#ifndef SHARED_POINTER_H
#define SHARED_POINTER_H

#include <stddef.h>

class some_object;

class shared_pointer
{
    public:
        explicit shared_pointer(some_object* pointer = nullptr);
        ~shared_pointer();
        shared_pointer(const shared_pointer& other);
        shared_pointer(shared_pointer&& other);
        shared_pointer& operator=(const shared_pointer& other);
        shared_pointer& operator=(shared_pointer&& other);
        some_object* get();
    private:
        void release();
        some_object* m_pointer;
        size_t* m_count;
};

#endif
