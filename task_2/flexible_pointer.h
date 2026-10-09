#ifndef FLEXIBLE_POINTER_H
#define FLEXIBLE_POINTER_H

class some_object;

class flexible_pointer
{
    public:
        explicit flexible_pointer(some_object* pointer = nullptr, bool owner = false);
        ~flexible_pointer();
        flexible_pointer(const flexible_pointer&) = delete;
        flexible_pointer& operator=(const flexible_pointer&) = delete;
        flexible_pointer(flexible_pointer&& other);
        flexible_pointer& operator=(flexible_pointer&& other);
        bool is_owner() const;
        some_object* get();
    private:
        some_object* m_pointer;
        bool m_owner;
};

#endif
