#!/bin/sh
IQ=/oem/usr/share/iqfiles
JSON=$IQ/os04a10_CMK-OT1607-PV1_50IRC-F16.json
BIN=$IQ/os04a10_CMK-OT1607-PV1_50IRC-F16.bin
if [ ! -f "$JSON" ] || [ ! -f "$BIN" ]; then
	echo "OS04A10 IQ files are not in $IQ" >&2
	exit 1
fi
ln -sfn "$JSON" "$IQ/os04c10_OS04C10_default.json"
ln -sfn "$BIN" "$IQ/os04c10_OS04C10_default.bin"
