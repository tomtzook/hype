#pragma once

#include <crt.h>
#include <debug.h>
#include <error.h>


namespace hype::result {

enum code_t {
    SUCCESS = 0,
    ALLOCATION_ERROR,
    NOT_SUPPORTED
};

}
