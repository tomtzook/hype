
#include "common.h"

#include "commonefi.h"


common::result efi::result::efi_result(EFI_STATUS efi_status) noexcept {
    return common::result(efi_status, efi::result::CATEGORY);
}

template<>
common::result::result(EFI_STATUS code) noexcept
        : m_code(code)
        , m_category(efi::result::CATEGORY)
{}

#ifdef _DEBUG
const wchar_t* efi::result::debug::to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case EFI_SUCCESS: return L"EFI_SUCCESS";
        case EFI_LOAD_ERROR: return L"EFI_LOAD_ERROR";
        case EFI_INVALID_PARAMETER: return L"EFI_INVALID_PARAMETER";
        case EFI_UNSUPPORTED: return L"EFI_UNSUPPORTED";
        case EFI_BAD_BUFFER_SIZE: return L"EFI_BAD_BUFFER_SIZE";
        case EFI_BUFFER_TOO_SMALL: return L"EFI_BUFFER_TOO_SMALL";
        case EFI_NOT_READY: return L"EFI_NOT_READY";
        case EFI_DEVICE_ERROR: return L"EFI_DEVICE_ERROR";
        case EFI_WRITE_PROTECTED: return L"EFI_WRITE_PROTECTED";
        case EFI_OUT_OF_RESOURCES: return L"EFI_OUT_OF_RESOURCES";
        case EFI_VOLUME_CORRUPTED: return L"EFI_VOLUME_CORRUPTED";
        case EFI_VOLUME_FULL: return L"EFI_VOLUME_FULL";
        case EFI_NO_MEDIA: return L"EFI_NO_MEDIA";
        case EFI_MEDIA_CHANGED: return L"EFI_MEDIA_CHANGED";
        case EFI_NOT_FOUND: return L"EFI_NOT_FOUND";
        case EFI_ACCESS_DENIED: return L"EFI_ACCESS_DENIED";
        case EFI_NO_RESPONSE: return L"EFI_NO_RESPONSE";
        case EFI_NO_MAPPING: return L"EFI_NO_MAPPING";
        case EFI_TIMEOUT: return L"EFI_TIMEOUT";
        case EFI_NOT_STARTED: return L"EFI_NOT_STARTED";
        case EFI_ALREADY_STARTED: return L"EFI_ALREADY_STARTED";
        case EFI_ABORTED: return L"EFI_ABORTED";
        case EFI_ICMP_ERROR: return L"EFI_ICMP_ERROR";
        case EFI_TFTP_ERROR: return L"EFI_TFTP_ERROR";
        case EFI_PROTOCOL_ERROR: return L"EFI_PROTOCOL_ERROR";
        case EFI_INCOMPATIBLE_VERSION: return L"EFI_INCOMPATIBLE_VERSION";
        case EFI_SECURITY_VIOLATION: return L"EFI_SECURITY_VIOLATION";
        case EFI_CRC_ERROR: return L"EFI_CRC_ERROR";
        case EFI_END_OF_MEDIA: return L"EFI_END_OF_MEDIA";
        case EFI_END_OF_FILE: return L"EFI_END_OF_FILE";
        case EFI_INVALID_LANGUAGE: return L"EFI_INVALID_LANGUAGE";
        case EFI_COMPROMISED_DATA: return L"EFI_COMPROMISED_DATA";
        case EFI_WARN_UNKNOWN_GLYPH: return L"EFI_WARN_UNKNOWN_GLYPH";
        case EFI_WARN_DELETE_FAILURE: return L"EFI_WARN_DELETE_FAILURE";
        case EFI_WARN_WRITE_FAILURE: return L"EFI_WARN_WRITE_FAILURE";
        case EFI_WARN_BUFFER_TOO_SMALL: return L"EFI_WARN_BUFFER_TOO_SMALL";
        default: return L"";
    }
}
#endif
