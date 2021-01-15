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
- [CMAKE + UEFI](https://github.com/eruffaldi/uefiboot)
- [CMAKE + UEFI 2](https://github.com/matlo607/uefi-test)

### Setting Up Environment

#### EDK II

EDK II is a modern, feature-rich, cross-platform firmware development environment for the UEFI and PI specifications.

[Prerequisites](https://github.com/tianocore/tianocore.github.io/wiki/Using-EDK-II-with-Native-GCC#Ubuntu_1604_LTS__Ubuntu_1610)
[Build edk2](https://github.com/tianocore/tianocore.github.io/wiki/Common-instructions)

```shell
git clone https://github.com/tianocore/edk2.git
cd edk2
git checkout edk2-stable202011
git submodule update --init
make -C BaseTools
source edksetup.sh
```

Now we should be in a _EDK II_ env in the shell, so we can build with it.

#### OVMF

[Build OVMF](https://github.com/tianocore/tianocore.github.io/wiki/How-to-build-OVMF)

Modify configurations to wanted target under `Conf/target.txt`:
```
ACTIVE_PLATFORM       = OvmfPkg/OvmfPkgX64.dsc
TARGET_ARCH           = X64
TOOL_CHAIN_TAG        = GCC5
```

Run `build` from the _EDK-II_ env to build. 
The binary should be under `Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd`.

[Run OVMF](https://github.com/tianocore/tianocore.github.io/wiki/How-to-run-OVMF)

Install __QEMU__:
```shell
sudo apt install qemu-system
```

Run _QEMU_ with the build binary:
```shell
qemu-system-x86_64 -L . --bios /path/to/edk2/Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd -net none
```

#### Building UEFI Application

UEFI applications use Microsoft's Portable Executable format.

Install cross compiler:
```shell
sudo apt-get install mingw-w64
```

We will need an _sdk_ for using _UEFI_. We'll use _GNU-EFI_.

_GNU-EFI_ is a very lightweight developing environment to create UEFI applications.

[Building GNU-EFI](https://wiki.osdev.org/GNU-EFI)

```shell
git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
cd gnu-efi
make
sudo make install
```

We can then add it to the project using _cmake_.
