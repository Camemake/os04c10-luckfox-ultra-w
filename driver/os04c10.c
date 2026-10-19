// SPDX-License-Identifier: GPL-2.0

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/version.h>
#include <linux/rk-camera-module.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>
#include <phy-rockchip-csi2-dphy-common.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
#include <linux/unaligned.h>
#else
#include <asm/unaligned.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
#define v4l2_subdev_state v4l2_subdev_pad_config
#endif

#define OS04C10_XCLK_FREQ		12000000UL
#define OS04C10_LINK_FREQ_HZ	500000000ULL
#define OS04C10_PIXEL_RATE		200000000ULL
#define OS04C10_WIDTH		2688
#define OS04C10_HEIGHT		1520
#define OS04C10_LANES		2
#define OS04C10_STREAM_REG		0x0100
#define OS04C10_STREAM_ON		0x01
#define OS04C10_STREAM_OFF		0x00
#define OS04C10_DELAY_MARKER	0xffff

struct os04c10_reg {
	u16 address;
	u8 value;
};

static const struct os04c10_reg os04c10_mode_2688x1520_regs[] = {
	{ 0x0100, 0x00 },
	{ 0xffff, 0x0a },
	{ 0x0103, 0x01 },
	{ 0xffff, 0x0a },
	{ 0x0301, 0x84 },
	{ 0x0303, 0x01 },
	{ 0x0305, 0x3c },
	{ 0x0306, 0x00 },
	{ 0x0307, 0x17 },
	{ 0x0323, 0x04 },
	{ 0x0324, 0x01 },
	{ 0x0325, 0x62 },
	{ 0x3012, 0x06 },
	{ 0x3013, 0x02 },
	{ 0x3016, 0x32 },
	{ 0x3021, 0x03 },
	{ 0x3106, 0x25 },
	{ 0x3107, 0xa1 },
	{ 0x3500, 0x00 },
	{ 0x3501, 0x00 },
	{ 0x3502, 0x80 },
	{ 0x3503, 0x88 },
	{ 0x3508, 0x00 },
	{ 0x3509, 0x80 },
	{ 0x350a, 0x04 },
	{ 0x350b, 0x00 },
	{ 0x350c, 0x00 },
	{ 0x350d, 0x80 },
	{ 0x350e, 0x04 },
	{ 0x350f, 0x00 },
	{ 0x3510, 0x00 },
	{ 0x3511, 0x00 },
	{ 0x3512, 0x20 },
	{ 0x3624, 0x02 },
	{ 0x3625, 0x4c },
	{ 0x3660, 0x00 },
	{ 0x3666, 0xa5 },
	{ 0x3667, 0xa5 },
	{ 0x366a, 0x64 },
	{ 0x3673, 0x0d },
	{ 0x3672, 0x0d },
	{ 0x3671, 0x0d },
	{ 0x3670, 0x0d },
	{ 0x3685, 0x00 },
	{ 0x3694, 0x0d },
	{ 0x3693, 0x0d },
	{ 0x3692, 0x0d },
	{ 0x3691, 0x0d },
	{ 0x3696, 0x4c },
	{ 0x3697, 0x4c },
	{ 0x3698, 0x40 },
	{ 0x3699, 0x80 },
	{ 0x369a, 0x18 },
	{ 0x369b, 0x1f },
	{ 0x369c, 0x14 },
	{ 0x369d, 0x80 },
	{ 0x369e, 0x40 },
	{ 0x369f, 0x21 },
	{ 0x36a0, 0x12 },
	{ 0x36a1, 0x5d },
	{ 0x36a2, 0x66 },
	{ 0x370a, 0x00 },
	{ 0x370e, 0x0c },
	{ 0x3710, 0x00 },
	{ 0x3713, 0x00 },
	{ 0x3725, 0x02 },
	{ 0x372a, 0x03 },
	{ 0x3738, 0xce },
	{ 0x3739, 0x10 },
	{ 0x3748, 0x00 },
	{ 0x374a, 0x00 },
	{ 0x374c, 0x00 },
	{ 0x374e, 0x00 },
	{ 0x3756, 0x00 },
	{ 0x3757, 0x0e },
	{ 0x3767, 0x00 },
	{ 0x3771, 0x00 },
	{ 0x377b, 0x20 },
	{ 0x377c, 0x00 },
	{ 0x377d, 0x0c },
	{ 0x3781, 0x03 },
	{ 0x3782, 0x00 },
	{ 0x3789, 0x14 },
	{ 0x3795, 0x02 },
	{ 0x379c, 0x00 },
	{ 0x379d, 0x00 },
	{ 0x37b8, 0x04 },
	{ 0x37ba, 0x03 },
	{ 0x37bb, 0x00 },
	{ 0x37bc, 0x04 },
	{ 0x37be, 0x08 },
	{ 0x37c4, 0x11 },
	{ 0x37c5, 0x80 },
	{ 0x37c6, 0x14 },
	{ 0x37c7, 0x08 },
	{ 0x37da, 0x11 },
	{ 0x381f, 0x08 },
	{ 0x3829, 0x03 },
	{ 0x3832, 0x00 },
	{ 0x3881, 0x00 },
	{ 0x3888, 0x04 },
	{ 0x388b, 0x00 },
	{ 0x3c80, 0x10 },
	{ 0x3c86, 0x00 },
	{ 0x3c9f, 0x01 },
	{ 0x3d85, 0x1b },
	{ 0x3d8c, 0x71 },
	{ 0x3d8d, 0xe2 },
	{ 0x3f00, 0x0b },
	{ 0x3f06, 0x04 },
	{ 0x400a, 0x01 },
	{ 0x400b, 0x50 },
	{ 0x400e, 0x08 },
	{ 0x4040, 0x00 },
	{ 0x4041, 0x07 },
	{ 0x4043, 0x7e },
	{ 0x4045, 0x7e },
	{ 0x4047, 0x7e },
	{ 0x4049, 0x7e },
	{ 0x4090, 0x14 },
	{ 0x40b0, 0x00 },
	{ 0x40b1, 0x00 },
	{ 0x40b2, 0x00 },
	{ 0x40b3, 0x00 },
	{ 0x40b4, 0x00 },
	{ 0x40b5, 0x00 },
	{ 0x40b7, 0x00 },
	{ 0x40b8, 0x00 },
	{ 0x40b9, 0x00 },
	{ 0x40ba, 0x00 },
	{ 0x4301, 0x00 },
	{ 0x4303, 0x00 },
	{ 0x4502, 0x04 },
	{ 0x4503, 0x00 },
	{ 0x4504, 0x06 },
	{ 0x4506, 0x00 },
	{ 0x4507, 0x64 },
	{ 0x4803, 0x10 },
	{ 0x480c, 0x32 },
	{ 0x480e, 0x00 },
	{ 0x4813, 0x00 },
	{ 0x4819, 0x70 },
	{ 0x481f, 0x30 },
	{ 0x4823, 0x3c },
	{ 0x4825, 0x32 },
	{ 0x4833, 0x10 },
	{ 0x484b, 0x07 },
	{ 0x488b, 0x00 },
	{ 0x4d00, 0x04 },
	{ 0x4d01, 0xad },
	{ 0x4d02, 0xbc },
	{ 0x4d03, 0xa1 },
	{ 0x4d04, 0x1f },
	{ 0x4d05, 0x4c },
	{ 0x4d0b, 0x01 },
	{ 0x4e00, 0x2a },
	{ 0x4e0d, 0x00 },
	{ 0x5001, 0x09 },
	{ 0x5004, 0x00 },
	{ 0x5080, 0x04 },
	{ 0x5036, 0x00 },
	{ 0x5180, 0x70 },
	{ 0x5181, 0x10 },
	{ 0x520a, 0x03 },
	{ 0x520b, 0x06 },
	{ 0x520c, 0x0c },
	{ 0x580b, 0x0f },
	{ 0x580d, 0x00 },
	{ 0x580f, 0x00 },
	{ 0x5820, 0x00 },
	{ 0x5821, 0x00 },
	{ 0x301c, 0xf0 },
	{ 0x301e, 0xb4 },
	{ 0x301f, 0xd0 },
	{ 0x3022, 0x01 },
	{ 0x3109, 0xe7 },
	{ 0x3600, 0x00 },
	{ 0x3610, 0x65 },
	{ 0x3611, 0x85 },
	{ 0x3613, 0x3a },
	{ 0x3615, 0x60 },
	{ 0x3621, 0x90 },
	{ 0x3620, 0x0c },
	{ 0x3629, 0x00 },
	{ 0x3661, 0x04 },
	{ 0x3662, 0x10 },
	{ 0x3664, 0x70 },
	{ 0x3665, 0x00 },
	{ 0x3681, 0xa6 },
	{ 0x3682, 0x53 },
	{ 0x3683, 0x2a },
	{ 0x3684, 0x15 },
	{ 0x3700, 0x2a },
	{ 0x3701, 0x12 },
	{ 0x3703, 0x28 },
	{ 0x3704, 0x0e },
	{ 0x3706, 0x4a },
	{ 0x3709, 0x4a },
	{ 0x370b, 0xa2 },
	{ 0x370c, 0x01 },
	{ 0x370f, 0x04 },
	{ 0x3714, 0x24 },
	{ 0x3716, 0x24 },
	{ 0x3719, 0x11 },
	{ 0x371a, 0x1e },
	{ 0x3720, 0x00 },
	{ 0x3724, 0x13 },
	{ 0x373f, 0xb0 },
	{ 0x3741, 0x4a },
	{ 0x3743, 0x4a },
	{ 0x3745, 0x4a },
	{ 0x3747, 0x4a },
	{ 0x3749, 0xa2 },
	{ 0x374b, 0xa2 },
	{ 0x374d, 0xa2 },
	{ 0x374f, 0xa2 },
	{ 0x3755, 0x10 },
	{ 0x376c, 0x00 },
	{ 0x378d, 0x30 },
	{ 0x3790, 0x4a },
	{ 0x3791, 0xa2 },
	{ 0x3798, 0x40 },
	{ 0x379e, 0x00 },
	{ 0x379f, 0x04 },
	{ 0x37a1, 0x10 },
	{ 0x37a2, 0x1e },
	{ 0x37a8, 0x10 },
	{ 0x37a9, 0x1e },
	{ 0x37ac, 0xa0 },
	{ 0x37b9, 0x01 },
	{ 0x37bd, 0x01 },
	{ 0x37bf, 0x26 },
	{ 0x37c0, 0x11 },
	{ 0x37c2, 0x04 },
	{ 0x37cd, 0x19 },
	{ 0x37d8, 0x02 },
	{ 0x37d9, 0x08 },
	{ 0x37e5, 0x02 },
	{ 0x3800, 0x00 },
	{ 0x3801, 0x00 },
	{ 0x3802, 0x00 },
	{ 0x3803, 0x00 },
	{ 0x3804, 0x0a },
	{ 0x3805, 0x8f },
	{ 0x3806, 0x05 },
	{ 0x3807, 0xff },
	{ 0x3808, 0x0a },
	{ 0x3809, 0x80 },
	{ 0x380a, 0x05 },
	{ 0x380b, 0xf0 },
	{ 0x380c, 0x08 },
	{ 0x380d, 0x5c },
	{ 0x380e, 0x06 },
	{ 0x380f, 0x26 },
	{ 0x3811, 0x08 },
	{ 0x3813, 0x08 },
	{ 0x3814, 0x01 },
	{ 0x3815, 0x01 },
	{ 0x3816, 0x01 },
	{ 0x3817, 0x01 },
	{ 0x3820, 0x88 },
	{ 0x3821, 0x00 },
	{ 0x3880, 0x25 },
	{ 0x3882, 0x20 },
	{ 0x3c91, 0x0b },
	{ 0x3c94, 0x45 },
	{ 0x4000, 0xf3 },
	{ 0x4001, 0x60 },
	{ 0x4003, 0x40 },
	{ 0x4008, 0x02 },
	{ 0x4009, 0x0d },
	{ 0x4300, 0xff },
	{ 0x4302, 0x0f },
	{ 0x4305, 0x83 },
	{ 0x4505, 0x84 },
	{ 0x4800, 0x64 },
	{ 0x4809, 0x1e },
	{ 0x480a, 0x04 },
	{ 0x4837, 0x08 },
	{ 0x4c00, 0x08 },
	{ 0x4c01, 0x00 },
	{ 0x4c04, 0x00 },
	{ 0x4c05, 0x00 },
	{ 0x5000, 0xe9 },
	{ 0x0100, 0x01 },
};

