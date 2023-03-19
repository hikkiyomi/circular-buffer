#include "../include/circular_buffer.h"

#include <algorithm>
#include <iostream>
#include <vector>

int main(int, char**) {
    CircularBuffer<int> a({1, 2, 3});

    a.push_back(123);

    sort(a.begin(), a.end());

    for (auto i : a) {
        std::cout << i << " ";
    }

    return 0;
}
