#include "../include/circular_buffer.h"

#include <algorithm>
#include <iostream>
#include <vector>

int main(int, char**) {
    CircularBuffer<int> a(5);
    
    //std::sort(a.begin(), a.end());

    for (auto x : a) {
        std::cout << x << " ";
    }

    return 0;
}
