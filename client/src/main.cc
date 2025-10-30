#include "client.h"
#include <iostream>

int main() {
    try {
        MessageUClient client;
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

