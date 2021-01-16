# Compiling for EFI

## Manual

- [UEFI](https://wiki.osdev.org/UEFI)

```shell
$ g++ main.cpp                           \
      -c                                 \
      -fno-stack-protector               \
      -fpic                              \
      -fshort-wchar                      \
      -mno-red-zone                      \
      -I /path/to/gnu-efi/headers        \
      -I /path/to/gnu-efi/headers/x86_64 \
      -DEFI_FUNCTION_WRAPPER             \
      -o main.o
 
$ ld main.o                         \
     /path/to/crt0-efi-x86_64.o     \
     -nostdlib                      \
     -znocombreloc                  \
     -T /path/to/elf_x86_64_efi.lds \
     -shared                        \
     -Bsymbolic                     \
     -L /path/to/libs               \
     -l:libgnuefi.a                 \
     -l:libefi.a                    \
     -o main.so
 
$ objcopy -j .text                \
          -j .sdata               \
          -j .data                \
          -j .dynamic             \
          -j .dynsym              \
          -j .rel                 \
          -j .rela                \
          -j .reloc               \
          --target=efi-app-x86_64 \
          main.so                 \
          main.efi
```

## EFI Make

- [EFI example](http://www.rodsbooks.com/efi-programming/hello.html)

A _makefile_ to build an _EFI_ application:
```make
ARCH            = x86_64

OBJS            = main.o
TARGET          = hello.efi

CC              = g++
EFIINC          = /usr/local/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIB             = /usr/local/lib
EFILIB          = /usr/local/lib/
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi.lds

CFLAGS          = $(EFIINCS) -fno-stack-protector -fpic \
		  		  -fshort-wchar -mno-red-zone -Wall 
ifeq ($(ARCH),x86_64)
	  CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS) 

all: $(TARGET)

hello.so: $(OBJS)
		ld $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

%.efi: %.so
		objcopy -j .text -j .sdata -j .data -j .dynamic \
					-j .dynsym  -j .rel -j .rela -j .reloc \
							--target=efi-app-$(ARCH) $^ $@

```
