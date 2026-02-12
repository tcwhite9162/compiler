#pragma once

#include <cstddef>
#include <cstring>
#include <iostream>
#include <string_view>

class Arena {
  public:
    explicit Arena(size_t capacity = 1024 * 1024) {
        buffer = new char[capacity];
        end = buffer + capacity;
        ptr = buffer;
    }

    ~Arena() {
        delete[] buffer;
    }


    char* alloc_bytes(size_t size, size_t align = alignof(std::max_align_t)) {
        uintptr_t current = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t aligned = (current + align - 1) & ~(align - 1);

        if (aligned + size > reinterpret_cast<uintptr_t>(end)) {
            std::cerr << "Arena out of memory" << std::endl;
            std::exit(-1);
        }

        ptr = reinterpret_cast<char*>(aligned + size);
        return reinterpret_cast<char*>(aligned);
    }

    template<typename T, typename... Args>
    T* alloc(Args&&... args) {
        char* mem = alloc_bytes(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    std::string_view copy(const char* begin, size_t len) {
        char* mem = alloc_bytes(len);
        std::memcpy(mem, begin, len);
        return std::string_view(mem, len);
    }

    [[nodiscard]] size_t used() const { return ptr - buffer; }
    [[nodiscard]] size_t remaining() const { return end - ptr; }

  private:
    char* buffer;
    char* ptr;
    char* end;
};
