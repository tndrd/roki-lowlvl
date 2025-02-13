import rokipytest as rpt
from rokipytest import Picamera2
import rokilowlvlpy as roki
import time
rpt.mute_picamera()

TEST_TIME_S = 1
FRAME_DURATION_US = 16700
FRAME_DURATION_MS = FRAME_DURATION_US // 1000
DURATION_THRESHOLD = 4

picam2 = Picamera2(camera_num=0)

mb = rpt.create_motherboard()

rpt.call(mb, mb.reset_strobe_containers())
rpt.call(mb, mb.configure_strobe_filter(FRAME_DURATION_MS, DURATION_THRESHOLD))

picam2.start()
rpt.eprint(f"Running for {TEST_TIME_S}s...")
time.sleep(1)
picam2.set_controls({"FrameDurationLimits": (FRAME_DURATION_US, FRAME_DURATION_US)})
time.sleep(TEST_TIME_S)
picam2.stop()

imu = rpt.call(mb, mb.get_imu_container_info())
body = rpt.call(mb, mb.get_body_container_info())

if not body.active:
    rpt.eprint("Warning: body callback is disabled")
elif rpt.check(body.num_av == 0):
    rpt.failure_stop("No frames in body frCont")

if rpt.check(imu.num_av == 0):
    rpt.failure_stop("No frames in IMU frCont")

rpt.call(mb, mb.reset_strobe_containers())

imu = rpt.call(mb, mb.get_imu_container_info())
body = rpt.call(mb, mb.get_body_container_info())

if rpt.check((body.num_av != 0) or (body.first != 0)):
    rpt.failure_stop("Body frCont did not reset")

if rpt.check((imu.num_av != 0) or (imu.first != 0)):
    rpt.failure_stop("IMU frCont did not reset")

rpt.end_test()
