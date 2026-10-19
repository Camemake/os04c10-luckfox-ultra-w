#!/bin/sh
ISP=/dev/video11
CROP=/dev/v4l-subdev2
killall v4l2-ctl nv12color mpi_enc_test python3 rkaiq_3A_server simple_vi_get_frame_rkaiq simple_vi_bind_venc_jpeg 2>/dev/null
sleep 1
echo 220 > /sys/bus/i2c/devices/4-0036/exposure
setsid rkaiq_3A_server >/tmp/3a.log 2>&1 </dev/null &
sleep 1
v4l2-ctl -d "$ISP" --set-fmt-video=width=2688,height=1520,pixelformat=NV12 >/tmp/v11fmt.txt 2>&1
v4l2-ctl -d "$CROP" --set-subdev-selection pad=0,target=crop,left=0,top=0,width=2688,height=1520 >/tmp/crop.txt 2>&1
v4l2-ctl -d "$ISP" --set-selection=target=crop,left=0,top=0,width=2688,height=1520 >>/tmp/crop.txt 2>&1
rm -f /tmp/nv.fifo
mkfifo /tmp/nv.fifo
setsid sh -c 'mpi_enc_test -i /tmp/nv.fifo -o /dev/stdout -w 2688 -h 1520 -t 8 -f 0 -n 1000000 2>/tmp/enc.log | python3 /userdata/mjpegd.py' >/tmp/pipe.log 2>&1 </dev/null &
sleep 1
setsid /userdata/nv12color > /tmp/nv.fifo </dev/null &
