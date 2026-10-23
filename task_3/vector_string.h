#ifndef VECTOR_STRING_H
#define VECTOR_STRING_H

#include <stddef.h>

class string;

class vector_string
{
    public:
        vector_string ();
        // Инициализировать массив копиями val, в количестве count
        vector_string (const string &val, size_t count);
        ~vector_string ();
        vector_string (const vector_string &val);
        vector_string &operator= (const vector_string &val);
        vector_string (vector_string &&val);
        vector_string &operator= (vector_string &&val);
        vector_string operator+ (const vector_string &val);
        vector_string &operator+= (const vector_string &val);
        
        const string &operator[] (size_t i) const;
        string &operator[] (size_t i);
        size_t size () const { return _size; }
        
        // true если массив пустой
        bool empty () const;
        
        // добавить копию строки str в конец массива
        // вернуть ссылку на текущий объект
        vector_string &push_back (string &&str);
        
        // добавить копию строки str в начало массива
        // вернуть ссылку на текущий объект
        vector_string &push_front (string &&str);
        
        // выполнить конкатенацию с массивом rhs
        // вернуть ссылку на текущий объект
        vector_string &append (vector_string &&rhs);
        vector_string &append (const vector_string &rhs);
        
        // удалить подмассив от индекса start до индекса end
        // вернуть ссылку на текущий объект
        // NOTE: индекс end -- не включительно!
        // NOTE: erase (0, size) удаляет весь подмассив
        // NOTE: erase (0, 0) не удаляет ничего
        vector_string &erase (size_t start, size_t end);
        
        // индекс первого вхождения строки str
        // сравнение на точное равенство
        // вернуть size если подстроки нет
        size_t find (const string &str) const;
        
        // индекс последнего вхождения строки str
        // сравнение на точное равенство
        // вернуть size если подстроки нет
        size_t rfind (const string &str) const;
        
        // заменить подстроку what подстрокой val во всех элементах
        // вернуть ссылку на текущий объект
        vector_string &replace (const string &what, const string &val);
        
        // вернуть подмассив от индекса start до индекса end
        // NOTE: индекс end -- не включительно!
        // NOTE: substr (0, size) возвращает копию массива
        // NOTE: substr (0, 0) возвращает пустой массив
        vector_string subvector (size_t start, size_t end) const;
    private:
        string *data{};
        size_t _size{};
    };

#endif