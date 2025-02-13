#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "roki-lowlvl/MbFactory.hpp"
#include "roki-mb-interface/MotherboardAdapter.hpp"
#include "roki-mb-interface/Protocols/RokiRcb4Adapter.hpp"
#include "roki-mb-interface/Protocols/ZubrAdapter.hpp"

#define str(a) #a
#define xstr(a) str(a)

namespace MI = MbInterface;
namespace py = pybind11;

PYBIND11_MODULE(PYTHON_MODULE_NAME, m) {
  m.doc() = "Library for communicating with Roki's hardware.\n"
            "Author: Lekhterev V.V. @tndrd, Starkit 2024";

  /* Motherboard */

  py::class_<MI::Quaternion>(m, "Quaternion")
      .def(py::init<>())
      .def_readwrite("x", &MI::Quaternion::X)
      .def_readwrite("y", &MI::Quaternion::Y)
      .def_readwrite("z", &MI::Quaternion::Z)
      .def_readwrite("w", &MI::Quaternion::W);

  py::class_<MI::Timestamp>(m, "Timestamp")
      .def(py::init<>())
      .def_readwrite("time_s", &MI::Timestamp::TimeS)
      .def_readwrite("time_ns", &MI::Timestamp::TimeNS);

  py::class_<MI::IMUFrame>(m, "IMUFrame")
      .def(py::init<>())
      .def_readwrite("orientation", &MI::IMUFrame::Orientation)
      .def_readwrite("timestamp", &MI::IMUFrame::Timestamp)
      .def_readwrite("sensor_id", &MI::IMUFrame::SensorID);

  py::class_<MI::FrameContainerInfo>(m, "FrameContainerInfo")
      .def(py::init<>())
      .def_readwrite("first", &MI::FrameContainerInfo::First)
      .def_readwrite("num_av", &MI::FrameContainerInfo::NumAv)
      .def_readwrite("max_frames", &MI::FrameContainerInfo::MaxFrames)
      .def_readwrite("active", &MI::FrameContainerInfo::Active);

  py::class_<MI::BodyQueueInfo>(m, "BodyQueueInfo")
      .def(py::init<>())
      .def_readwrite("size", &MI::BodyQueueInfo::Size)
      .def_readwrite("capacity", &MI::BodyQueueInfo::Capacity);

  py::enum_<MI::TTYConfig::StopbitsCount>(m, "Stopbits", py::arithmetic())
      .value("One", MI::TTYConfig::StopbitsCount::One)
      .value("Two", MI::TTYConfig::StopbitsCount::Two);

  py::class_<MI::TTYConfig>(m, "TTYConfig")
      .def(py::init<>())
      .def_readwrite("port", &MI::TTYConfig::Port)
      .def_readwrite("baudrate", &MI::TTYConfig::Baudrate)
      .def_readwrite("stopbits", &MI::TTYConfig::Stopbits)
      .def_readwrite("parity_bit", &MI::TTYConfig::ParityBit)
      .def_readwrite("timeout", &MI::TTYConfig::Timeout);

  using MA = MI::MotherboardAdapter;

  py::class_<MA>(m, "Motherboard")
      .def(py::init<>())
      .def("configure", &MA::Configure)
      .def("get_imu_frame", &MA::GetIMUFrame)
      .def("get_body_frame", &MA::GetBodyFrame)
      .def("get_imu_container_info", &MA::GetIMUContainerInfo)
      .def("get_body_container_info", &MA::GetBodyContainerInfo)
      .def("reset_strobe_containers", &MA::ResetStrobeContainers)
      .def("set_imu_strobe_offset", &MA::SetIMUStrobeOffset)
      .def("set_body_strobe_offset", &MA::SetBodyStrobeOffset)
      .def("get_imu_latest", &MA::GetIMULatest)
      .def("get_strobe_width", &MA::GetStrobeWidth)
      .def("configure_strobe_filter", &MA::ConfigureStrobeFilter)
      .def("get_body_queue_info", &MA::GetBodyQueueInfo)
      .def("set_body_queue_period", &MA::SetBodyQueuePeriod)
      .def("is_ok", &MA::IsOk)
      .def("get_error", &MA::GetError)
      .def("reset_body_queue", &MA::ResetBodyQueue)
      .def("set_body_timeout", &MA::SetBodyTimeout)
      .def("disable_body_arq", &MA::DisableBodyARQ)
      .def("reset_body_strobe_callback", &MA::ResetBodyStrobeCallback);
      
   m.def("create_motherboard", &LowLvl::MbFactoryAdapter::CreateAdapter);

  /* Protocols */

  auto pModule =
      m.def_submodule("protocols", "Collection of hardware protocol clients");

  /* Rcb4 */
  using Rcb4 = MbProtocols::RokiRcb4Adapter;

  py::class_<Rcb4> rcb4(pModule, "Rcb4");

  py::class_<Rcb4::ServoData>(rcb4, "ServoData")
      .def(py::init<>())
      .def_readwrite("id", &Rcb4::ServoData::Id)
      .def_readwrite("sio", &Rcb4::ServoData::Sio)
      .def_readwrite("data", &Rcb4::ServoData::Data);

  rcb4.def(py::init<MI::MotherboardAdapter &>());
  rcb4.def("check_acknowledge", &Rcb4::checkAcknowledge);
  rcb4.def("get_pio", &Rcb4::getPio);
  rcb4.def("set_pio", &Rcb4::setPio);
  rcb4.def("get_pio_mode", &Rcb4::getPioMode);
  rcb4.def("set_pio_mode", &Rcb4::setPioMode);
  rcb4.def("set_single_servo",
           static_cast<bool (Rcb4::*)(Rcb4::ServoData, uint8_t)>(
               &Rcb4::setSingleServo));
  rcb4.def("set_free_single_servo", &Rcb4::setFreeSingleServo);
  rcb4.def("set_hold_single_servo", &Rcb4::setHoldSingleServo);
  rcb4.def("set_servo_pos", &Rcb4::setServoPos);
  rcb4.def("set_servo_pos_async", &Rcb4::setServoPosAsync, py::arg("servo_data"),
           py::arg("frames"), py::arg("pause") = 0);
  rcb4.def("set_free_pos", &Rcb4::setFreePos);
  rcb4.def("set_hold_pos", &Rcb4::setHoldPos);
  rcb4.def("get_single_pos", &Rcb4::getSinglePos);
  rcb4.def("set_servo_speed", &Rcb4::setServoSpeed);
  rcb4.def("set_servo_stretch", &Rcb4::setServoStretch);
  rcb4.def("read_addr", &Rcb4::moveRamToComCmdSynchronize);
  rcb4.def("motion_play", &Rcb4::motionPlay);
  rcb4.def("enable_arq", &Rcb4::enableARQ);
  rcb4.def("enable_strobe_callback", &Rcb4::enableStrobeCallback);

  rcb4.def("get_error", &Rcb4::GetError);
  rcb4.def("is_ok", &Rcb4::IsOk);

  using Zubr = MbProtocols::ZubrAdapter;
  py::class_<Zubr> zubr(pModule, "Zubr");

  zubr.def(py::init<MI::MotherboardAdapter &>());
  zubr.def("mem_i_set", &Zubr::MemISet);
  zubr.def("mem_f_set", &Zubr::MemFSet);
  zubr.def("mem_i_get", &Zubr::MemIGet);
  zubr.def("mem_f_get", &Zubr::MemFGet);
  zubr.def("is_ok", &Zubr::IsOk);
  zubr.def("get_error", &Zubr::GetError);
}
