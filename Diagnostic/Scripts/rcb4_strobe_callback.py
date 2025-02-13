import rokilowlvlpy as roki
import rokipytest as rpt

mb = rpt.create_motherboard()
rcb = roki.protocols.Rcb4(mb)

rpt.call(rcb, rcb.enable_strobe_callback())
rpt.call(mb, mb.reset_body_strobe_callback())