static const struct os04c10_reg os04c10_id_checks[] = {
	{ 0x300a, 0x53 },
	{ 0x300b, 0x04 },
};
static const u8 os04c10_id_masks[] = {
	0xff,
	0xff,
};

static const char * const os04c10_supply_names[] = {
	"avdd", "dovdd", "dvdd"
};

static const s64 os04c10_link_freq_menu[] = {
	500000000LL,
};

static u32 os04c10_exposure = 0x80;
static u32 os04c10_gain = 0x80;

struct os04c10 {
	struct i2c_client *client;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_ctrl_handler ctrl_handler;
	struct clk *xclk;
	struct regulator_bulk_data supplies[3];
	struct gpio_desc *reset_gpio;
	struct gpio_desc *pwdn_gpio;
	struct v4l2_mbus_framefmt fmt;
	bool streaming;
};

#define to_os04c10(_sd) container_of(_sd, struct os04c10, sd)

static int os04c10_read_reg(struct os04c10 *priv, u16 address, u8 *value)
{
	struct i2c_client *client = priv->client;
	u8 addr[2];
	u8 data[1];
	struct i2c_msg msgs[2];
	int ret;

	put_unaligned_be16(address, addr);
	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = sizeof(addr);
	msgs[0].buf = addr;
	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = sizeof(data);
	msgs[1].buf = data;
	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret != 2)
		return ret < 0 ? ret : -EIO;
	*value = data[0];
	return 0;
}

