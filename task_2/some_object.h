#ifndef SOME_OBJECT_H
#define SOME_OBJECT_H

class some_object
{
    public:
        some_object();
        explicit some_object(int value);
        some_object(const some_object& other);
        some_object(some_object&& other);
        ~some_object() = default;
        some_object& operator=(const some_object& other);
        some_object& operator=(some_object&& other);
        int get_value() const;
        void set_value(int value);
    private:
        int m_value;
};

#endif
