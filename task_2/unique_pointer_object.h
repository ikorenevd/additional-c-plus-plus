#ifndef UNIQUE_POINTER_OBJECT_H
#define UNIQUE_POINTER_OBJECT_H

class some_object;

class unique_pointer_object
{
    public:
        explicit unique_pointer_object(some_object* pointer = nullptr);
        ~unique_pointer_object();
        unique_pointer_object(const unique_pointer_object&) = delete;
        unique_pointer_object& operator=(const unique_pointer_object&) = delete;
        unique_pointer_object(unique_pointer_object&& other);
        unique_pointer_object& operator=(unique_pointer_object&& other);
        some_object* get();
    private:
        some_object* m_pointer;
};

#endif
