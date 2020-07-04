#include "libfdt_env.h"

#include "fdt.h"
#include "libfdt.h"

#include "libfdt_internal.h"


uint16_t fdt16_to_cpu(fdt16_t x)
{
	return (FDT_FORCE uint16_t)CPU_TO_FDT16(x);
}
fdt16_t cpu_to_fdt16(uint16_t x)
{
	return (FDT_FORCE fdt16_t)CPU_TO_FDT16(x);
}

uint32_t fdt32_to_cpu(fdt32_t x)
{
	return (FDT_FORCE uint32_t)CPU_TO_FDT32(x);
}
inline fdt32_t cpu_to_fdt32(uint32_t x)
{
	return (FDT_FORCE fdt32_t)CPU_TO_FDT32(x);
}

inline uint64_t fdt64_to_cpu(fdt64_t x)
{
	return (FDT_FORCE uint64_t)CPU_TO_FDT64(x);
}

fdt64_t cpu_to_fdt64(uint64_t x)
{
	return (FDT_FORCE fdt64_t)CPU_TO_FDT64(x);
}

const void *fdt_offset_ptr_(const void *fdt, int offset)
{
	return (const char *)fdt + fdt_off_dt_struct(fdt) + offset;
}

void *fdt_offset_ptr_w_(void *fdt, int offset)
{
	return (void *)(uintptr_t)fdt_offset_ptr_(fdt, offset);
}

const struct fdt_reserve_entry *fdt_mem_rsv_(const void *fdt, int n)
{
	const struct fdt_reserve_entry *rsv_table =
		(const struct fdt_reserve_entry *)
		((const char *)fdt + fdt_off_mem_rsvmap(fdt));

	return rsv_table + n;
}
struct fdt_reserve_entry *fdt_mem_rsv_w_(void *fdt, int n)
{
	return (void *)(uintptr_t)fdt_mem_rsv_(fdt, n);
}


