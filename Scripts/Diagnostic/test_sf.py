import rokipytest as rpt
from rokipytest import Picamera2
import rokilowlvlpy as roki
import time
rpt.mute_picamera()

TEST_TIME_S = 5
UPDATE_PERIOD_S = 0.1
FRAME_DURATION_US = 16700
FRAME_DURATION_MS = FRAME_DURATION_US // 1000
DURATION_THRESHOLD = 4
FRAME_TOLERANCE = 2

n_frames = 0

n_strobes_imu = 0
n_strobes_body = 0

failed = False

do_compare = True
mb = rpt.create_motherboard()

def fits(val, target):
    return abs(val - target) < target * 0.02

def pre_callback(request):
    global n_frames
    global n_strobes_imu
    global n_strobes_body
    global failed

    duration = request.get_metadata()['FrameDuration']
    if fits(duration, FRAME_DURATION_US):
        n_frames += 1
        if not do_compare: return

        cont = rpt.call(mb, mb.get_imu_container_info())
        n_strobes_imu = cont.first + cont.num_av

        cont = rpt.call(mb, mb.get_imu_container_info())
        n_strobes_body = cont.first + cont.num_av

        cond1 = abs(n_frames - n_strobes_imu) > FRAME_TOLERANCE
        cond2 = abs(n_frames - n_strobes_body) > FRAME_TOLERANCE

        if (cond1 or cond2):
            failed = True

picam2 = Picamera2(camera_num=0)
picam2.pre_callback = pre_callback

rpt.call(mb, mb.reset_strobe_containers())
rpt.call(mb, mb.configure_strobe_filter(FRAME_DURATION_MS, DURATION_THRESHOLD))

picam2.start()
rpt.eprint(f"Running for {TEST_TIME_S + 1}s...")
time.sleep(1)
picam2.set_controls({"FrameDurationLimits": (FRAME_DURATION_US, FRAME_DURATION_US)})

n_ticks = TEST_TIME_S // UPDATE_PERIOD_S
ticks = 0
while (ticks < n_ticks):
    rpt.eprint(f"\rimu: {n_strobes_imu}, body: {n_strobes_body}, frames: {n_frames}", end="")
    if failed: break
    ticks += 1
    time.sleep(UPDATE_PERIOD_S)
rpt.eprint("\n")

width = rpt.call(mb, mb.get_strobe_width())
rpt.eprint(f"Strobe width: {width}")

picam2.stop()

if rpt.check(failed):
    rpt.failure_stop("Difference in frame count exceeded the tolerance")

if rpt.check(abs(width - FRAME_DURATION_MS) > DURATION_THRESHOLD):
    rpt.failure_stop("Inappropriate strobe width")

rpt.call(mb, mb.reset_strobe_containers())

imu = rpt.call(mb, mb.get_imu_container_info())
body = rpt.call(mb, mb.get_body_container_info())

if rpt.check(body.num_av != 0):
    rpt.failure_stop("Body frCont did not reset")

if rpt.check(imu.num_av != 0):
    rpt.failure_stop("IMU frCont did not reset")

rpt.end_test()
