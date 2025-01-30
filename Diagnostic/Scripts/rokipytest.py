import sys
import os
import rokilowlvlpy

_USE_MOCKS = False
_USER_NAME = "RokiPyTest"


try:
    from picamera2 import Picamera2
except:
    if not "ROKI_USE_HARDWARE_MOCKS" in os.environ:
        raise
    from picamera2_mock import Picamera2
    _USE_MOCKS = True

def is_mocking(): return _USE_MOCKS

def check(boolean):
    if is_mocking(): return False
    return boolean

def mute_picamera():
    Picamera2.set_logging(Picamera2.ERROR)
    os.environ["LIBCAMERA_LOG_LEVELS"] = "4"

def eprint(msg, **kwargs):
    print(msg, **kwargs, file=sys.stderr)

def failure_stop(msg):
    eprint(msg)
    sys.exit(1)

def end_test():
    sys.exit(0)

def create_motherboard():
    return rokilowlvlpy.create_motherboard(_USER_NAME)

def call(mb, val):
    ret = None

    if type(val) is tuple:
        ret = val[1]
        val = val[0]
    
    if val == False:
        failure_stop(mb.get_error())

    return ret
