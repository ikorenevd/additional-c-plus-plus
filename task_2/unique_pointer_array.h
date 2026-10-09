#ifndef UNIQUE_POINTER_ARRAY_H
#define UNIQUE_POINTER_ARRAY_H

class unique_pointer_array
{
    public:
        explicit unique_pointer_array(int* pointer = nullptr);
        ~unique_pointer_array();
        unique_pointer_array(const unique_pointer_array&) = delete;
        unique_pointer_array& operator=(const unique_pointer_array&) = delete;
        unique_pointer_array(unique_pointer_array&& other);
        unique_pointer_array& operator=(unique_pointer_array&& other);
        int* get();
    private:
        int* m_pointer;
};

#endif