static int os04c10_write_reg(struct os04c10 *priv, u16 address, u8 value)
{
	struct i2c_client *client = priv->client;
	u8 buf[3];
	struct i2c_msg msg;
	int ret;

	put_unaligned_be16(address, buf);
	buf[2] = value;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(buf);
	msg.buf = buf;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		return ret < 0 ? ret : -EIO;
	return 0;
}

static int os04c10_write_regs(struct os04c10 *priv, const struct os04c10_reg *regs, unsigned int count)
{
	unsigned int i;
	int ret;

	for (i = 0; i < count; i++) {
		if (regs[i].address == OS04C10_DELAY_MARKER) {
			usleep_range(regs[i].value * 1000,
				     regs[i].value * 1000 + 500);
			continue;
		}
		ret = os04c10_write_reg(priv, regs[i].address, regs[i].value);
		if (ret) {
			dev_err(&priv->client->dev,
				"os04c10: reg 0x%04x <- 0x%02x failed: %d\n",
				regs[i].address, regs[i].value, ret);
			return ret;
		}
	}
	return 0;
}

static int os04c10_identify(struct os04c10 *priv)
{
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(os04c10_id_checks); i++) {
		u8 value;
		ret = os04c10_read_reg(priv, os04c10_id_checks[i].address, &value);
		if (ret)
			return ret;
		if ((value & os04c10_id_masks[i]) != (os04c10_id_checks[i].value & os04c10_id_masks[i])) {
			dev_err(&priv->client->dev, "os04c10: chip id mismatch at 0x%x: read 0x%x, expected 0x%x\n",
				 os04c10_id_checks[i].address, value,
				 os04c10_id_checks[i].value);
			return -ENXIO;
		}
	}
	return 0;
}

