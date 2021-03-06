#ifndef __ARCH_AARCH64_COMMON_INCLUDE_IO_H__
#define __ARCH_AARCH64_COMMON_INCLUDE_IO_H__


#include "common/include/inttypes.h"
#include "common/include/mem.h"
#include "common/include/atomic.h"


static inline void mmio_mb()
{
    atomic_mb();
}


static inline u8 mmio_read8(unsigned long addr)
{
    volatile u8 *ptr = (u8 *)addr;
    u8 val = 0;

    mmio_mb();
    val = *ptr;
    mmio_mb();

    return val;
}

static inline void mmio_write8(unsigned long addr, u8 val)
{
    volatile u8 *ptr = (u8 *)addr;

    mmio_mb();
    *ptr = val;
    mmio_mb();
}

static inline u16 mmio_read16(unsigned long addr)
{
    volatile u16 *ptr = (u16 *)addr;
    u16 val = 0;

    mmio_mb();
    val = *ptr;
    mmio_mb();

    return val;
}

static inline void mmio_write16(unsigned long addr, u16 val)
{
    volatile u16 *ptr = (u16 *)addr;

    mmio_mb();
    *ptr = val;
    mmio_mb();
}

static inline u32 mmio_read32(unsigned long addr)
{
    volatile u32 *ptr = (u32 *)addr;
    u32 val = 0;

    mmio_mb();
    val = *ptr;
    mmio_mb();

    return val;
}

static inline void mmio_write32(unsigned long addr, u32 val)
{
    volatile u32 *ptr = (u32 *)addr;

    mmio_mb();
    *ptr = val;
    mmio_mb();
}

static inline u64 mmio_read64(unsigned long addr)
{
    volatile u64 *ptr = (u64 *)addr;
    u64 val = 0;

    mmio_mb();
    val = *ptr;
    mmio_mb();

    return val;
}

static inline void mmio_write64(unsigned long addr, u64 val)
{
    volatile u64 *ptr = (u64 *)addr;

    mmio_mb();
    *ptr = val;
    mmio_mb();
}


static inline u8 port_read8(unsigned long addr)
    { return mmio_read8(addr); }
static inline void port_write8(unsigned long addr, u8 val)
    { mmio_write8(addr, val); }

static inline u16 port_read16(unsigned long addr)
    { return mmio_read16(addr); }
static inline void port_write16(unsigned long addr, u16 val)
    { mmio_write16(addr, val); }

static inline u32 port_read32(unsigned long addr)
    { return mmio_read32(addr); }
static inline void port_write32(unsigned long addr, u32 val)
    { mmio_write32(addr, val); }

static inline u64 port_read64(unsigned long addr)
    { return mmio_read64(addr); }
static inline void port_write64(unsigned long addr, u64 val)
    { mmio_write64(addr, val); }


#endif
