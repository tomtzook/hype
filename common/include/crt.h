#pragma once

#include "types.h"
#include "environment.h"
#include "debug.h"


void* operator new(size_t size) noexcept;
void* operator new(size_t size, environment::alignment_t alignment) noexcept;

void* operator new[](size_t size) noexcept;
void* operator new[](size_t size, environment::alignment_t alignment) noexcept;

void operator delete(void* memory) noexcept;
void operator delete[](void* memory) noexcept;