static int os04c10_power_on(struct os04c10 *priv)
{
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(priv->supplies), priv->supplies);
	if (ret)
		return ret;
	usleep_range(1000, 2000);
	ret = clk_set_rate(priv->xclk, OS04C10_XCLK_FREQ);
	if (ret)
		dev_warn(&priv->client->dev, "xvclk set_rate %lu failed: %d\n",
			 OS04C10_XCLK_FREQ, ret);
	ret = clk_prepare_enable(priv->xclk);
	if (ret)
		goto disable_supplies;
	if (clk_get_rate(priv->xclk) &&
	    clk_get_rate(priv->xclk) != OS04C10_XCLK_FREQ)
		dev_warn(&priv->client->dev,
			 "xvclk is %lu Hz, OS04C10_MIPI_2L was captured at %lu Hz\n",
			 clk_get_rate(priv->xclk), OS04C10_XCLK_FREQ);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(priv->reset_gpio, 1);
	gpiod_set_value_cansleep(priv->pwdn_gpio, 1);
	usleep_range(10000, 12000);
	return 0;
disable_supplies:
	regulator_bulk_disable(ARRAY_SIZE(priv->supplies), priv->supplies);
	return ret;
}

static int os04c10_power_off(struct os04c10 *priv)
{
	gpiod_set_value_cansleep(priv->reset_gpio, 0);
	gpiod_set_value_cansleep(priv->pwdn_gpio, 0);
	clk_disable_unprepare(priv->xclk);
	regulator_bulk_disable(ARRAY_SIZE(priv->supplies), priv->supplies);
	return 0;
}

static int os04c10_init_controls(struct os04c10 *priv)
{
	struct v4l2_ctrl_handler *handler = &priv->ctrl_handler;
	v4l2_ctrl_handler_init(handler, 4);
	v4l2_ctrl_new_int_menu(handler, NULL, V4L2_CID_LINK_FREQ,
				  0, 0, os04c10_link_freq_menu);
	v4l2_ctrl_new_std(handler, NULL, V4L2_CID_PIXEL_RATE,
			 OS04C10_PIXEL_RATE, OS04C10_PIXEL_RATE, 1, OS04C10_PIXEL_RATE);
	v4l2_ctrl_new_std(handler, NULL, V4L2_CID_HBLANK,
			 1592, 1592, 1, 1592);
	v4l2_ctrl_new_std(handler, NULL, V4L2_CID_VBLANK,
			 54, 54, 1, 54);
	if (handler->error)
		return handler->error;
	priv->sd.ctrl_handler = handler;
	return 0;
}

static int os04c10_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_state *state,
			     struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index > 0)
		return -EINVAL;
	code->code = MEDIA_BUS_FMT_SBGGR10_1X10;
	return 0;
}

static int os04c10_enum_frame_size(struct v4l2_subdev *sd, struct v4l2_subdev_state *state,
			      struct v4l2_subdev_frame_size_enum *fse)
{
	if (fse->index > 0 || fse->code != MEDIA_BUS_FMT_SBGGR10_1X10)
		return -EINVAL;
	fse->min_width = OS04C10_WIDTH;
	fse->max_width = OS04C10_WIDTH;
	fse->min_height = OS04C10_HEIGHT;
	fse->max_height = OS04C10_HEIGHT;
	return 0;
}

static int os04c10_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *state,
		       struct v4l2_subdev_format *fmt)
{
	struct os04c10 *priv = to_os04c10(sd);

	fmt->format = priv->fmt;
	return 0;
}

static int os04c10_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *state,
		       struct v4l2_subdev_format *fmt)
{
	struct os04c10 *priv = to_os04c10(sd);

/* One mode only. TRY and ACTIVE both return it. */
	fmt->format = priv->fmt;
	return 0;
}

static int os04c10_get_selection(struct v4l2_subdev *sd, struct v4l2_subdev_state *state,
			     struct v4l2_subdev_selection *sel)
{
	if (sel->target != V4L2_SEL_TGT_CROP &&
	    sel->target != V4L2_SEL_TGT_CROP_BOUNDS &&
	    sel->target != V4L2_SEL_TGT_CROP_DEFAULT)
		return -EINVAL;
	sel->r.left = 0;
	sel->r.top = 0;
	sel->r.width = OS04C10_WIDTH;
	sel->r.height = OS04C10_HEIGHT;
	return 0;
}

