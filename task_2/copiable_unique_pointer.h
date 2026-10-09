#ifndef COPIABLE_UNIQUE_POINTER_H
#define COPIABLE_UNIQUE_POINTER_H

class some_object;

class copiable_unique_pointer
{
    public:
        explicit copiable_unique_pointer(some_object* pointer = nullptr);
        ~copiable_unique_pointer();
        copiable_unique_pointer(const copiable_unique_pointer& other);
        copiable_unique_pointer& operator=(const copiable_unique_pointer& other);
        copiable_unique_pointer(copiable_unique_pointer&& other);
        copiable_unique_pointer& operator=(copiable_unique_pointer&& other);
        some_object* get();
    private:
        some_object* m_pointer;
};

#endif
