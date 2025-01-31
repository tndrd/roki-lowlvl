#include "rcb4-base-class/Rcb4BaseClass.hpp"
#include "roki-lowlvl/MbFactory.hpp"
#include "roki-mb-interface/Protocols/RokiRcb4.hpp"
#include "roki-mb-interface/Protocols/SKServoAdapter.hpp"
#include "roki-mb-interface/Protocols/ZubrAdapter.hpp"

#include <gtest/gtest.h>
#include <math.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#define str(a) #a
#define xstr(a) str(a)

#define SCRIPTS_DIR xstr(DIAG_SCRIPTS_DIR) "/"
#define USER_NAME "MbDiagnostic"
#define QUAT_ABS_TOLERANCE 0.001
#define HEAD_IMU_SENSOR_ID 37

#ifdef USE_MOCKS
#pragma message USE_MOCKS is set, supressing some checks
#else
#pragma message USE_MOCKS is not set, building normally
#endif

using namespace MbInterface;

#define MB_CALL(foo) ASSERT_TRUE(mb.foo) << mb.GetError() << std::endl
#define INIT_MB auto mb = LowLvl::MbFactory::Create(USER_NAME);

#define INIT_RCB                                                               \
  INIT_MB;                                                                     \
  MbProtocols::RokiRcb4 rcb4{mb};                                              \
  ASSERT_TRUE(rcb4.checkAcknowledge()) << rcb4.GetError() << std::endl;

#define RCB_CALL(foo) ASSERT_TRUE(rcb4.foo) << rcb4.GetError() << std::endl;

#define INIT_ZUBR                                                              \
  INIT_MB;                                                                     \
  MbProtocols::ZubrAdapter zubr{mb};

#define ZUBR_CHECK(ret) ASSERT_TRUE(ret) << zubr.GetError() << std::endl;

#define INIT_SKS                                                               \
  INIT_MB;                                                                     \
  MbProtocols::SKServoAdapter sks{mb};

#define SKS_CHECK(ret) ASSERT_TRUE(ret) << sks.GetError() << std::endl;

bool AskPrompt(const std::string &msg) {
  std::cerr << msg << "? [y/n]" << std::endl;
  char resp = getchar();
  getchar();

  return resp == 'y';
}

bool AskSkip() {
  std::cerr << "Want to skip? [y/n]: " << std::endl;
  char resp = getchar();
  getchar();

  return resp == 'y';
}

#define PROMPT(msg) ASSERT_TRUE(AskPrompt(msg))
#define ASK_SKIP                                                               \
  if (AskSkip())                                                               \
    ASSERT_TRUE(true);

bool RokiPyTestRun(const std::string &name,
                   /* const */ std::vector<std::string> /* & */ args = {}) {
  pid_t pid = fork();

  if (pid == 0) { // child
    std::string path = SCRIPTS_DIR + name;

    std::string python3 = "python3";

    std::vector<char *> argv;
    argv.push_back(&python3[0]);
    argv.push_back(&path[0]);

    for (auto &arg : args)
      argv.push_back(&arg[0]);

    argv.push_back(NULL);

    execvp(argv[0], argv.data());

    perror("Failed to execute");
    exit(1);
  }
  if (pid > 0) { // parent
    int status;
    wait(&status);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
      return true;
    else
      return false;
  }

  perror("Failed to fork");
  return false;
}

#define RPTEST(name)                                                           \
  ASSERT_TRUE(RokiPyTestRun(name))                                             \
      << "Failed to run test " << name << std::endl;
#define RPTEST_ARG(name, args)                                                 \
  ASSERT_TRUE(RokiPyTestRun(name, args))                                       \
      << "Failed to run test " << name << std::endl;

void PrintFrame(const IMUFrame &frame) {
  std::cerr << "Orientation:" << std::endl;
  std::cerr << "  X: " << frame.Orientation.X << std::endl;
  std::cerr << "  Y: " << frame.Orientation.Y << std::endl;
  std::cerr << "  Z: " << frame.Orientation.Z << std::endl;
  std::cerr << "  W: " << frame.Orientation.W << std::endl;
  std::cerr << "Timestamp:" << std::endl;
  std::cerr << "  S: " << frame.Timestamp.TimeS << std::endl;
  std::cerr << " nS: " << frame.Timestamp.TimeNS << std::endl;
  std::cerr << "SensorId: " << +frame.SensorID << std::endl;
}

void PrintServoData(const uint8_t *data, size_t size) {
  std::cerr << "TODO" << std::endl;
}

float QuatVal(const uint8_t *data) {
  const int16_t *ptr = reinterpret_cast<const int16_t *>(data);
  int16_t val = *ptr;
  float result = float(val) / 16384.0f;
  return result;
}

void ValidateFrame(const IMUFrame &frame) {
  auto &q = frame.Orientation;
  float sum2 = q.X * q.X + q.Y * q.Y + q.Z * q.Z + q.W * q.W;
#ifndef USE_MOCKS
  EXPECT_LE(abs(std::sqrt(sum2) - 1), QUAT_ABS_TOLERANCE);
  EXPECT_EQ(frame.SensorID, HEAD_IMU_SENSOR_ID);
#endif
}

