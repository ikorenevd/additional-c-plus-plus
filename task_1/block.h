#ifndef BLOCK_H
#define BLOCK_H

#include <cstddef>
#include <cstdint>

class block
{
    public:
        block() = default;
        block(size_t size)
        {
            allocate_block(size);
        }
        block(double* start, size_t size)
        {
            link_block(start, size);
        }
        ~block()
        {
            free_block();
        }
        // Выделение памяти для класса (возвращает указатель на полученный
        // массив) Выделенный массив надо заполнить нулями
        double* allocate_block(size_t size);
        // Сохранение внешнего указателя в класс
        void link_block(double* start, size_t size);
        // Очистка памяти (только если указатель не внешний)
        void free_block();
        // Обмен указателями на массив между классами
        void swap_block(block& rhs);
        // Копирование содержимого массива из текущего класса в класс dest
        void copy_to(block& dest);
        // Копирование содержимого массива из класса src в текущий класс
        void copy_from(block& src);
        // Прибавляем не более чем m_size элементов из rhs в хранящийся массив
        // Возвращаем максимальный элемент из полученного массива
        double add_block(const block& rhs);
        // Вычитаем не более чем m_size элементов из rhs из хранящегося массива
        // Возвращаем минимальный элемент из полученного массива
        double sub_block(const block& rhs);
        // Печать не более чем 20 элементов массива в stderr
        // Формат печати каждого элемента:
        // fprintf (stderr, "%e ", m_start[i])
        void print_block();

    private:
        double* m_start{};
        size_t m_size{};
};

#endif