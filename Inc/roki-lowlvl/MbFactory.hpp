#pragma once

#include "roki-lowlvl/Aliases.hpp"
#include "roki-mb-daemon/DaemonTools.hpp"
#include "roki-mb-daemon/Helpers.hpp"
#include "roki-mb-interface/Motherboard.hpp"

#include <memory>

namespace LowLvl {
class MbFactory {
public:
  template <typename T> struct Wrapper : public T {
  private:
    std::shared_ptr<void> _{nullptr, [](void *) { FreePort(); }};

  public:
    Wrapper() = default;
    Wrapper(const Wrapper &) = delete;
    Wrapper &operator=(const Wrapper &) = delete;
    Wrapper(Wrapper &&) = default;
    Wrapper &operator=(Wrapper &&) = default;

    virtual ~Wrapper() = default;
  };

private:
  static TTYConfig GetConfig(const std::string &username);
  static const char *GetEnv(const char *env);
  static std::string GetPortFromDaemon(const std::string &username);
  static void FreePort();

public:
  static Wrapper<Motherboard> Create(const std::string &username);
  static Wrapper<MotherboardAdapter> CreateAdapter(const std::string &username);
};

class MbFactoryAdapter {
  public:
    static std::unique_ptr<Motherboard> Create(const std::string &username);
    static std::unique_ptr<MotherboardAdapter> CreateAdapter(const std::string& username);
};

} // namespace LowLvl