#include "skiplist.h"
#include <iostream>

int main() {
    skip_list<int> sl = {5, 2, 7, 1, 3};
    
    // Insert elements
    sl.insert(4);
    
    // Iterate in order
    for (int x : sl) {
        std::cout << x << " "; // 1 2 3 4 5 7
    }
    
    // Find elements
    if (sl.find(3) != sl.end()) {
        std::cout << "\nFound 3!";
    }
    
    // Erase elements
    sl.erase(5);
    
    return 0;
}
