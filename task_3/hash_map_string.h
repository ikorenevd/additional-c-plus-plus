#ifndef HASH_MAP_STRING_H
#define HASH_MAP_STRING_H

#include <stddef.h>

class string;
class hash_element;

class hash_node
{
    public:
        hash_element *first{};
        size_t count{};
};

class hash_map_string
{
    public:
        hash_map_string ();
        ~hash_map_string ();
        hash_map_string (const hash_map_string &val);
        hash_map_string &operator= (const hash_map_string &val);
        hash_map_string (hash_map_string &&val);
        hash_map_string &operator= (hash_map_string &&val);
        
        size_t size () const { return _size; }
        bool empty () const;
        
        // Вернуть значение по ключу (it)
        // Бросить исключение, если такого ключа нет
        const string &at (const string &it) const;
        string &at (const string &it);
        
        // Вернуть значение по ключу (it)
        // Создать по умолчанию и вернуть, если такого ключа нет
        string &operator[] (const string &it);
        
        // Вставить значение (what) по ключу (it)
        // Вернуть true, если вставка прошла успешно
        // Вернуть false, если такой ключ уже есть
        bool insert (const string &it, const string &what);
        bool insert (string &&it, string &&what);
        
        // Удалить значение по ключу (it)
        // Вернуть true, если удаление прошло успешно
        // Вернуть false, если такого ключа нет
        bool erase (const string &it);
        
        // Заменить подстроку (what) на подстроку (val) во всех элементах
        // Вернуть количество измененнных элементов
        size_t replace (const string &what, const string &val);
        
        // Вырезать (переместить все узлы) все узлы с хэшем (val)
        // Если такого ключа нет, создать пустой объект
        hash_map_string cut_subhash (unsigned char val);
        
        // Вернуть количество значений с ключом строго меньшим заданного (it)
        size_t count_lt (const string &val) const;
    private:
        // Отдельный объект под каждое значение хэша (unsigned char)
        // Unsigned char принимает значения от 0 до 255
        hash_node head[256];
        size_t _size{};
};

#endif