static int os04c10_apply_ae(struct os04c10 *priv)
{
	u32 exposure = os04c10_exposure;
	u32 gain = os04c10_gain;
	int ret;

	ret = os04c10_write_reg(priv, 0x3500, (exposure >> 16) & 0xff);
	if (ret)
		return ret;
	ret = os04c10_write_reg(priv, 0x3501, (exposure >> 8) & 0xff);
	if (ret)
		return ret;
	ret = os04c10_write_reg(priv, 0x3502, exposure & 0xff);
	if (ret)
		return ret;
	ret = os04c10_write_reg(priv, 0x3508, (gain >> 8) & 0xff);
	if (ret)
		return ret;
	return os04c10_write_reg(priv, 0x3509, gain & 0xff);
}

static ssize_t exposure_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return sprintf(buf, "%u\n", os04c10_exposure);
}

static ssize_t exposure_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct os04c10 *priv = to_os04c10(sd);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 0, &value);
	if (ret)
		return ret;
	if (value < 8)
		value = 8;
	if (value > 0x0f00)
		value = 0x0f00;
	os04c10_exposure = value;
	os04c10_apply_ae(priv);
	return count;
}
static DEVICE_ATTR_RW(exposure);

static ssize_t analogue_gain_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", os04c10_gain);
}

static ssize_t analogue_gain_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct os04c10 *priv = to_os04c10(sd);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 0, &value);
	if (ret)
		return ret;
	if (value < 0x10)
		value = 0x10;
	if (value > 0x07ff)
		value = 0x07ff;
	os04c10_gain = value;
	os04c10_apply_ae(priv);
	return count;
}
static DEVICE_ATTR_RW(analogue_gain);

static int os04c10_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct os04c10 *priv = to_os04c10(sd);
	int ret;

	if (enable == priv->streaming)
		return 0;
	if (!enable) {
		os04c10_write_reg(priv, OS04C10_STREAM_REG, OS04C10_STREAM_OFF);
		pm_runtime_put(&priv->client->dev);
		priv->streaming = false;
		dev_info(&priv->client->dev, "os04c10: stream off\n");
		return 0;
	}
	ret = pm_runtime_resume_and_get(&priv->client->dev);
	if (ret < 0) {
		dev_err(&priv->client->dev, "os04c10: resume: %d\n", ret);
		return ret;
	}
	ret = os04c10_write_regs(priv, os04c10_mode_2688x1520_regs, ARRAY_SIZE(os04c10_mode_2688x1520_regs));
	if (ret) {
		dev_err(&priv->client->dev, "os04c10: mode table: %d\n", ret);
		goto err_pm;
	}
	ret = os04c10_apply_ae(priv);
	if (ret)
		dev_err(&priv->client->dev, "os04c10: exposure: %d\n", ret);
	ret = v4l2_ctrl_handler_setup(&priv->ctrl_handler);
	if (ret) {
		dev_err(&priv->client->dev, "os04c10: ctrl setup: %d\n", ret);
		goto err_pm;
	}
	ret = os04c10_write_reg(priv, OS04C10_STREAM_REG, OS04C10_STREAM_ON);
	if (ret) {
		dev_err(&priv->client->dev, "os04c10: stream on: %d\n", ret);
		goto err_pm;
	}
	priv->streaming = true;
	dev_info(&priv->client->dev, "os04c10: stream on\n");
	return 0;
err_pm:
	pm_runtime_put(&priv->client->dev);
	return ret;
}

static long os04c10_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct os04c10 *priv = to_os04c10(sd);
	struct rkmodule_inf *inf;
	struct rkmodule_hdr_cfg *hdr;
	u32 stream;

	switch (cmd) {
	case RKMODULE_GET_MODULE_INFO:
		inf = arg;
		memset(inf, 0, sizeof(*inf));
		strscpy(inf->base.sensor, "os04c10", sizeof(inf->base.sensor));
		strscpy(inf->base.module, "OS04C10",
			sizeof(inf->base.module));
		strscpy(inf->base.lens, "default", sizeof(inf->base.lens));
		return 0;
	case RKMODULE_GET_HDR_CFG:
		hdr = arg;
		memset(hdr, 0, sizeof(*hdr));
		hdr->hdr_mode = NO_HDR;
		hdr->esp.mode = HDR_NORMAL_VC;
		return 0;
	case RKMODULE_SET_HDR_CFG:
		return 0;
	case RKMODULE_SET_QUICK_STREAM:
		stream = *(u32 *)arg ? OS04C10_STREAM_ON : OS04C10_STREAM_OFF;
		return os04c10_write_reg(priv, OS04C10_STREAM_REG, stream);
	default:
		return -ENOIOCTLCMD;
	}
}

static const struct v4l2_subdev_core_ops os04c10_core_ops = {
	.ioctl = os04c10_ioctl,
};
static int os04c10_g_frame_interval(struct v4l2_subdev *sd,
				    struct v4l2_subdev_frame_interval *fi)
{
	fi->interval.numerator = 10000;
	fi->interval.denominator = 296900;
	return 0;
}

