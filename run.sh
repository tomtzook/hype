#!/bin/bash

BUILD_PATH=build
RUN_PATH=build/qemu

OVMF_DISK_IMG=.edk2build/OVMF.fd
BINARY_PATH=${BUILD_PATH}/bin/hype.efi

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
mkdir -p ${RUN_PATH}/image/EFI/BOOT
cp ${BINARY_PATH} ${RUN_PATH}/image/EFI/BOOT/BOOTX64.efi

# run
qemu-system-x86_64 \
  -cpu host \
  -m 1G \
  -enable-kvm \
  -bios ${OVMF_DISK_IMG} \
  -drive if=ide,format=raw,file=fat:rw:${RUN_PATH}/image,index=0,media=disk
