CC ?= arm-none-eabi-gcc
CFLAGS := -marm -O3 -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=softfp
CFLAGS += -nostdlib -nostartfiles -fno-builtin
LDFLAGS := -lgcc -Wl,-e,_start

nv12color: nv12color.c
	$(CC) $(CFLAGS) -o $@ nv12color.c $(LDFLAGS)

clean:
	rm -f nv12color
