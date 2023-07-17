#pragma once

#include "gtest/gtest.h"

#include <iostream>
#include <map>


struct MemStats
{
    unsigned int count = 0;
    size_t size = 0;

    MemStats(int _count = 0, int _size = 0)
        : count(_count)
        , size(_size)
    {}

    MemStats& operator=(const MemStats& other) = default;

    MemStats operator+(const MemStats& other) const
    {
        return MemStats(count + other.count, size + other.size);
    }

    MemStats operator-(const MemStats& other) const
    {
        return MemStats(count - other.count, size - other.size);
    }

    MemStats operator*(unsigned int num) const
    {
        return MemStats(count * num, size * num);
    }

    MemStats operator/(unsigned int num) const
    {
        return MemStats(count / num, size / num);
    }

    bool operator==(const MemStats& other) const
    {
        return (count == other.count && size == other.size);
    }

    void reset()
    {
        count = 0;
        size = 0;
    }

    friend void PrintTo(const MemStats& m, std::ostream* os)
    {
        *os << "(" << m.count << "," << m.size << ")";
    }
};

static MemStats memStats, memCheckPoint;

#define RESET_MEM()                                                                                \
    memStats.reset();                                                                              \
    memCheckPoint.reset();

// Macro used to check the current number of allocations and size allocated so far.
#define EXPECT_MEM(_stats)                                                                         \
    {                                                                                              \
        MemStats s = memStats;                                                                     \
        EXPECT_EQ(s, _stats);                                                                      \
        memStats = s;                                                                              \
    }

#define EXPECT_ALLOC(_amount)                                                                      \
    EXPECT_MEM(memCheckPoint + _amount);                                                           \
    memCheckPoint = memStats;

#define EXPECT_DEALLOC(_amount)                                                                    \
    EXPECT_MEM(memCheckPoint - _amount);                                                           \
    memCheckPoint = memStats;

#define EXPECT_NO_ALLOC() EXPECT_MEM(memCheckPoint);


// Code adapted from
// https://stackoverflow.com/questions/438515/how-to-track-memory-allocations-in-c-especially-new-delete

// Similar implementation:
// http://wyw.dcweb.cn/leakage.htm
// https://github.com/adah1972/nvwa/blob/master/nvwa/debug_new.h
// https://github.com/adah1972/nvwa/blob/master/nvwa/debug_new.cpp

template<typename T>
struct AllocatorMalloc: std::allocator<T>
{
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef AllocatorMalloc<U> other;
    };

    AllocatorMalloc()
    {}

    template<typename U>
    AllocatorMalloc(AllocatorMalloc<U> const& u)
        : std::allocator<T>(u)
    {}

    pointer allocate(size_type size, std::allocator<void>::const_pointer = 0)
    {
        void* p = std::malloc(size * sizeof(T));
        if (p == 0) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type)
    {
        std::free(p);
    }
};

typedef std::map<void*, std::size_t, std::less<void*>,
                 AllocatorMalloc<std::pair<void* const, std::size_t>>>
    addr_map_t;

addr_map_t* get_map()
{
    // don't use normal new to avoid infinite recursion.
    static addr_map_t* addr_map = new (std::malloc(sizeof *addr_map)) addr_map_t;
    return addr_map;
}

void* operator new(std::size_t size) throw()
{
    // we are required to return non-null
    void* mem = std::malloc(size == 0 ? 1 : size);
    if (mem == 0) {
        throw std::bad_alloc();
    }
    (*get_map())[mem] = size;
    memStats.count++;
    memStats.size += size;
    return mem;
}

void operator delete(void* mem) throw()
{
    memStats.count--;
    memStats.size -= (*get_map())[mem];
    if (get_map()->erase(mem) == 0) {
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void operator delete(void* mem, std::size_t size) throw()
{
    size_t mapSize = (*get_map())[mem];
    memStats.count--;
    memStats.size -= mapSize;
    if (mapSize != size) {
        std::cerr << "bug: memory at " << mem << " delete operator called with mismatching sizes\n";
    }
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void* operator new[](std::size_t size) throw()
{
    // we are required to return non-null
    void* mem = std::malloc(size == 0 ? 1 : size);
    if (mem == 0) {
        throw std::bad_alloc();
    }
    (*get_map())[mem] = size;
    memStats.count++;
    memStats.size += size;
    return mem;
}

void operator delete[](void* mem) throw()
{
    memStats.count--;
    memStats.size -= (*get_map())[mem];
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void operator delete[](void* mem, std::size_t size) throw()
{
    size_t mapSize = (*get_map())[mem];
    memStats.count--;
    memStats.size -= mapSize;
    if (mapSize != size) {
        std::cerr << "bug: memory at " << mem << " delete operator called with mismatching sizes\n";
    }
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}
