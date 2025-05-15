#!/bin/bash

if [[ -z "$1" ]]; then
  echo path to binary missing.
  exit 1
fi

BINARY_PATH="$1"

if [ ! -f ${BINARY_PATH} ]; then
    echo "path given is of missing file"
    exit 1
fi

OVMF_DISK_IMG=.edk2build/OVMF.fd
RUN_PATH=build_qemu

# make image
mkdir -p ${RUN_PATH}
mkdir -p ${RUN_PATH}/image/EFI/BOOT
cp ${BINARY_PATH} ${RUN_PATH}/image/EFI/BOOT/BOOTX64.efi

# run
# -gdb tcp::9000 -S
qemu-system-x86_64 \
  -cpu host \
  -m 1G \
  -M q35,accel=tcg \
  -gdb tcp::9000 \
  -d int,cpu_reset,pcall,guest_errors -no-reboot -no-shutdown \
  -enable-kvm \
  -bios ${OVMF_DISK_IMG} \
  -drive if=ide,format=raw,file=fat:rw:${RUN_PATH}/image,index=0,media=disk
