#pragma once

constexpr size_t pool_alignment = 0x01000000u;
constexpr size_t pool_growth = 0x00010000u;

constexpr size_t pool_count = 10u;
constexpr size_t pool_size_array_length = 32u;
constexpr size_t pool_addr_array_length = (0x80000000u / pool_alignment) << 1;
