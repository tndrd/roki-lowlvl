import rokilowlvlpy as roki
import rokipytest as rpt
import time
import struct

TESTING_TIME_S = 10
UPDATE_PERIOD_MS = 10
SUM_TOLERANCE = 0.2
ROUND_DIGITS = 2

SENSOR_ID = 37

mb = rpt.create_motherboard()

rpt.eprint(f"Running for {TESTING_TIME_S}s...")

for i in range(0, (1000 * TESTING_TIME_S) // UPDATE_PERIOD_MS):

    result, data = mb.get_imu_latest()

    if not result:
        rpt.failure_stop(f"Failed to obtain Head IMU data: {mb.get_error()}")

    x = data.orientation.x
    y = data.orientation.y
    z = data.orientation.z
    w = data.orientation.w

    s = x*x + y*y + z*z + w*w
    
    if rpt.check(abs(s - 1) > SUM_TOLERANCE):
        report = f"Bad quaternion sum: {round(s, ROUND_DIGITS)}\n"
        report += f" x = {round(x, ROUND_DIGITS)}\n"
        report += f" y = {round(y, ROUND_DIGITS)}\n"
        report += f" z = {round(z, ROUND_DIGITS)}\n"
        report += f" w = {round(w, ROUND_DIGITS)}\n"
        rpt.failure_stop(report)

    if rpt.check(data.sensor_id != SENSOR_ID):
        rpt.failure_stop(f"Wrong SensorID: got {data.sensor_id}, expected {SENSOR_ID}")

    time.sleep(UPDATE_PERIOD_MS / 1000)

rpt.end_test()

