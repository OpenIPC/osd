#!/bin/bash
DL="https://github.com/openipc/firmware/releases/download/latest"

if [[ "$1" = *-sst6 ]]; then
	CC=cortex_a7_thumb2_hf-gcc13-glibc-4_9
else
	CC=cortex_a7_thumb2-gcc13-musl-4_9
fi

GCC=$PWD/toolchain/$CC/bin/arm-linux-gcc

if [ ! -e toolchain/$CC ]; then
	wget -c -nv --show-progress $DL/$CC.tgz -P $PWD
	mkdir -p toolchain/$CC
	tar -xf $CC.tgz -C toolchain/$CC --strip-components=1 || exit 1
	rm -f $CC.tgz
fi

if [ ! -e firmware ]; then
	git clone https://github.com/openipc/firmware --depth=1
fi

if [ "$1" = "aio-goke" ]; then
	DRV=$PWD/firmware/general/package/goke-osdrv-gk7205v200/files/lib
	make -C aio -B CC=$GCC DRV=$DRV $1
elif [ "$1" = "aio-hisi" ]; then
	DRV=$PWD/firmware/general/package/hisilicon-osdrv-hi3516ev200/files/lib
	make -C aio -B CC=$GCC DRV=$DRV $1
elif [ "$1" = "aio-sst6" ]; then
	DRV=$PWD/firmware/general/package/sigmastar-osdrv-infinity6e/files/lib
	make -C aio -B CC=$GCC DRV=$DRV $1
else
	echo "Usage: $0 [aio-goke|aio-hisi|aio-sst6]"
	exit 1
fi
