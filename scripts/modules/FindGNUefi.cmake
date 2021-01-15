include(FindPackageHandleStandardArgs)

find_path(GNUEFI_INCLUDE_DIR_efi "efi.h"
        PATHS "/usr/include"
        PATH_SUFFIXES "efi")

find_path(GNUEFI_INCLUDE_DIR_efiarch "efibind.h"
        PATHS "/usr/include"
        PATH_SUFFIXES "efi/${CMAKE_HOST_SYSTEM_PROCESSOR}")

find_path(GNUEFI_INCLUDE_DIR_efiprotocol "efivar.h"
        PATHS "/usr/include"
        PATH_SUFFIXES "efi/protocol")

find_library(GNUEFI_LIBRARY_efi "efi"
        PATHS "/usr/lib"
        PATH_SUFFIXES "lib" "lib64")

find_package_handle_standard_args(GNUefi DEFAULT_MSG
        GNUEFI_INCLUDE_DIR_efi
        GNUEFI_INCLUDE_DIR_efiarch
        GNUEFI_INCLUDE_DIR_efiprotocol
        GNUEFI_LIBRARY_efi)

if(GNUefi_FOUND)
    set(GNUEFI_INCLUDE_DIRS ${GNUEFI_INCLUDE_DIR_efi} ${GNUEFI_INCLUDE_DIR_efiarch} ${GNUEFI_INCLUDE_DIR_efiprotocol})
    set(GNUEFI_LIBRARIES ${GNUEFI_LIBRARY_efi})

    message(STATUS "Found GNUefi\n"
            " * include: ${GNUEFI_INCLUDE_DIRS}\n"
            " * libraries: ${GNUEFI_LIBRARIES}\n")
    mark_as_advanced(CONAN_GNU-EFI_ROOT
            GNUEFI_INCLUDE_DIR_efi
            GNUEFI_INCLUDE_DIR_efiarch
            GNUEFI_INCLUDE_DIR_efiprotocol
            GNUEFI_LIBRARY_efi)

    if(NOT TARGET GnuEfi::GnuEfi)
        add_library(GnuEfi::GnuEfi UNKNOWN IMPORTED)
        set_target_properties(GnuEfi::GnuEfi PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GNUEFI_INCLUDE_DIRS}")
        set_target_properties(GnuEfi::GnuEfi PROPERTIES
                IMPORTED_LOCATION ${GNUEFI_LIBRARIES})

        set(GNUEFI_COMPILE_DEFINITION "-D_GNU_EFI")
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(GNUEFI_COMPILE_DEFINITION "${GNUEFI_COMPILE_DEFINITION} -DEFI_DEBUG")
        endif()

        set_target_properties(GnuEfi::GnuEfi PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "CONFIG_${CMAKE_HOST_SYSTEM_PROCESSOR} -DGNU_EFI_USE_MS_ABI ${GNUEFI_COMPILE_DEFINITION}")
    endif()
endif()
