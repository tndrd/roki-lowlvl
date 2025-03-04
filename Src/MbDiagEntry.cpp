#include "roki-lowlvl/MbFactory.hpp"
#include "roki-mb-daemon/Helpers.hpp"
#include "gtest/gtest.h"
#include <cassert>
#include <cstring>
#include <functional>
#include <glob.h>
#include <sstream>
#include <string>

using namespace std::string_literals;

#define PREDIAG_MSG "Failed to complete pre-diagnostic checks: "s
#define PREDIAG_NAME "PreDiagnostic"

#define STR(x) #x
#define XSTR(x) STR(x)

#define DIAG_SUITES_PATH XSTR(DIAG_SUITES_DIR)

void PreDiagnosticRoutine() try {
  auto mb = LowLvl::MbFactory::Create(PREDIAG_NAME);
} catch (std::exception &e) {
  throw std::runtime_error(PREDIAG_MSG + e.what());
} catch (...) {
  throw std::runtime_error(PREDIAG_MSG + "Unknown exception");
}

int WarnPathError(const char *epath, int eerrno) {
  std::cout << "Warning: " << epath << ": " << strerror(eerrno) << std::endl;
  return 0;
}

std::vector<std::string> GetSuites() {
  glob_t globbuf{};
  glob(DIAG_SUITES_PATH "/*.suite", 0, WarnPathError, &globbuf);

  MbDaemon::Helpers::Defer _([&globbuf] { globfree(&globbuf); });

  std::vector<std::string> paths(globbuf.gl_pathc, "");

  for (int i = 0; i < paths.size(); ++i) {
    paths[i] = globbuf.gl_pathv[i];
  }

  return paths;
}

void ListSuites(const std::vector<std::string> &paths) {
  for (const std::string &path : paths) {
    std::cout << " - ";
    size_t pos1 = path.rfind('/');
    size_t pos2 = path.find('.');

    for (size_t i = pos1 + 1; i < pos2; ++i)
      std::cout << path[i];

    std::cout << std::endl;
  }
}

void PrintHelp() {
  std::cout << "\nmbdiag: Hardware diagnostic utility" << std::endl;
  std::cout << "Author: Lekhterev V.V. @tndrd, Starkit 2025"
               "\n"
            << std::endl;
  std::cout << "Usage:\n  mbdiag <SUITE_NAME> - runs diagnostic suite\n"
            << std::endl;

  std::cout << "List of available suites:" << std::endl;
  ListSuites(GetSuites());
}

std::string LoadSuite(const std::string &suitename) {
  std::ifstream strm;
  std::string path = DIAG_SUITES_PATH "/" + suitename + ".suite";
  strm.open(path);

  if (strm.fail())
    throw MbDaemon::Helpers::ErrnoException("Failed to open " + path, errno);

  char c = 0;
  strm >> c;

  std::stringstream ss;

  while (!strm.eof() && c != '\n') {
    ss << c;
    strm >> c;
  }

  return ss.str();
}

int main(int argc, char *argv[]) try {
  if (argc != 2) {
    PrintHelp();
    return 0;
  }

  PreDiagnosticRoutine();

  auto pattern = LoadSuite(argv[1]);

  std::string arg = "--gtest_filter=" + pattern;

  argv[1] = &arg[0];

  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
} catch (std::exception &e) {
  std::cout << "Caught exception: " << e.what() << std::endl;
} catch (...) {
  std::cout << "Unknown exception" << std::endl;
}

#include "MbDiagImpl.cpp"