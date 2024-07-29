import rokipytest as rpt
from rokipytest import Picamera2

import rokilowlvlpy as roki
import time
import sys
rpt.mute_picamera()

assert len(sys.argv) == 2

TEST_TIME_S = int(sys.argv[1])
FRAME_DURATION_US = 16700
FRAME_DURATION_MS = FRAME_DURATION_US // 1000
DURATION_THRESHOLD = 4

picam2 = Picamera2(camera_num=0)
mb = rpt.create_motherboard()

rpt.call(mb, mb.reset_strobe_containers())
rpt.call(mb, mb.configure_strobe_filter(FRAME_DURATION_MS, DURATION_THRESHOLD))

picam2.start()
rpt.eprint(f"Running for {TEST_TIME_S + 1}s...")
time.sleep(1)
picam2.set_controls({"FrameDurationLimits": (FRAME_DURATION_US, FRAME_DURATION_US)})
time.sleep(TEST_TIME_S)
picam2.stop()

imu = rpt.call(mb, mb.get_imu_container_info())
body = rpt.call(mb, mb.get_body_container_info())

if rpt.check(body.num_av == 0):
    rpt.failure_stop("No frames in body frCont")

if rpt.check(imu.num_av == 0):
    rpt.failure_stop("No frames in IMU frCont")

rpt.end_test()
