import rokilowlvlpy as roki
import rokipytest as rpt
import time
import math

params = roki.protocols.SKServo.Params

AMPLITUDE = 1000
FREQUENCY = 6e-2 / 6.28
DURATION_MS = 2000

mb = rpt.create_motherboard()
sks = roki.protocols.SKServo(mb)

ids = []

for i in range(0, 16):
    ok, _ = sks.get_param(i, params.SIGNATURE)
    if ok:
        ids.append(i)

if len(ids) == 0:
    rpt.failure_stop("No servos detected")

pos = 0
x = 0

while (x < DURATION_MS):
    for id in ids:
        rpt.call(sks, sks.set_position(id, pos))

    pos = int(AMPLITUDE * math.sin(x * FREQUENCY))
    x += 1

    if (x == DURATION_MS - 1):
        pos = 0

    time.sleep(1e-3)

time.sleep(1)

for id in ids:
    rpt.call(sks, sks.set_free(id))

time.sleep(1)

for id in ids:
    rpt.call(sks, sks.set_param(id, params.SERVO_TARGET_VAL, 0))

for id in ids:
    rsp = sks.get_param(id, params.SERVO_TARGET_VAL)
    rpt.call(sks, rsp)

    if (rsp[1].value != 0):
        rpt.failure_stop("Bad parameter value")

time.sleep(1)

for id in ids:
    rpt.call(sks, sks.set_free(id))

rpt.end_test()
