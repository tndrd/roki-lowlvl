#include "roki-lowlvl/MbFactory.hpp"
#define str(a) #a
#define xstr(a) str(a)

using namespace MbDaemon::Helpers;
using namespace std::string_literals;
using namespace LowLvl;

TTYConfig MbFactory::GetConfig(const std::string &username) {
  const char *baudStr = GetEnv(xstr(SERIAL_BAUDRATE_ENV));
  const char *stopStr = GetEnv(xstr(SERIAL_STOPBITS_ENV));
  const char *prtyStr = GetEnv(xstr(SERIAL_PARITY_ENV));
  const char *toutStr = GetEnv(xstr(SERIAL_TIMEOUT_ENV));

  TTYConfig conf;

  conf.Baudrate = std::stoul(baudStr);
  conf.Timeout = std::stof(toutStr);

  size_t stopbitsVal = std::stoul(stopStr);

  if (stopbitsVal == 1)
    conf.Stopbits = TTYConfig::StopbitsCount::One;
  else if (stopbitsVal == 2)
    conf.Stopbits = TTYConfig::StopbitsCount::Two;
  else
    throw FEXCEPT(std::runtime_error, "Bad value for stopbit count: " +
                                          std::to_string(stopbitsVal));

  size_t parityVal = std::stoul(stopStr);

  if (stopbitsVal == 0)
    conf.ParityBit = false;
  else if (stopbitsVal == 1)
    conf.ParityBit = true;
  else
    throw FEXCEPT(std::runtime_error,
                  "Bad value for parity bit: " + std::to_string(parityVal));

  conf.Port = GetPortFromDaemon(username);

  return conf;
}

const char *MbFactory::GetEnv(const char *env) {
  const char *ptr = std::getenv(env);
  if (!ptr)
    throw FEXCEPT(std::runtime_error, env + " env is not defined"s);

  return ptr;
}

std::string MbFactory::GetPortFromDaemon(const std::string &username) {
  MbDaemon::DaemonTools daemon{};

  using Client = MbDaemon::Client;
  using Proc = Client::Proc::Connect;

  auto client = daemon.Connect();

  Proc::Request request;

  request.Name.Data = username.c_str();
  request.Name.Size = username.size();
  request.PID = getpid();

  std::string responce = client.Call<Proc>(request).ToCxxStr();
  client.SoftDisconnect();

  return responce;
}

void MbFactory::FreePort() {
  MbDaemon::DaemonTools daemon{};

  using Client = MbDaemon::Client;
  using Proc = Client::Proc::Disconnect;

  try {
    auto client = daemon.Connect();
    client.Call<Proc>({});
    client.SoftDisconnect();
  } catch (std::exception &e) {
    std::cout << "Failed to cleanly exit: " << e.what();
  }
}

auto MbFactory::Create(const std::string &username) -> Wrapper<Motherboard> {
  TTYConfig conf = GetConfig(username);

  Wrapper<Motherboard> mb;
  if (!mb.Configure(conf))
    throw FEXCEPT(std::runtime_error,
                  "Failed to configure Motherboard: " + mb.GetError());

  return std::move(mb);
}

auto MbFactory::CreateAdapter(const std::string &username)
    -> Wrapper<MotherboardAdapter> {
  TTYConfig conf = GetConfig(username);

  Wrapper<MotherboardAdapter> mb;
  if (!mb.Configure(conf))
    throw FEXCEPT(std::runtime_error,
                  "Failed to configure Motherboard: " + mb.GetError());

  return std::move(mb);
}

std::unique_ptr<Motherboard> MbFactoryAdapter::Create(const std::string& username) {
  return std::make_unique<MbFactory::Wrapper<Motherboard>>(MbFactory::Create(username));
}

std::unique_ptr<MotherboardAdapter> MbFactoryAdapter::CreateAdapter(const std::string& username) {
    return std::make_unique<MbFactory::Wrapper<MotherboardAdapter>>(MbFactory::CreateAdapter(username));
}