static int os04c10_get_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				   struct v4l2_mbus_config *config)
{
	if (pad)
		return -EINVAL;

	/*
	 * RV1106 5.10 keeps the lane count in flags. 1 << (lanes - 1) is
	 * V4L2_MBUS_CSI2_2_LANE. The Xunfei table has mipiclk=1 and 0x4800
	 * with the clock-lane gate set, so the CSI-2 clock is non-continuous.
	 */
	config->type = V4L2_MBUS_CSI2_DPHY;
	config->flags = (1U << (OS04C10_LANES - 1)) |
			V4L2_MBUS_CSI2_CHANNEL_0 |
			V4L2_MBUS_CSI2_NONCONTINUOUS_CLOCK;
	return 0;
}

static const struct v4l2_subdev_video_ops os04c10_video_ops = {
	.s_stream = os04c10_s_stream,
	.g_frame_interval = os04c10_g_frame_interval,
};
static const struct v4l2_subdev_pad_ops os04c10_pad_ops = {
	.enum_mbus_code = os04c10_enum_mbus_code,
	.enum_frame_size = os04c10_enum_frame_size,
	.get_fmt = os04c10_get_fmt,
	.set_fmt = os04c10_set_fmt,
	.get_selection = os04c10_get_selection,
	.get_mbus_config = os04c10_get_mbus_config,
};
static const struct v4l2_subdev_ops os04c10_subdev_ops = {
	.core = &os04c10_core_ops,
	.video = &os04c10_video_ops,
	.pad = &os04c10_pad_ops,
};

static int os04c10_runtime_resume(struct device *dev)
{
	struct v4l2_subdev *sd = dev_get_drvdata(dev);
	return os04c10_power_on(to_os04c10(sd));
}

static int os04c10_runtime_suspend(struct device *dev)
{
	struct v4l2_subdev *sd = dev_get_drvdata(dev);
	return os04c10_power_off(to_os04c10(sd));
}

static const struct dev_pm_ops os04c10_pm_ops = {
	SET_RUNTIME_PM_OPS(os04c10_runtime_resume, os04c10_runtime_suspend, NULL)
};

/*
 * The D-PHY notifier is registered at boot, before this module exists.
 * v4l2_async_register_subdev() only binds a sensor when that notifier
 * already has a v4l2_device parent; on this 5.10 tree the late sensor
 * stays on the unmatched list and the CIF has nothing to stream.
 * The D-PHY stores its media entity as platform drvdata, so attach the
 * sensor the same way the notifier's bound callback would have.
 */
static const char os04c10_isp_name[] = "rkisp0";

static struct v4l2_device *os04c10_find_isp(struct device *log, void *isp,
					    struct device *dev)
{
	int off;

	dev_info(log, "os04c10: isp scan %p dev %p w0 %p w2 %p\n",
		 isp, dev, *(void **)isp, *((void **)isp + 2));
	for (off = 0; off < 1200; off += 4) {
		struct v4l2_device *v4l2 = (void *)((char *)isp + off);
		struct media_device *mdev;
		const char *model;
		int i;

		if (v4l2->dev != dev)
			continue;
		mdev = v4l2->mdev;
		if ((unsigned long)mdev < 0x80000000UL)
			continue;
		model = mdev->model;
		for (i = 0; i < 24; i++) {
			if (model[i] == 'r' && model[i + 1] == 'k' &&
			    model[i + 2] == 'i') {
				dev_info(log, "os04c10: isp at %d model %s\n",
					 off, model + i);
				return v4l2;
			}
		}
		dev_info(log, "os04c10: dev hit %d mdev %p\n", off, mdev);
	}
	return NULL;
}

