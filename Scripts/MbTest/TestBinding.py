import rokilowlvlpy as roki

frame = roki.imu_frame()

_ = frame.orientation.x
_ = frame.orientation.y
_ = frame.orientation.z
_ = frame.orientation.w

_ = frame.timestamp.time_s
_ = frame.timestamp.time_ns
_ = frame.sensor_id

info = roki.FrameContainerInfo()
_ = info.first
_ = info.num_av
_ = info.max_frames

info = roki.body_queue_info()
_ = info.size
_ = info.capacity

_ = roki.Stopbits.One
_ = roki.Stopbits.Two

conf = roki.TTYConfig()
_ = conf.port
_ = conf.baudrate
_ = conf.stopbits
_ = conf.parity_bit
_ = conf.timeout

mb = roki.Motherboard()

mb.configure(conf)
mb.get_imu_frame(0)
mb.get_body_frame(0)
mb.get_imu_container_Info()
mb.get_body_container_Info()
mb.reset_strobe_containerss()
mb.ret_imu__strobe_offset(0)
mb.set_body__strobe_offset(0)
mb.get_imu_latest()
mb.get_strobe_width()
mb.configure_strobe_filter(0,0)
mb.get_body_queue_info()
mb.set_body_queue_period(0)
mb.is_ok()
mb.get_error()
mb.reset_body_queue()

sd = roki.protocols.Rcb4.ServoData()
_ = sd.id
_ = sd.sio
_ = sd.data

rcb = roki.protocols.Rcb4(mb)
rcb.check_acknowledge()
rcb.set_servo_pos([], 0)

rcb.set_servo_pos_async([], 0)
rcb.set_servo_pos_async([], 0, 42)
rcb.read_addr(0x0060, 8)
rcb.motion_play(0)
rcb.get_error()
rcb.is_ok()

zubr = roki.protocols.Zubr(mb)
zubr.mem_i_set(0, 0)
zubr.mem_f_set(0, 0)
zubr.mem_i_get(0)
zubr.mem_f_Get(0)

try:
    mb = roki.create_motherboard()
except NameError: raise
except Exception: pass