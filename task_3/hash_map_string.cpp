#include "./hash_map_string.h"
#include "./string.h"

#include <exception>

class hash_element
{
    public:
        string key;
        string value;
        hash_element* next{};

        hash_element(const string& it, const string& what) : key(it), value(what)
        {
        }

        hash_element(string&& it, string&& what) : key((string&&)it), value((string&&)what)
        {
        }
};

hash_map_string::hash_map_string()
{
}

hash_map_string::~hash_map_string()
{
    for (size_t i = 0; i < 256; i++)
    {
        hash_element* node = head[i].first;

        while (node != nullptr)
        {
            hash_element* next = node->next;
            delete node;
            node = next;
        }
    }
}

hash_map_string::hash_map_string(const hash_map_string& val) : hash_map_string()
{
    for (size_t i = 0; i < 256; i++)
    {
        hash_element* node = val.head[i].first;
        hash_element** place = &head[i].first;

        while (node != nullptr)
        {
            *place = new hash_element(node->key, node->value);
            
            place = &(*place)->next;
            head[i].count++;
            _size++;
            node = node->next;
        }
    }
}

hash_map_string& hash_map_string::operator=(const hash_map_string& val)
{
    if (this == &val)
        return *this;

    hash_map_string result(val);

    return (*this = (hash_map_string&&)result);
}

hash_map_string::hash_map_string(hash_map_string&& val) : hash_map_string()
{
    *this = (hash_map_string&&)val;
}

hash_map_string& hash_map_string::operator=(hash_map_string&& val)
{
    if (this == &val)
        return *this;

    for (size_t i = 0; i < 256; i++)
    {
        hash_element* node = head[i].first;

        while (node != nullptr)
        {
            hash_element* next = node->next;
            delete node;
            node = next;
        }

        head[i].first = val.head[i].first;
        head[i].count = val.head[i].count;
        val.head[i].first = nullptr;
        val.head[i].count = 0;
    }

    _size = val._size;
    val._size = 0;

    return *this;
}

bool hash_map_string::empty() const
{
    return _size == 0;
}

const string& hash_map_string::at(const string& it) const
{
    size_t hash = (unsigned char)it[0];
    hash_element* node = head[hash].first;

    while (node != nullptr)
    {
        if (node->key == it)
            return node->value;

        node = node->next;
    }

    throw std::exception();
}

string& hash_map_string::at(const string& it)
{
    size_t hash = (unsigned char)it[0];
    hash_element* node = head[hash].first;

    while (node != nullptr)
    {
        if (node->key == it)
            return node->value;

        node = node->next;
    }

    throw std::exception();
}

string& hash_map_string::operator[](const string& it)
{
    size_t hash = (unsigned char)it[0];
    hash_element** place = &head[hash].first;

    while (*place != nullptr)
    {
        if ((*place)->key == it)
            return (*place)->value;

        place = &(*place)->next;
    }

    hash_element* inserted = new hash_element(it, string());

    *place = inserted;
    head[hash].count++;
    _size++;

    return inserted->value;
}

bool hash_map_string::insert(const string& it, const string& what)
{
    size_t hash = (unsigned char)it[0];
    hash_element** place = &head[hash].first;

    while (*place != nullptr)
    {
        if ((*place)->key == it)
            return 0;

        place = &(*place)->next;
    }

    *place = new hash_element(it, what);
    head[hash].count++;
    _size++;

    return 1;
}

bool hash_map_string::insert(string&& it, string&& what)
{
    size_t hash = (unsigned char)it[0];
    hash_element** place = &head[hash].first;

    while (*place != nullptr)
    {
        if ((*place)->key == it)
            return 0;

        place = &(*place)->next;
    }

    *place = new hash_element((string&&)it, (string&&)what);
    head[hash].count++;
    _size++;

    return 1;
}

bool hash_map_string::erase(const string& it)
{
    size_t hash = (unsigned char)it[0];
    hash_element** place = &head[hash].first;

    while (*place != nullptr)
    {
        if ((*place)->key == it)
        {
            hash_element* removed = *place;
            *place = removed->next;
            delete removed;
            head[hash].count--;
            _size--;

            return 1;
        }

        place = &(*place)->next;
    }

    return 0;
}

size_t hash_map_string::replace(const string& what, const string& val)
{
    if (_size == 0)
        return 0;

    string old_value(what);
    string new_value(val);
    if (old_value == new_value)
        return 0;

    size_t result = 0;

    for (size_t i = 0; i < 256; i++)
    {
        hash_element* node = head[i].first;

        while (node != nullptr)
        {
            if (node->value.replace(old_value, new_value))
                result++;

            node = node->next;
        }
    }

    return result;
}

hash_map_string hash_map_string::cut_subhash(unsigned char val)
{
    hash_map_string result;

    result.head[val].first = head[val].first;
    result.head[val].count = head[val].count;
    result._size = head[val].count;
    
    head[val].first = nullptr;
    head[val].count = 0;
    _size -= result._size;

    return result;
}

size_t hash_map_string::count_lt(const string& val) const
{
    size_t hash = (unsigned char)val[0];
    size_t result = 0;

    for (size_t i = 0; i < hash; i++)
        result += head[i].count;

    hash_element* node = head[hash].first;

    while (node != nullptr)
    {
        if (node->key < val)
            result++;

        node = node->next;
    }

    return result;
}