void PrintBodyIMU(const uint8_t *data, size_t size) {
  assert(size == 8);
  std::cerr << "Orientation:" << std::endl;
  std::cerr << "  X: " << QuatVal(data + 0) << std::endl;
  std::cerr << "  Y: " << QuatVal(data + 2) << std::endl;
  std::cerr << "  Z: " << QuatVal(data + 4) << std::endl;
  std::cerr << "  W: " << QuatVal(data + 6) << std::endl;
}

TEST(SerialInterface, NoFile) {
  MbSerial si;
  MbSerial::TTYConfig tty;

  tty.Baudrate = 123456;
  tty.ParityBit = true;
  tty.Stopbits = MbSerial::TTYConfig::StopbitsCount::One;
  tty.Timeout = 12;
  tty.Port = "TRASH";

  ASSERT_FALSE(si.Configure(tty)) << si.GetError() << std::endl;
}

TEST(SerialInterface, WrongTimeout) {
  MbSerial si;
  MbSerial::TTYConfig tty;

  tty.Baudrate = 123456;
  tty.ParityBit = true;
  tty.Stopbits = MbSerial::TTYConfig::StopbitsCount::One;
  tty.Timeout = 42;
  tty.Port = "/dev/ttyAMA2";

  ASSERT_FALSE(si.Configure(tty));
}

TEST(MbFactory, Create) {
  using F = LowLvl::MbFactory;
  using FA = LowLvl::MbFactoryAdapter;
  { auto mb = F::Create(USER_NAME); }
  { auto mb = F::CreateAdapter(USER_NAME); }
  { auto mb = FA::Create(USER_NAME); }
  { auto mb = FA::CreateAdapter(USER_NAME); }
}

TEST(Motherboard, GetIMULatest) {
  INIT_MB;

  IMUFrame frame;
  MB_CALL(GetIMULatest(frame));
  ValidateFrame(frame);
}

TEST(Motherboard, ConfigureStrobeFilter) {
  INIT_MB;

  MB_CALL(ConfigureStrobeFilter(0, 0));
}

TEST(Motherboard, SetIMUStrobeOffset) {
  INIT_MB;

  MB_CALL(SetIMUStrobeOffset(0));
}

TEST(Motherboard, SetBodyStrobeOffset) {
  INIT_MB;

  MB_CALL(SetBodyStrobeOffset(0));
}

TEST(Motherboard, GetIMUContainerInfo) {
  INIT_MB;

  FrameContainerInfo info;

  MB_CALL(GetIMUContainerInfo(info));
}

TEST(Motherboard, GetIMUFrame) {
  RPTEST_ARG("take_photos.py", {"1"});

  INIT_MB;

  FrameContainerInfo info;
  IMUFrame frame;

  MB_CALL(GetIMUContainerInfo(info));
#ifndef USE_MOCKS
  ASSERT_GT(info.NumAv, 0) << "Queue empty" << std::endl;
#endif
  MB_CALL(GetIMUFrame(info.First, frame));
  ValidateFrame(frame);

#ifndef USE_MOCKS
  ASSERT_FALSE(mb.GetIMUFrame(info.First - 1, frame))
      << "This frame should not be available" << std::endl;
#endif
}

TEST(Motherboard, GetBodyContainerInfo) {
  INIT_MB;

  FrameContainerInfo dummy;

  MB_CALL(GetBodyContainerInfo(dummy));
}

TEST(Motherboard, GetBodyFrame) {
  RPTEST_ARG("take_photos.py", {"1"});

  INIT_MB;

  FrameContainerInfo info;
  BodyResponce responce;

  MB_CALL(GetBodyContainerInfo(info));
#ifndef USE_MOCKS
  ASSERT_GT(info.NumAv, 0) << "Queue empty" << std::endl;
#endif
  MB_CALL(GetBodyFrame(info.First, responce));

#ifndef USE_MOCKS
  ASSERT_FALSE(mb.GetBodyFrame(info.First - 1, responce))
      << "This frame should not be available" << std::endl;
#endif
}

TEST(Motherboard, ResetStrobeContatiners) { RPTEST("test_reset_sc.py"); }

TEST(Motherboard, BodySendForward) {
  INIT_MB;

  Rcb4BaseClass rcb4;

  std::array<uint8_t, 4> requestData;
  std::array<uint8_t, 4> responceData;
  rcb4.acknowledgeCmd(requestData.data());

  MB_CALL(BodySendForward(requestData.data(), 4, responceData.data(), 4));
#ifndef USE_MOCKS
  ASSERT_EQ(requestData, responceData);
#endif
}

TEST(Motherboard, GetBodyQueueInfo) {
  INIT_MB;

  BodyQueueInfo info;
  MB_CALL(GetBodyQueueInfo(info));
}

TEST(Motherboard, SetBodyQueuePeriod) {
  INIT_MB;

  MB_CALL(SetBodyQueuePeriod(20));
#ifndef USE_MOCKS
  ASSERT_FALSE(mb.SetBodyQueuePeriod(0));
#endif
}

