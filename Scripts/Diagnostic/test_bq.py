import rokilowlvlpy as roki
import rokipytest as rpt
from threading import Thread
import time

REMAIN_TOLERANCE = 5

def return_servo(rcb):
    sd = roki.protocols.Rcb4.ServoData()
    sd.id = 8
    sd.sio = 0
    sd.data = 7500
    rpt.call(rcb, rcb.set_servo_pos([sd], 5))

def fill_bq(rcb, count, pause):
    sd = roki.protocols.Rcb4.ServoData()
    sd.id = 8
    sd.sio = 0
    
    do_pause = False
    for x in range(count):
        value = 7500 + (-0.5 + x / count) * 1000
        sd.data = int(value)
        pause_val = 0
        if do_pause:
            pause_val = pause
            do_pause = False
        else:
            do_pause = True
        rcb.set_servo_pos_async([sd], 5, pause_val)

def test_bq_period(period, pause = 0, TestReset = False):
    period_old = period
    period = int(period * (92 / 100))

    mb = rpt.create_motherboard()

    rcb = roki.protocols.Rcb4(mb)
    rpt.call(rcb, rcb.check_acknowledge())

    count = rpt.call(mb, mb.get_body_queue_info()).capacity
    rpt.call(mb, mb.set_body_queue_period(period))

    queue_time = count * (period_old / 1000) * (pause / 2 + 1) 
    
    start = time.perf_counter()
    fill_bq(rcb, count, pause)
    filled = time.perf_counter()
    
    if rpt.is_mocking():
        sleep_time = 0
    else:
        sleep_time = queue_time - (filled - start)
    
    if TestReset:
        time.sleep(sleep_time / 2)
        rpt.call(mb, mb.reset_body_queue())
    else:
        time.sleep(sleep_time)

    remains = rpt.call(mb, mb.get_body_queue_info()).size

    if (remains > REMAIN_TOLERANCE):
        return_servo(rcb)
        rpt.failure_stop(f"BQ Period Error: remains {remains} objects")
    return_servo(rcb)

test_bq_period(5, 5)
test_bq_period(5, 0, True)
test_bq_period(10, 1)
test_bq_period(20, 1)
test_bq_period(20, 0, True)
test_bq_period(2)

rpt.end_test()
