# hype

__Hype__ is a type 1 hypervisor for the _Intel x86-64_ architecture
utilizing the _VT-x_ virtualization technology, written in _C++_. 
__Hype__ is capable of booting directly from _UEFI_.

At the moment, the project is planned for running a _linux distro_,
but this may change.

This is mostly a research and experimentation project rather then
a product for use.

## Resources

- [Intel's IA-32 Software Development Manual Volume 3](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-system-programming-manual-325384.html)
- [UEFI Specification Manual](https://www.uefi.org/sites/default/files/resources/UEFI%20Spec%202_6.pdf)
- [OSDev](https://wiki.osdev.org/)
  
## Other Hypervisors

- [Bareflank](https://github.com/Bareflank/hypervisor/)
- [hvpp](https://github.com/wbenny/hvpp)
- [zpp](https://github.com/eyalz800/zpp_hypervisor)


## Setting Up Environment

### Building UEFI Application

We will need an _sdk_ for using _UEFI_. We'll use __GNU-EFI__.

__GNU-EFI__ is a very lightweight developing environment to create _UEFI_ applications.

- [Building GNU-EFI](https://wiki.osdev.org/GNU-EFI)

```shell
git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
cd gnu-efi
make
sudo make install
```

We can also install directly from _apt_:
```shell
sudo apt install gnu-efi
```

We can then add it to the project using _cmake_. This requires
adding a [script](scripts/modules/FindGNUefi.cmake) to find the library binaries and headers, as well as 
setting a bunch of flags for correct compilation. [See CMakeLists.txt](hype/CMakeLists.txt).

Made with references from: 
- [eruffaldi/uefiboot](https://github.com/eruffaldi/uefiboot)
- [matlo607/uefi-test](https://github.com/matlo607/uefi-test)

With correctly configured _cmake_, we will get `.efi` binary.

### Running

- [Run OVMF](https://github.com/tianocore/tianocore.github.io/wiki/How-to-run-OVMF)
- [Boot EFI with QEMU](https://unix.stackexchange.com/questions/52996/how-to-boot-efi-kernel-using-qemu-kvm)
- [Building OVMF](BUILD_OVMF.md)

Install __QEMU__ and __OVMF__:
```shell
sudo apt install qemu-system ovmf
```

Run __QEMU__:
```shell
qemu-system-x86_64 -enable-kvm --bios /usr/share/ovmf/OVMF.fd -net none
```
