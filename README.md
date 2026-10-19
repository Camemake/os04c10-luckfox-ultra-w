# OS04C10 on the Luckfox Pico Ultra W

Driver, device tree, and board programs for the Camemake OmniVision OS04C10 module on the Luckfox Pico Ultra W. The SoC is the Rockchip **RV1106G3**. The camera port on this kit is MIPI CSI-2, two lanes. The SCCB/I2C bus on that port is **1.8 V**.

This module is not a stock item on the Camemake site. Module design, a matching IQ file, and a connector built for a given host are covered by [Camemake custom camera services](https://www.camemake.eu/services-capabilities). The company site is [camemake.eu](https://www.camemake.eu/).

## I2C on this kit is 1.8 V

Most small CSI kits, including the usual Raspberry Pi camera connector, run the sensor control bus at **3.3 V**. This board does not.

On the Luckfox Pico Ultra W schematic the camera I2C nets are `MIPI_I2C_SCL` and `MIPI_I2C_SDA`. Each one has a **4.7 kΩ pull-up to `VCC_1V8`**. They are not pulled up to 3.3 V. Those pins sit on the RV1106 **VCCIO7** bank. The pin list printed on the Luckfox schematic marks VCCIO7 as **1.8 V only**.

A module whose SCCB pins are pulled to 3.3 V will not talk on this connector, and driving 3.3 V into these pins is outside the bank rating. The 3.3 V rail is present on the same FFC, for module power. It is a different net from the I2C pins.

The device tree models the sensor I/O supply (`dovdd`) as 1.8 V, the analog supply (`avdd`) as 2.8 V, and the core supply (`dvdd`) as 1.2 V.

## Development kit

| | |
|---|---|
| Board | Luckfox Pico Ultra W |
| SoC | Rockchip RV1106G3 |
| CPU | ARM Cortex-A7, one core, up to 1.2 GHz |
| ISP | Rockchip rkisp. Luckfox rates the chip at a 5 megapixel input, 30 fps. This mode is 2688×1520. |
| VICAP | Rockchip rkcif, driver `rkcif` |
| MIPI PHY | Rockchip CSI-2 D-PHY. This board wires two data lanes. |
| Memory | 256 MB DDR3L |
| Storage | 8 GB eMMC |
| NPU | 1 TOPS (int4 / int8 / int16), not used by this driver |
| Wireless | 2.4 GHz Wi-Fi 6 and Bluetooth 5.2 on the Ultra W |
| Camera connector | H1, 20-pin, 0.5 mm FFC (`FPC-0.5mm-20pin` on the schematic) |
| System | Luckfox official Buildroot image for the Pico Ultra W eMMC. Luckfox documents the CSI path for Buildroot. |
| Kernel this module was built for | `5.10.160`, vermagic `5.10.160 mod_unload ARMv7 thumb2 p2v8` |
| Login on that image | `root` / `luckfox` |

Board page: [Luckfox Pico Ultra](https://wiki.luckfox.com/Luckfox-Pico-Ultra/). Schematic: [Luckfox Pico Ultra W PDF](https://files.luckfox.com/wiki/Luckfox-Pico/PDF/Luckfox%20Pico%20Ultra%20W.pdf). Rockchip: [rock-chips.com](https://www.rock-chips.com/).

The stock device tree enables a SmartSens SC3336 on this same CSI port, and has a MIS5001 node beside it. Both are disabled by `dts/rv1106-luckfox-pico-ultra-w-os04c10.dtsi` so they release GPIO3_C5 and the D-PHY endpoint.

## Sensor

OmniVision OS04C10, programmed from the 2-lane register table `OS04C10_MIPI_2L`.

| Item | Value |
|---|---|
| Active size | 2688 × 1520 |
| Bayer | BGGR |
| Bus format | `MEDIA_BUS_FMT_SBGGR10_1X10` |
| CSI-2 lanes | 2 (`data-lanes = <1 2>`) |
| Lane bit rate | 1000 Mbps. Register `0x4837 = 0x08` is a unit interval of 1/8 ns × 8 = 1.0 ns. |
| Link frequency | 500 MHz (half the lane bit rate) |
| Pixel rate | 200 MHz |
| Horizontal total | 4280 pixel periods. The table writes HTS `0x085c` = 2140 counts, and each count is two pixels at this width. |
| Vertical total | 1574 lines (`0x0626`) |
| Sensor frame rate | about 29.7 fps |
| CSI clock | non-continuous |
| XVCLK the table was captured at | 12 MHz |
| XVCLK on this board | `MCLK_REF_MIPI0`. The running board clock is 24 MHz. The driver warns and keeps going. The link rate above is the rate the registers program. |
| Control bus | I2C / SCCB, 7-bit address `0x36`, 16-bit register address, 8-bit data |
| Controller | I2C4, pin mux `i2c4m2` |
| Chip ID | `0x300A = 0x53`, `0x300B = 0x04` |
| Stream | `0x0100`: `0x01` streams, `0x00` stops |
| Exposure | `0x3500`..`0x3502`, 24-bit. Sysfs clamps the value to 8 .. `0x0F00` (3840). |
| Analogue gain | sysfs clamps the value to `0x10` .. `0x07FF` |
| Table delays | a row with address `0xFFFF` is a delay of that many milliseconds, not a register write |
| Module name in the device tree | `OS04C10` |
| Compatible | `ovti,os04c10` |

The ISP on this board delivers the full frame at **15 fps**. A 90-frame capture with `rk_mpi_vi_test` measured 15.06 fps. `v4l2-ctl` on the ISP node, writing to `/dev/null`, measured 15.0 fps. Asking the pipeline for 20 fps does not produce 20 fps. The sensor timing is ~29.7 fps; the ISP readback path on this kit is the 15 fps limit.

## Connector pinout

Connector **H1** on the Pico Ultra W is the camera FFC: **20 pins, 0.5 mm pitch**. The metal contacts of the cable face the RV1106. Power the board off before seating the cable.

The schematic names the nets. It does not print a 1-to-20 table in a form that can be copied without the drawing, so this table is the net list, the SoC function, and the voltage. Pin order is the H1 footprint on the [published schematic](https://files.luckfox.com/wiki/Luckfox-Pico/PDF/Luckfox%20Pico%20Ultra%20W.pdf), sheet section **CAM**.

| Net on H1 | RV1106 function | GPIO | Voltage |
|---|---|---|---|
| `MIPI_I2C_SCL` | `I2C4_SCL_M2` | GPIO3_C7 | **1.8 V**, 4.7 kΩ to `VCC_1V8` |
| `MIPI_I2C_SDA` | `I2C4_SDA_M2` | GPIO3_D0 | **1.8 V**, 4.7 kΩ to `VCC_1V8` |
| `MIPI/LVDS_MCLK0` | `MIPI_CLK0_OUT` | GPIO3_C4 | 1.8 V bank, sensor XVCLK |
| `MIPI/LVDS_RST` | `VI_CIF_VSYNC_M0` | GPIO3_C5 | 1.8 V bank. Active-high in this device tree. |
| `MIPI/LVDS_CLK_P` | `MIPI_CSI_RX_CLK0P` | GPIO3_C1 | MIPI HS pair |
| `MIPI/LVDS_CLK_N` | `MIPI_CSI_RX_CLK0N` | GPIO3_C0 | MIPI HS pair |
| `MIPI/LVDS_D0_P` | `MIPI_CSI_RX_D0P` | GPIO3_C3 | MIPI HS pair, lane 1 |
| `MIPI/LVDS_D0_N` | `MIPI_CSI_RX_D0N` | GPIO3_C2 | MIPI HS pair, lane 1 |
| `MIPI/LVDS_D1_P` | `MIPI_CSI_RX_D1P` | GPIO3_B7 | MIPI HS pair, lane 2 |
| `MIPI/LVDS_D1_N` | `MIPI_CSI_RX_D1N` | GPIO3_B6 | MIPI HS pair, lane 2 |
| `VCC_3V3` | board 3.3 V | | 3.3 V module power |
| `GND` | ground | | 0 V |

The RV1106 also has CSI lane 2, lane 3, a second clock pair, `I2C3`, and `MIPI_CLK1_OUT` on the same I/O bank. Those nets are not the two-lane camera port this board brings to H1. The device tree programs lanes 1 and 2 only.

Reset is `reset-gpios = <&gpio3 RK_PC5 GPIO_ACTIVE_HIGH>`. `RK_PC5` is GPIO3_C5. The driver holds that GPIO high while it talks to the sensor. Flipping the flag to active-low removes the acknowledge at address `0x36`.

`pwdn-gpios` is optional. This connector does not require a second power-down GPIO.

## What is in this tree

```
dts/rv1106-luckfox-pico-ultra-w-os04c10.dtsi   sensor node for the Ultra W
driver/os04c10.c                               kernel module source
driver/Makefile                                out-of-tree build against the Luckfox kernel
prebuilt/os04c10.ko                            module for kernel 5.10.160, 17036 bytes
board/S99os04c10                               loads the module from /userdata at boot
board/iq-link.sh                               points rkaiq at a starting IQ file
board/nv12color.c                              ISP capture and UV correction
board/Makefile                                 build of nv12color
prebuilt/nv12color                             that program, already built
board/start-live.sh                            15 fps MJPEG path
board/mjpegd.py                                multipart JPEG server on port 8090
board/live.html                                page served by mjpegd.py
```

`prebuilt/os04c10.ko` matches vermagic `5.10.160 mod_unload ARMv7 thumb2 p2v8`, including the trailing space. Load it on the official Ultra W Buildroot kernel. A different kernel needs a rebuild from `driver/`.

## Device tree

Include `dts/rv1106-luckfox-pico-ultra-w-os04c10.dtsi` from `rv1106g-luckfox-pico-ultra-w.dts`, after `rv1106-luckfox-pico-ultra-ipc.dtsi`.

The fragment does four things:

- Declares fixed regulators `vcc_cam_avdd` (2.8 V), `vcc_cam_dovdd` (1.8 V), `vcc_cam_dvdd` (1.2 V).
- Sets `sc3336` and `mis5001` to `disabled`.
- Points `csi_dphy_input0` at the OS04C10 endpoint, with `data-lanes = <1 2>`.
- Adds `os04c10@36` on `&i2c4` with `pinctrl-0 = <&i2c4m2_xfer>`, clock `MCLK_REF_MIPI0`, `assigned-clock-rates = <12000000>`, reset GPIO3_C5 active-high, and `rockchip,camera-module-name = "OS04C10"`.

On the Ultra series, I2C4 is shared with the CSI wires. Luckfox enables CSI by setting `pinctrl-0` of `&i2c4` to `<&i2c4m2_xfer>` and by leaving `&csi2_dphy_hw` enabled. A pinctrl that also claims `i2c4m0` or `i2c4m1` moves those pins off the camera connector. CSI changes in `luckfox-config` take effect after a reboot.

The boot image has to contain this node. The stock boot image has the SC3336 node, not the OS04C10 node. Rebuild the boot FIT from the Luckfox SDK after the include, and write that boot image with the Luckfox / Rockchip upgrade tool. The running kernel reads the DTB from the boot partition.

If the module has its own 12 MHz crystal on the clock pin, drop `pinctrl-0 = <&mipi_refclk_out0>` and point `clocks` at a fixed 12 MHz clock instead of `MCLK_REF_MIPI0`. OS04C10 on this kit uses the board clock.

## Build the module

The source needs the Luckfox SDK kernel tree that produced `5.10.160`, because it includes `linux/rk-camera-module.h` and `phy-rockchip-csi2-dphy-common.h`.

```sh
cd driver
make KDIR=/path/to/luckfox-pico/sysdrv/source/kernel
```

The cross compiler is the one the SDK uses for that kernel (`arm-linux-gnueabihf-gcc` or the SDK’s `arm-rockchip830-linux-uclibcgnueabihf-gcc`). `ARCH=arm CROSS_COMPILE=...` are passed through `make` the same way as any other out-of-tree module.

`modules.order` and the intermediate objects stay out of git. The file to install is `os04c10.ko`.

The module’s probe identifies the chip, registers the V4L2 subdev under the Rockchip name `m00_b_os04c10 4-0036`, attaches the CSI-2 D-PHY, and registers the sensor with the rkisp sideband interface. That attach is part of `insmod`. Do not unbind `rkisp`, and do not rebind `csi2-dphy0`, after the module is loaded. Unbinding rkisp on this kernel faults in the async notifier, and rebinding the D-PHY drops the media link this module just made.

The driver leaves the sensor powered. A runtime suspend that drops reset does not bring I2C back on the following resume.

## Install on the board

Copy these files to `/userdata` on the board:

```
prebuilt/os04c10.ko
prebuilt/nv12color
board/mjpegd.py
board/live.html
board/start-live.sh
board/iq-link.sh
```

Install the init script:

```sh
cp board/S99os04c10 /etc/init.d/S99os04c10
chmod 755 /etc/init.d/S99os04c10 /userdata/os04c10.ko /userdata/nv12color /userdata/start-live.sh /userdata/iq-link.sh
```

`S99os04c10` runs `insmod /userdata/os04c10.ko`. After the next boot, or after `insmod` by hand:

```sh
ls /sys/bus/i2c/devices/4-0036
dmesg | grep os04c10
media-ctl -p -d /dev/media0
media-ctl -p -d /dev/media1
v4l2-ctl --list-devices
```

The sensor node is `/sys/bus/i2c/devices/4-0036`. A successful probe prints `os04c10: sensor ready`. The chip-id check reads `0x53` then `0x04`. While the driver owns the client, `i2c-tools` on that address returns `EBUSY`. Exposure and gain go through sysfs:

```sh
echo 220 > /sys/bus/i2c/devices/4-0036/exposure
echo 128 > /sys/bus/i2c/devices/4-0036/analogue_gain
```

`220` is the exposure used for an indoor scene with this lens. Values at `0x0340` and above clip the window in a bright room. The files accept decimal or `0x` hex. Exposure is limited to 8 .. 3840. Gain is limited to 16 .. 2047.

## Video nodes

On the official Ultra W Buildroot image, after this module is loaded:

| Node | What it is | Format |
|---|---|---|
| sensor subdev | `m00_b_os04c10 4-0036` | one source pad, SBGGR10 2688×1520 |
| `/dev/video0` | rkcif capture | V4L2 `BG10`, 2688×1520, `bytesperline` 3584, `sizeimage` 5447680 |
| `/dev/video11` | rkisp mainpath | NV12, 2688×1520, `bytesperline` 2688, `sizeimage` 6128640 |
| `/dev/v4l-subdev2` | rkisp ISP subdev | sink SBGGR10_1X10 2688×1520 |

Confirm the numbers with `v4l2-ctl --list-devices` if the graph has extra cameras. `board/start-live.sh` uses `/dev/video11` and `/dev/v4l-subdev2`, which are the nodes on this image.

`v4l2-ctl -d /dev/video11 --list-formats-ext` reports a stepwise maximum of 800×600. That enumeration is wrong for this pipeline. `VIDIOC_G_FMT` and the frames that come out are 2688×1520. The ISP crop, however, really does default to `(0,0)` / 800×600, which is a magnified corner of the sensor. Set both crops before capturing:

```sh
v4l2-ctl -d /dev/video11 --set-fmt-video=width=2688,height=1520,pixelformat=NV12
v4l2-ctl -d /dev/v4l-subdev2 --set-subdev-selection pad=0,target=crop,left=0,top=0,width=2688,height=1520
v4l2-ctl -d /dev/video11 --set-selection=target=crop,left=0,top=0,width=2688,height=1520
```

`media-ctl -V` on the ISP media device returns `EINVAL` on this image. The selection ioctls above are the ones that stick.

### CIF RAW10 packing

`/dev/video0` is packed RAW10, four pixels in five bytes, least significant bits first. This is the Rockchip CIF compact layout, not the MIPI long-packet layout where a fifth byte holds the two low bits of four pixels.

```
p0 = b0 | ((b1 & 0x03) << 8)
p1 = (b1 >> 2) | ((b2 & 0x0f) << 6)
p2 = (b2 >> 4) | ((b3 & 0x3f) << 4)
p3 = (b3 >> 6) | (b4 << 2)
```

BGGR at pixel `(x, y)`: blue when `x` and `y` are even, red when both are odd, green on the other two sites.

### ISP NV12

One frame is `2688 * 1520 * 3 / 2 = 6128640` bytes. The Y plane is the first `2688 * 1520 = 4085760` bytes. The chroma plane is the remaining 2042880 bytes, interleaved U then V.

## Color

With no 3A server, the ISP output is a green-tinted luma image. Chroma is almost constant. White balance on that buffer cannot recover the chart.

`rkaiq_3A_server` looks up the IQ files from the sensor name and the device-tree module name:

```
/etc/iqfiles/os04c10_OS04C10_default.json
/etc/iqfiles/os04c10_OS04C10_default.bin
```

On the official image `/etc/iqfiles` points at `/oem/usr/share/iqfiles`. That directory has no IQ file for this module. `board/iq-link.sh` links the two names above to the OS04A10 files that the image does ship:

```
os04a10_CMK-OT1607-PV1_50IRC-F16.json
os04a10_CMK-OT1607-PV1_50IRC-F16.bin
```

That is enough for the ISP to demosaic and produce a color picture. It is an OS04A10 calibration. Editing `CALIB_WB_MODE` or `mwbGain` in the JSON does not change the picture, because the running 3A path applies the `.bin`. A calibration of this module, this lens, and this IR filter is part of a Camemake custom camera build: [services](https://www.camemake.eu/services-capabilities).

`/oem` has to be writable for the two symlinks. If the link step fails, remount that partition read-write, run `iq-link.sh` again, and start `rkaiq_3A_server` after the names exist. The server logs `invalid main scene len`, a failed horizontal-flip set, and a failed exposure set on start of frame. Those messages are non-fatal. Exposure written to sysfs still applies. Do not pass `--help` to `rkaiq_3A_server`; that path faults.

`nv12color` is the correction used on top of the OS04A10 starting file. It captures `/dev/video11` itself (four MMAP buffers, multiplanar), subtracts **8 from every U byte and 18 from every V byte** with saturating NEON arithmetic, and writes the 6128640-byte frame to stdout. One process is the whole capture path. A second process on the same frames costs about a frame per second on the single Cortex-A7.

Replace `nv12color` with `v4l2-ctl --stream-mmap --stream-to=-` when a real OS04C10 IQ file is installed and the UV shift is no longer wanted.

## Live picture

`board/start-live.sh` does the following, in order:

1. Stops a previous copy of the pipeline.
2. Sets exposure to 220.
3. Starts `rkaiq_3A_server`.
4. Sets the ISP format and both crops to 2688×1520.
5. Starts `mpi_enc_test` (Rockchip MPP) reading NV12 from `/tmp/nv.fifo` and writing JPEG to stdout. Type `-t 8` is MJPEG. Pixel format `-f 0` is NV12.
6. Pipes that JPEG stream into `python3 /userdata/mjpegd.py`.
7. Starts `/userdata/nv12color` writing the fifo.

MPP is the encoder. `ffmpeg` on this image has a software MJPEG encoder at about 1.5 fps for this resolution, and its `h264_v4l2m2m` device is absent. `/dev/mpp_service` is the device `mpi_enc_test` uses. There is no `/dev/video21` encoder node on this image.

`mjpegd.py` listens on `0.0.0.0:8090`.

| URL | Body |
|---|---|
| `/` and `/live.html` | `board/live.html` from `/userdata/live.html` |
| `/stream.mjpg` | `multipart/x-mixed-replace`, boundary `frame` |
| `/frame.jpg` and `/shot.jpg` | the latest JPEG |
| `/fps` | frames adopted over a window of at least two seconds |

From a host that reaches the board with `adb`:

```sh
adb forward tcp:8090 tcp:8090
```

Open `http://127.0.0.1:8090/live.html`. The page points the image at `stream.mjpg` on the same host and prints the rate from `/fps`.

Measured rates on this kit, 2688×1520:

| Path | Rate |
|---|---|
| ISP capture, `v4l2-ctl` to `/dev/null` | 15.0 fps |
| `rk_mpi_vi_test`, 90 frames, no encode | 15.06 fps |
| This pipeline, no browser client | 15.0 to 15.2 fps |
| This pipeline with one browser client on the MJPEG URL | about 11 to 12 fps |

The browser figure is the USB forward moving JPEGs of roughly 230 KB. 15 fps at that size is about 3.5 MB/s, which is past what the forward sustains. The encoder’s `-qc` and `-ini` switches on `mpi_enc_test` do not change that packet size; the ini path is not implemented in this copy of the tool. The rate on `/fps` with nothing connected to `/stream.mjpg` is the ISP rate.

`/tmp` is an 85 MB tmpfs. This pipeline uses a fifo, so it does not fill `/tmp`. A capture tool that writes a JPEG per frame into `/tmp` fills that filesystem in under a minute at 15 fps.

## Rebuild nv12color

```sh
cd board
make
```

`arm-none-eabi-gcc` flags are `-marm -O3 -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=softfp -nostdlib -nostartfiles -fno-builtin -lgcc -Wl,-e,_start`. The program is freestanding. Entry is `_start`. `prebuilt/nv12color` is that binary, 6116 bytes.

The ioctl sizes are the 32-bit ARM layout of kernel 5.10: `v4l2_requestbuffers` is 20 bytes, `v4l2_buffer` is 68 bytes, buffer type multiplanar capture is 9, memory MMAP is 1. `VIDIOC_REQBUFS` is `0xC0145608`.

## Rockchip software used here

| Piece | Role |
|---|---|
| RV1106G3 | SoC. CIF input, ISP, MPP encoder, I2C4, GPIO3, CRU clock `MCLK_REF_MIPI0`. |
| Kernel 5.10.160 | Luckfox Buildroot kernel. The out-of-tree module has to match its vermagic. |
| `rkcif` | Writes packed RAW10 to the CIF video node. |
| `rkisp` / `rkisp-vir0` | Demosaic and NV12 output. The driver registers the sensor on the sditf port of this ISP. |
| `rkaiq_3A_server` | Rockchip 3A. Requires the two IQ filenames above. |
| `mpi_enc_test` | Rockchip MPP sample in `/oem/usr/bin`. JPEG from NV12. |
| `v4l2-ctl`, `media-ctl` | Format, crop, and graph. Shipped on the image. |

A module-specific IQ file, a lens, and a mechanical design for a product are outside this repository. That work is [Camemake custom camera services](https://www.camemake.eu/services-capabilities).
