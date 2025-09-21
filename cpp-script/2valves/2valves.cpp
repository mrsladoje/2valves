#include "ParallelWorker.h"
#include <iostream>

int main() {
    ParallelWorker worker;
    auto result = worker.run();
    std::cout << result.dump(2) << std::endl;
    return 0;
}
