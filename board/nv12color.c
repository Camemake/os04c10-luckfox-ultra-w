#include <arm_neon.h>

#define W 2688
#define H 1520
#define YSIZE (W * H)
#define UVSIZE (YSIZE / 2)
#define FRAME (YSIZE + UVSIZE)
#define NBUF 4

#define MPLANE 9
#define MMAP 1

#define _IOC(dir, type, nr, size) \
	(((dir) << 30) | ((type) << 8) | (nr) | ((size) << 16))
#define VIDIOC_REQBUFS  _IOC(3, 'V', 8, 20)
#define VIDIOC_QUERYBUF _IOC(3, 'V', 9, 68)
#define VIDIOC_QBUF     _IOC(3, 'V', 15, 68)
#define VIDIOC_DQBUF    _IOC(3, 'V', 17, 68)
#define VIDIOC_STREAMON _IOC(1, 'V', 18, 4)

struct reqbufs {
	unsigned int count;
	unsigned int type;
	unsigned int memory;
	unsigned int capabilities;
	unsigned int reserved;
};

struct plane {
	unsigned int bytesused;
	unsigned int length;
	unsigned int mem_offset;
	unsigned int data_offset;
	unsigned int reserved[11];
};

struct buffer {
	unsigned int index;
	unsigned int type;
	unsigned int bytesused;
	unsigned int flags;
	unsigned int field;
	long tv_sec;
	long tv_usec;
	unsigned int tc_type;
	unsigned int tc_flags;
	unsigned char tc_rest[8];
	unsigned int sequence;
	unsigned int memory;
	struct plane *planes;
	unsigned int length;
	unsigned int reserved2;
	unsigned int request;
};

static unsigned char *maps[NBUF];
static struct plane planes[NBUF];

static int sys_open(const char *path, int flags)
{
	register int r0 asm("r0") = (int)path;
	register int r1 asm("r1") = flags;
	register int r7 asm("r7") = 5;

	asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r7) : "memory");
	return r0;
}

static int sys_ioctl(int fd, unsigned cmd, void *arg)
{
	register int r0 asm("r0") = fd;
	register unsigned r1 asm("r1") = cmd;
	register void *r2 asm("r2") = arg;
	register int r7 asm("r7") = 54;

	asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r7) : "memory");
	return r0;
}

static void *sys_mmap(unsigned int size, int fd, unsigned int off)
{
	register int r0 asm("r0") = 0;
	register unsigned r1 asm("r1") = size;
	register int r2 asm("r2") = 3;
	register int r3 asm("r3") = 1;
	register int r4 asm("r4") = fd;
	register unsigned r5 asm("r5") = off >> 12;
	register int r7 asm("r7") = 192;

	asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r7) : "memory");
	return (void *)r0;
}

static int sys_write(int fd, const void *buf, int n)
{
	register int r0 asm("r0") = fd;
	register const void *r1 asm("r1") = buf;
	register int r2 asm("r2") = n;
	register int r7 asm("r7") = 4;

	asm volatile("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r7) : "memory");
	return r0;
}

static void sys_exit(int code)
{
	register int r0 asm("r0") = code;
	register int r7 asm("r7") = 1;

	asm volatile("svc #0" : : "r"(r0), "r"(r7));
	for (;;)
		;
}

static void zero(void *p, int n)
{
	unsigned char *b = p;
	int i;

	for (i = 0; i < n; i++)
		b[i] = 0;
}

static int write_full(const unsigned char *src, int n)
{
	int put = 0;

	while (put < n) {
		int r = sys_write(1, src + put, n - put);

		if (r <= 0)
			return -1;
		put += r;
	}
	return 0;
}

static void shift(unsigned char *uv)
{
	uint8x16_t sub = {8, 18, 8, 18, 8, 18, 8, 18,
			  8, 18, 8, 18, 8, 18, 8, 18};
	int i;

	for (i = 0; i < UVSIZE; i += 16)
		vst1q_u8(uv + i, vqsubq_u8(vld1q_u8(uv + i), sub));
}

void _start(void)
{
	struct reqbufs req;
	int fd, i, on;
	static const char dev[] = "/dev/video11";

	fd = sys_open(dev, 2);
	if (fd < 0)
		sys_exit(2);
	zero(&req, sizeof(req));
	req.count = NBUF;
	req.type = MPLANE;
	req.memory = MMAP;
	if (sys_ioctl(fd, VIDIOC_REQBUFS, &req) < 0 || req.count < 2)
		sys_exit(3);
	if (req.count > NBUF)
		req.count = NBUF;
	for (i = 0; i < (int)req.count; i++) {
		struct buffer buf;

		zero(&buf, sizeof(buf));
		zero(&planes[i], sizeof(planes[i]));
		buf.index = i;
		buf.type = MPLANE;
		buf.memory = MMAP;
		buf.planes = &planes[i];
		buf.length = 1;
		if (sys_ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0)
			sys_exit(4);
		maps[i] = sys_mmap(planes[i].length, fd, planes[i].mem_offset);
		if ((unsigned)maps[i] >= 0xfffff000u)
			sys_exit(5);
		if (sys_ioctl(fd, VIDIOC_QBUF, &buf) < 0)
			sys_exit(6);
	}
	on = MPLANE;
	if (sys_ioctl(fd, VIDIOC_STREAMON, &on) < 0)
		sys_exit(7);
	for (;;) {
		struct buffer buf;
		unsigned char *p;
		unsigned int n;

		zero(&buf, sizeof(buf));
		buf.type = MPLANE;
		buf.memory = MMAP;
		buf.length = 1;
		buf.planes = &planes[0];
		if (sys_ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
			sys_exit(8);
		p = maps[buf.index];
		n = planes[0].bytesused;
		if (n < FRAME)
			n = FRAME;
		shift(p + YSIZE);
		if (write_full(p, FRAME))
			sys_exit(0);
		buf.planes = &planes[0];
		buf.length = 1;
		if (sys_ioctl(fd, VIDIOC_QBUF, &buf) < 0)
			sys_exit(9);
	}
}