static void os04c10_register_sditf(struct device *dev)
{
	struct device_node *np;
	struct platform_device *pdev;
	struct media_entity *entity;
	struct v4l2_subdev *sditf, *isp_sd;
	struct v4l2_device *v4l2;
	void *isp;
	int ret;

	np = of_find_node_by_path("/rkcif-mipi-lvds-sditf");
	if (!np)
		return;
	pdev = of_find_device_by_node(np);
	of_node_put(np);
	if (!pdev)
		return;
	entity = platform_get_drvdata(pdev);
	put_device(&pdev->dev);
	if (!entity)
		return;
	sditf = media_entity_to_v4l2_subdev(entity);
	if (sditf->v4l2_dev)
		return;

	np = of_find_node_by_path("/rkisp-vir0");
	if (!np)
		return;
	pdev = of_find_device_by_node(np);
	of_node_put(np);
	if (!pdev)
		return;
	isp = platform_get_drvdata(pdev);
	if (!isp) {
		put_device(&pdev->dev);
		return;
	}
	v4l2 = os04c10_find_isp(dev, isp, &pdev->dev);
	put_device(&pdev->dev);
	if (!v4l2) {
		dev_err(dev, "os04c10: isp v4l2 not found\n");
		return;
	}
	ret = v4l2_device_register_subdev(v4l2, sditf);
	if (ret) {
		dev_err(dev, "os04c10: sditf register %d\n", ret);
		return;
	}
	isp_sd = NULL;
	list_for_each_entry(isp_sd, &v4l2->subdevs, list) {
		if (!strcmp(isp_sd->name, "rkisp-isp-subdev"))
			break;
	}
	if (!isp_sd || strcmp(isp_sd->name, "rkisp-isp-subdev")) {
		dev_err(dev, "os04c10: isp subdev missing\n");
		return;
	}
	ret = media_create_pad_link(&sditf->entity, 0, &isp_sd->entity, 0,
				    MEDIA_LNK_FL_ENABLED);
	/*
	 * link_setup sets isp_inp (rkisp_device + 0x99ec) when the remote
	 * name contains "rkcif", but it does not mark the mainpath stream
	 * linked. start_streaming rejects the device unless that byte,
	 * at isp + 0x7eb3, is set. Both offsets come from video_rkisp.ko.
	 */
	{
		u32 inp = *(u32 *)((char *)isp + 0x99ec);
		void *owner = *(void **)((char *)isp + 0x7850);
		u8 flag = *((u8 *)isp + 0x7eb3);

		dev_info(dev, "os04c10: link %d inp %x owner %p flag %u\n",
			 ret, inp, owner, flag);
		if (!inp)
			*(u32 *)((char *)isp + 0x99ec) = 0x100;
		if (owner == isp)
			*((u8 *)isp + 0x7eb3) = 1;
		/*
		 * sensors[0] starts at isp+0x31c, num_sensors at isp+0xc2c.
		 * update_sensor_info matches the linked sditf against this.
		 */
		*(struct v4l2_subdev **)((char *)isp + 0x31c) = sditf;
		*(int *)((char *)isp + 0xc2c) = 1;
		{
			void *hw = *(void **)((char *)isp + 0x94);
			u32 ready = *(u32 *)((char *)hw + 0xc0);

			dev_info(dev, "os04c10: hw %p link count %u\n", hw, ready);
			*(u32 *)((char *)hw + 0xc0) = 4;
			*((u8 *)isp + 0xe065) = 1;
		}
		/*
		 * sditf mode.name must be non-NULL or the CIF refuses the
		 * online path. link_mode TOISP0 lets the channel enable run.
		 * Offsets are from the subdev, which sits 52 bytes into sditf.
		 */
		{
			char **namep = (char **)((char *)sditf + 416);

			dev_info(dev, "os04c10: sditf name %p mode %d link %d\n",
				 *namep, *(int *)(namep + 1),
				 *(int *)((char *)namep + 36));
			*namep = (char *)os04c10_isp_name;
			*(int *)((char *)namep + 36) = 1;
		}
	}
}

static int os04c10_attach_dphy(struct os04c10 *priv)
{
	struct v4l2_subdev *sd = &priv->sd;
	struct device_node *phy_np;
	struct platform_device *phy_pdev;
	struct media_entity *phy_ent;
	struct v4l2_subdev *phy_sd;
	struct csi2_dphy *dphy;
	struct csi2_sensor *sensor;
	int ret, i;

	if (sd->v4l2_dev)
		return 0;

	phy_np = of_find_node_by_path("/csi2-dphy0");
	if (!phy_np)
		return -ENODEV;
	phy_pdev = of_find_device_by_node(phy_np);
	of_node_put(phy_np);
	if (!phy_pdev)
		return -ENODEV;
	phy_ent = platform_get_drvdata(phy_pdev);
	phy_sd = media_entity_to_v4l2_subdev(phy_ent);
	if (!phy_sd || !phy_sd->v4l2_dev) {
		put_device(&phy_pdev->dev);
		return -ENODEV;
	}

	ret = v4l2_device_register_subdev(phy_sd->v4l2_dev, sd);
	if (ret) {
		put_device(&phy_pdev->dev);
		return ret;
	}
	ret = media_create_pad_link(&sd->entity, 0, phy_ent,
				    CSI2_DPHY_RX_PAD_SINK, MEDIA_LNK_FL_ENABLED);
	if (ret) {
		v4l2_device_unregister_subdev(sd);
		put_device(&phy_pdev->dev);
		return ret;
	}

	dphy = container_of(phy_sd, struct csi2_dphy, sd);
	for (i = 0; i < dphy->num_sensors; i++) {
		if (dphy->sensors[i].sd == sd) {
			put_device(&phy_pdev->dev);
			return 0;
		}
	}
	if (dphy->num_sensors >= MAX_DPHY_SENSORS) {
		put_device(&phy_pdev->dev);
		return -EBUSY;
	}
	sensor = &dphy->sensors[dphy->num_sensors++];
	sensor->sd = sd;
	sensor->lanes = OS04C10_LANES;
	sensor->mbus.type = V4L2_MBUS_CSI2_DPHY;
	sensor->mbus.flags = (1U << (OS04C10_LANES - 1)) |
			     V4L2_MBUS_CSI2_CHANNEL_0 |
			     V4L2_MBUS_CSI2_NONCONTINUOUS_CLOCK;
	sensor->format = priv->fmt;
	put_device(&phy_pdev->dev);
	return 0;
}

