#!/bin/bash

BUILD_PATH=build
RUN_PATH=build/qemu

OVMF_DISK_IMG=/usr/share/ovmf/OVMF.fd
BINARY_PATH=${BUILD_PATH}/bin/hype.efi
DISK_PATH=${RUN_PATH}/fat.img

# clean previous
rm -rf ${BUILD_PATH}

# build
mkdir ${BUILD_PATH}
cd ${BUILD_PATH}
cmake ..
make
cd ..

if [ ! -f ${BINARY_PATH} ]; then
    echo "Compilation failed!"
    exit 1
fi


# make image
mkdir -p ${RUN_PATH}

## prepare files for efi partition (application binary + startup script)
dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1

mmd -i /tmp/part.img ::/EFI
mmd -i /tmp/part.img ::/EFI/BOOT
mcopy -i /tmp/part.img ${BINARY_PATH} ::/EFI/BOOT/BOOTX64.efi

echo "bcfg boot add 0 FS0:\EFI\BOOT\BOOTX64.efi \"my_boot\"" > ${RUN_PATH}/startup.nsh
#echo app.efi >> ${RUN_PATH}/startup.nsh
mcopy -i /tmp/part.img ${RUN_PATH}/startup.nsh ::/startup.nsh

## make full image (with format and efi partition)
dd if=/dev/zero of=${DISK_PATH} bs=512 count=93750
parted ${DISK_PATH} -s -a minimal mklabel gpt
parted ${DISK_PATH} -s -a minimal mkpart EFI FAT16 2048s 93716s
parted ${DISK_PATH} -s -a minimal toggle 1 boot
## copy files into the image
dd if=/tmp/part.img of=${DISK_PATH} bs=512 count=91669 seek=2048 conv=notrunc

# run
qemu-system-x86_64 \
  -cpu host \
  -m 1G \
  -enable-kvm \
  -net none \
  -drive if=pflash,format=raw,unit=0,file=${OVMF_DISK_IMG},readonly=on \
  -drive if=ide,format=raw,file=${DISK_PATH}