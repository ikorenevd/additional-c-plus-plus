// Minimal allocation support for block tests linked without the C++ runtime.
// Allocation failure terminates the isolated suite, which the runner reports.
#include <cstddef>
#include <cstdio>
#include <cstdlib>

void *operator new[](std::size_t size)
{
    void *memory = std::malloc(size == 0 ? 1 : size);
    if (memory == nullptr)
    {
        std::fputs("test allocation failed\n", stderr);
        std::abort();
    }
    return memory;
}

void operator delete[](void *memory) noexcept
{
    std::free(memory);
}

void operator delete[](void *memory, std::size_t) noexcept
{
    std::free(memory);
}