static int os04c10_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct os04c10 *priv;
	unsigned long rate;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->client = client;
	v4l2_i2c_subdev_init(&priv->sd, client, &os04c10_subdev_ops);

	priv->xclk = devm_clk_get(dev, "xvclk");
	if (IS_ERR(priv->xclk))
		return PTR_ERR(priv->xclk);
	rate = clk_get_rate(priv->xclk);
	if (rate && rate != OS04C10_XCLK_FREQ)
		dev_warn(dev, "os04c10: xclk is %lu Hz, table was captured at %lu Hz\n",
			 rate, OS04C10_XCLK_FREQ);
	for (ret = 0; ret < ARRAY_SIZE(priv->supplies); ret++)
		priv->supplies[ret].supply = os04c10_supply_names[ret];
	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(priv->supplies), priv->supplies);
	if (ret) {
		dev_err(dev, "os04c10: regulators: %d\n", ret);
		return ret;
	}
	priv->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpio)) {
		ret = PTR_ERR(priv->reset_gpio);
		dev_err(dev, "os04c10: reset gpio: %d\n", ret);
		return ret;
	}
	priv->pwdn_gpio = devm_gpiod_get_optional(dev, "pwdn", GPIOD_OUT_LOW);
	if (IS_ERR(priv->pwdn_gpio)) {
		ret = PTR_ERR(priv->pwdn_gpio);
		dev_err(dev, "os04c10: pwdn gpio: %d\n", ret);
		return ret;
	}

	ret = os04c10_power_on(priv);
	if (ret) {
		dev_err(dev, "os04c10: power on: %d\n", ret);
		return ret;
	}
	ret = os04c10_identify(priv);
	if (ret) {
		dev_err(dev, "os04c10: identify: %d\n", ret);
		goto err_power;
	}
	ret = os04c10_init_controls(priv);
	if (ret) {
		dev_err(dev, "os04c10: controls: %d\n", ret);
		goto err_power;
	}

	priv->fmt.width = OS04C10_WIDTH;
	priv->fmt.height = OS04C10_HEIGHT;
	priv->fmt.code = MEDIA_BUS_FMT_SBGGR10_1X10;
	priv->fmt.field = V4L2_FIELD_NONE;
	priv->fmt.colorspace = V4L2_COLORSPACE_RAW;
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&priv->sd.entity, 1, &priv->pad);
	if (ret) {
		dev_err(dev, "os04c10: pads: %d\n", ret);
		goto err_ctrl;
	}
	priv->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	{
		u32 index = 0;
		const char *facing = "back";
		char face = 'b';

		of_property_read_u32(dev->of_node, "rockchip,camera-module-index", &index);
		of_property_read_string(dev->of_node, "rockchip,camera-module-facing", &facing);
		if (facing[0] != 'b')
			face = 'f';
		snprintf(priv->sd.name, sizeof(priv->sd.name), "m%02u_%c_os04c10 %s",
			 index, face, dev_name(dev));
	}
	ret = v4l2_async_register_subdev_sensor_common(&priv->sd);
	if (ret) {
		dev_err(dev, "os04c10: async register: %d\n", ret);
		goto err_entity;
	}
	ret = os04c10_attach_dphy(priv);
	if (ret) {
		dev_err(dev, "os04c10: attach dphy: %d\n", ret);
		goto err_async;
	}
	if (priv->sd.v4l2_dev) {
		ret = __v4l2_device_register_subdev_nodes(priv->sd.v4l2_dev, false);
		if (ret)
			dev_err(dev, "os04c10: subdev node: %d\n", ret);
	}
	os04c10_register_sditf(dev);
	dev_info(dev, "os04c10: sensor ready, v4l2 %s\n",
		 priv->sd.v4l2_dev ? "bound" : "unbound");
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_get_noresume(dev);
	device_create_file(dev, &dev_attr_exposure);
	device_create_file(dev, &dev_attr_analogue_gain);
	return 0;
err_async:
	v4l2_async_unregister_subdev(&priv->sd);
err_entity:
	media_entity_cleanup(&priv->sd.entity);
err_ctrl:
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
err_power:
	os04c10_power_off(priv);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
static void os04c10_remove(struct i2c_client *client)
#else
static int os04c10_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct os04c10 *priv = to_os04c10(sd);

	device_remove_file(&client->dev, &dev_attr_exposure);
	device_remove_file(&client->dev, &dev_attr_analogue_gain);
	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
	pm_runtime_disable(&client->dev);
	if (!pm_runtime_status_suspended(&client->dev))
		os04c10_power_off(priv);
	pm_runtime_set_suspended(&client->dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
	return 0;
#endif
}

static const struct of_device_id os04c10_of_match[] = {
	{ .compatible = "ovti,os04c10" },
	{ }
};
MODULE_DEVICE_TABLE(of, os04c10_of_match);

static const struct i2c_device_id os04c10_id[] = {
	{ "os04c10", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, os04c10_id);

static struct i2c_driver os04c10_i2c_driver = {
	.driver = {
		.name = "os04c10",
		.of_match_table = os04c10_of_match,
		.pm = &os04c10_pm_ops,
	},
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	.probe = os04c10_probe,
#else
	.probe_new = os04c10_probe,
#endif
	.remove = os04c10_remove,
	.id_table = os04c10_id,
};
module_i2c_driver(os04c10_i2c_driver);

MODULE_DESCRIPTION("OmniVision OS04C10 CSI-2 sensor driver for Rockchip RV1106");
MODULE_LICENSE("GPL v2");

