/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - CPU Registers
 * ---------------------------------------------------------------------------
 */
#pragma once
#include <stddef.h>
#include <utils.h>

typedef struct x86_CR3 {
    UINT reserved0 : 3;  // 0 - 2
    UINT pwt       : 1;  // 3
    UINT pcd       : 1;  // 4
    UINT reserved1 : 7;  // 5 - 11
    UINT physical  : 20; // 12 - 31
} __attribute__((packed)) x86_CR3;

#define x86_LOAD_REG(reg, source) __asm__ volatile("mov " #reg ", %0;" ::"r"(source))
#define x86_READ_REG(reg, dest) __asm__ volatile("mov %0, " #reg :"=r"(dest))
