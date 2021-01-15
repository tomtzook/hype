# Building OVMF

### EDK II

__EDK II__ is a modern, feature-rich, cross-platform firmware development environment 
for the _UEFI_ and _PI_ specifications.

- [Prerequisites](https://github.com/tianocore/tianocore.github.io/wiki/Using-EDK-II-with-Native-GCC#Ubuntu_1604_LTS__Ubuntu_1610)
- [Build edk2](https://github.com/tianocore/tianocore.github.io/wiki/Common-instructions)

```shell
git clone https://github.com/tianocore/edk2.git
cd edk2
git checkout edk2-stable202011
git submodule update --init
make -C BaseTools
source edksetup.sh
```

Now we should be in a __EDK II__ env in the shell, so we can build with it.

### OVMF

- [Build OVMF](https://github.com/tianocore/tianocore.github.io/wiki/How-to-build-OVMF)

Modify configurations to wanted target under `Conf/target.txt`:
```
ACTIVE_PLATFORM       = OvmfPkg/OvmfPkgX64.dsc
TARGET_ARCH           = X64
TOOL_CHAIN_TAG        = GCC5
```

Run `build` from the __EDK-II__ env to build.
The binary should be under `Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd`.
