#include "roki-lowlvl/MbFactory.hpp"
#include "gtest/gtest.h"

#include <string>

using namespace std::string_literals;

#define DONTRUN_CMD "dontrun"s
#define PREDIAG_MSG "Failed to complete pre-diagnostic checks: "s
#define PREDIAG_NAME "PreDiagnostic"

void PreDiagnosticRoutine() try {
  auto mb = LowLvl::MbFactory::Create(PREDIAG_NAME);
} catch (std::exception &e) {
  throw std::runtime_error(PREDIAG_MSG + e.what());
} catch (...) {
  throw std::runtime_error(PREDIAG_MSG + "Unknown exception");
}

int main(int argc, char *argv[]) try {
  if (argc == 2 && DONTRUN_CMD == argv[1]) {
    return 0;
  }

  PreDiagnosticRoutine();

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} catch (std::exception &e) {
  std::cout << "Caught exception: " << e.what() << std::endl;
} catch (...) {
  std::cout << "Unknown exception" << std::endl;
}

#include "MbDiagImpl.cpp"