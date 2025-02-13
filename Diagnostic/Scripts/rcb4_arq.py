import rokilowlvlpy as roki
import rokipytest as rpt

mb = rpt.create_motherboard()
rcb = roki.protocols.Rcb4(mb)

rpt.call(rcb, rcb.enable_arq(42))
rpt.call(mb, mb.disable_arq())