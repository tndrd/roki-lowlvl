#include "roki-lowlvl/MbFactory.hpp"

#include <string>

using namespace std::string_literals;

#define DONTRUN_CMD "dontrun"s

int main(int argc, char* argv[]) try {
    if (argc == 2 && DONTRUN_CMD == argv[1]) {
        return 0;
    }

    if (argc != 1) {
        std::cout << "Unknown args" << std::endl;
        return 1;
    }
     
    auto mb = LowLvl::MbFactory::Create("Vova");
    sleep(10);
    return 0;
} catch(std::exception& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
} catch(...) {
    std::cout << "Unknown exception" << std::endl;
}
