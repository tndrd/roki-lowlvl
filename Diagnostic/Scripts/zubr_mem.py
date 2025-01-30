import rokilowlvlpy as roki
import rokipytest as rpt

ADDR = 600

VALUE_INT = int(2**31 -1)
VALUE_FLOAT = 0.424242

FLOAT_TOLERANCE = 10**-5

mb = rpt.create_motherboard()

zubr = roki.protocols.Zubr(mb)

rpt.call(zubr, zubr.mem_i_set(ADDR, VALUE_INT))

ret, val = zubr.mem_i_get(ADDR)

if not ret:
    rpt.failure_stop(f"Failed to read data: {zubr.get_error()}")

if rpt.check(val != VALUE_INT):
    rpt.failure_stop(f"Sent and recieved values are different")

rpt.call(zubr, zubr.mem_f_set(ADDR, VALUE_FLOAT))

ret, val = zubr.mem_f_get(ADDR)

if not ret:
    rpt.failure_stop(f"Failed to read data: {zubr.get_error()}")

if rpt.check(abs(val - VALUE_FLOAT) > FLOAT_TOLERANCE):
    rpt.failure_stop("Sent and received values are different")

rpt.end_test()
