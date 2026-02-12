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

    char* alloc(size_t size) {
        if (ptr + size > end) {
            std::cerr << "Arena out of memory" << std::endl;
            std::exit(-1);
        }

        char* ret = ptr;
        ptr += size;
        return ret;
    }

    std::string_view copy(const char* begin, size_t len) {
        char* mem = alloc(len);
        std::memcpy(mem, begin, len);
        return std::string_view(mem, len);
    }

  private:
    char* buffer;
    char* ptr;
    char* end;
};