TEST(Motherboard, BodySendQueue) {
  INIT_MB;
  Rcb4BaseClass rcb4;
  BodyQueueInfo info;

  std::array<uint8_t, 4> requestData;
  rcb4.acknowledgeCmd(requestData.data());

  MB_CALL(GetBodyQueueInfo(info));

  for (int i = 0; i < info.Capacity; ++i) {
    MB_CALL(BodySendQueue(requestData.data(), 4, 4));
  }

#ifndef USE_MOCKS
  int attempts = 0;
  while (mb.BodySendQueue(requestData.data(), 4, 4)) {
    ASSERT_TRUE(attempts++ < info.Capacity)
        << "Failed to overflow body queue" << std::endl;
  }
#endif
}

TEST(Rcb4, Acknowledge) { INIT_RCB; }

TEST(Rcb4, SetServoPosSync) {
  INIT_RCB;

  MbProtocols::RokiRcb4::ServoData sd;

  sd.Id = 8;
  sd.Sio = 0;
  sd.Data = 8000;

  RCB_CALL(setServoPos(&sd, 1, 10));

  sd.Data = 7500;
  RCB_CALL(setServoPos(&sd, 1, 10));
}

TEST(Rcb4, SetServoPosAsync) {
  INIT_RCB;

  MbProtocols::RokiRcb4::ServoData sd;

  sd.Id = 8;
  sd.Sio = 0;
  sd.Data = 7000;

  RCB_CALL(setServoPosAsync(&sd, 1, 10));

  sd.Data = 7500;
  RCB_CALL(setServoPosAsync(&sd, 1, 10));
}

TEST(Motherboard, TestHeadIMU) { RPTEST("test_head_imu.py"); }
TEST(Rcb4, TestBodyIMU) { RPTEST("test_body_imu.py"); }

TEST(Motherboard, TestBodyQueue) { RPTEST("test_bq.py"); }

TEST(Rcb4, ReadRam) {
  INIT_RCB;

  std::array<uint8_t, 8> zeroes = {};
  std::array<uint8_t, 8> rxData;

  RCB_CALL(moveRamToComCmdSynchronize(0x0060, 8, rxData.data()));

  ASSERT_NE(rxData, zeroes) << "read resulted in zeroes" << std::endl;
}

TEST(Motherboard, TestStrobeFilter) { RPTEST("test_sf.py"); }

#define ZUBR_ADDR 600
#define ZUBR_IDATA 1 << 31 - 1
#define ZUBR_FDATA 0.424242f

TEST(Zubr, MemInt) {
  INIT_ZUBR;

  ZUBR_CHECK(zubr.MemISet(ZUBR_ADDR, ZUBR_IDATA));

  auto ret = zubr.MemIGet(ZUBR_ADDR);
  ZUBR_CHECK(std::get<0>(ret));
#ifndef USE_MOCKS
  ASSERT_EQ(std::get<1>(ret), ZUBR_IDATA);
#endif
}

TEST(Zubr, MemFloat) {
  INIT_ZUBR;

  ZUBR_CHECK(zubr.MemFSet(ZUBR_ADDR, ZUBR_FDATA));

  auto ret = zubr.MemFGet(ZUBR_ADDR);
  ZUBR_CHECK(std::get<0>(ret));
#ifndef USE_MOCKS
  ASSERT_EQ(std::get<1>(ret), ZUBR_FDATA);
#endif
}

TEST(Zubr, MemPython) { RPTEST("zubr_mem.py") }

void FindIds(int expectedCount = -1) {
  std::unordered_map<uint8_t, std::string> errs;
  std::unordered_map<uint8_t, int16_t> servos;

  auto idmin = StarkitServo::Limits::ServoIdMin;
  auto idmax = StarkitServo::Limits::ServoIdMax;

  for (int id = idmin; id <= idmax; ++id) {
    auto ret = sks.GetInfo(id);
    bool ok = std::get<0>(ret);
    auto resp = std::get<1>(ret);

    if (ok) {
      servos[id] = resp.Value;
      continue;
    }

    errs[id] = sks.GetError();
  }

  if (servos.size() != 0) {
    std::cerr << "Found servos, displaying positions: " << std::endl;
    for (const auto &iter : servos)
      std::cerr << "  #" << iter.first << ": " << iter.second << std::endl;
  }

  std::cerr << "Displaying unsuccessful requests: " << std::endl;

  for (const auto &iter : errs) {
    std::cerr << "  #" << iter.first << ": " << iter.second << std::endl;
  }

  if (expectedCount == -1)
    return;

  ASSERT_EQ(expectedCount, servos.size())
      << "Some servos did not respond correctly" << std::endl;
}

TEST(SKServo, FindServos) { FindIds(); }
TEST(SKServo, FindHeadServos) { FindIds(2); }

#undef MB_CALL
#undef INIT_MB
#undef INIT_RCB
#undef RCB_CALL
#undef PROMPT
#undef RPTEST
