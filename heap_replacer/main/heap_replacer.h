#pragma once

#include "util.h"

#include "initial_allocator/initial_allocator.h"

#include "memory_pools/memory_pool_manager.h"
#include "default_heap/default_heap_manager.h"
#include "scrap_heap/scrap_heap_manager.h"

namespace nvhr
{
	initial_allocator ina(INITIAL_ALLOCATOR_SIZE);
	memory_pool_manager* mpm;
	default_heap_manager* dhm;

	void* __fastcall nvhr_malloc(size_t size)
	{
		if (size < 4) [[unlikely]] { size = 4; }
		if (size <= 2 * KB) [[likely]]
		{
			if (void* address = mpm->malloc(size)) [[likely]] { return address; }
		}
		if (void* address = dhm->malloc(size)) [[likely]] { return address; }
		HR_MSGBOX("Failed malloc.");
		return nullptr;
	}

	void* __fastcall nvhr_calloc(size_t count, size_t size)
	{
		size *= count;
		if (size < 4) [[unlikely]] { size = 4; }
		if (size <= 2 * KB) [[likely]]
		{
			if (void* address = mpm->calloc(size)) [[likely]] { return address; }
		}
		if (void* address = dhm->calloc(size)) [[likely]] { return address; }
		HR_MSGBOX("Failed calloc.");
		return nullptr;
	}

	void* __fastcall nvhr_realloc(void* address, size_t size)
	{
		if (!address) [[unlikely]] { return nvhr_malloc(size); }
		size_t old_size, new_size;
		if ((old_size = nvhr_mem_size(address)) >= (new_size = size)) { return address; }
		void* new_address = nvhr_malloc(size);
		memcpy(new_address, address, new_size < old_size ? new_size : old_size);
		nvhr_free(address);
		return new_address;
	}

	void* __fastcall nvhr_recalloc(void* address, size_t count, size_t size)
	{
		if (!address) [[unlikely]] { return nvhr_calloc(count, size); }
		size_t old_size, new_size;;
		if ((old_size = nvhr_mem_size(address)) >= (new_size = size * count))
		{
			util::memset8(address, 0u, new_size);
			return address;
		}
		void* new_address = nvhr_calloc(count, size);
		nvhr_free(address);
		return new_address;
	}

	size_t __fastcall nvhr_mem_size(void* address)
	{
		if (!address) [[unlikely]] { return 0u; }
		if (size_t size = mpm->mem_size(address)) [[likely]] { return size; }
		return dhm->mem_size(address);
	}

	void __fastcall nvhr_free(void* address)
	{
		if (!address) [[unlikely]] { return; }
		if (mpm->free(address)) [[likely]] { return; }
		dhm->free(address);
	}

	void* __fastcall game_heap_allocate(TFPARAM(void* self, size_t size))
	{
		return nvhr_malloc(size);
	}

	void* __fastcall game_heap_reallocate(TFPARAM(void* self, void* address, size_t size))
	{
		return nvhr_realloc(address, size);
	}

	size_t __fastcall game_heap_msize(TFPARAM(void* self, void* address))
	{
		return nvhr_mem_size(address);
	}

	void __fastcall game_heap_free(TFPARAM(void* self, void* address))
	{
		nvhr_free(address);
	}

	void* __cdecl crt_malloc(size_t size)
	{
		return nvhr_malloc(size);
	}

	void* __cdecl crt_calloc(size_t count, size_t size)
	{
		return nvhr_calloc(count, size);
	}

	void* __cdecl crt_realloc(void* address, size_t size)
	{
		return nvhr_realloc(address, size);
	}

	void* __cdecl crt_recalloc(void* address, size_t count, size_t size)
	{
		return nvhr_recalloc(address, count, size);
	}

	size_t __cdecl crt_msize(void* address)
	{
		return nvhr_mem_size(address);
	}

	void __cdecl crt_free(void* address)
	{
		nvhr_free(address);
	}

	void* __fastcall nvhr_ina_malloc(size_t size)
	{
		return ina.malloc(size);
	}

	void* __fastcall nvhr_ina_calloc(size_t count, size_t size)
	{
		return ina.calloc(count, size);
	}

	size_t __fastcall nvhr_ina_mem_size(void* address)
	{
		return ina.mem_size(address);
	}

	void __fastcall nvhr_ina_free(void* address)
	{
		ina.free(address);
	}

	void apply_heap_hooks()
	{

		mpm = new memory_pool_manager();
		dhm = new default_heap_manager();
		Sleep(5000);
#if defined(FNV)

		util::patch_jmp(0xECD1C7, &crt_malloc);
		util::patch_jmp(0xED0CDF, &crt_malloc);
		util::patch_jmp(0xEDDD7D, &crt_calloc);
		util::patch_jmp(0xED0D24, &crt_calloc);
		util::patch_jmp(0xECCF5D, &crt_realloc);
		util::patch_jmp(0xED0D70, &crt_realloc);
		util::patch_jmp(0xEE1700, &crt_recalloc);
		util::patch_jmp(0xED0DBE, &crt_recalloc);
		util::patch_jmp(0xECD31F, &crt_msize);
		util::patch_jmp(0xECD291, &crt_free);

		util::patch_jmp(0xAA3E40, &game_heap_allocate);
		util::patch_jmp(0xAA4150, &game_heap_reallocate);
		util::patch_jmp(0xAA4200, &game_heap_reallocate);
		util::patch_jmp(0xAA44C0, &game_heap_msize);
		util::patch_jmp(0xAA4060, &game_heap_free);

		util::patch_ret(0xAA6840);
		util::patch_ret(0x866E00);
		util::patch_ret(0x866770);

		util::patch_ret(0xAA6F90);
		util::patch_ret(0xAA7030);
		util::patch_ret(0xAA7290);
		util::patch_ret(0xAA7300);

		util::patch_jmp(0x40FBF0, util::force_cast<void*>(&light_critical_section::lock));
		util::patch_jmp(0x40FBA0, util::force_cast<void*>(&light_critical_section::unlock));

		util::patch_ret(0xAA58D0);
		util::patch_ret(0x866D10);
		util::patch_ret(0xAA5C80);

		util::patch_jmp(0xAA53F0, util::force_cast<void*>(&scrap_heap_block::init_0x10000));
		util::patch_jmp(0xAA5410, util::force_cast<void*>(&scrap_heap_block::init_var));
		util::patch_jmp(0xAA54A0, util::force_cast<void*>(&scrap_heap_block::alloc));
		util::patch_jmp(0xAA5610, util::force_cast<void*>(&scrap_heap_block::free));
		util::patch_jmp(0xAA5460, util::force_cast<void*>(&scrap_heap_block::purge));

		util::patch_nops(0xAA38CA, 0xAA38E8 - 0xAA38CA);
		util::patch_jmp(0xAA42E0, util::force_cast<void*>(&scrap_heap_block::get_thread_scrap_heap));

		util::patch_nop_call(0xAA3060);

		util::patch_nop_call(0x86C56F);
		util::patch_nop_call(0xC42EB1);
		util::patch_nop_call(0xEC1701);

		util::patch_bytes(0x86EED4, (BYTE*)"\xEB\x55", 2);

#elif defined(FO3)

		util::patch_jmp(0xC063F5, &crt_malloc);
		util::patch_jmp(0xC0AB3F, &crt_malloc);
		util::patch_jmp(0xC1843C, &crt_calloc);
		util::patch_jmp(0xC0AB7F, &crt_calloc);
		util::patch_jmp(0xC06546, &crt_realloc);
		util::patch_jmp(0xC0ABC7, &crt_realloc);
		util::patch_jmp(0xC06761, &crt_recalloc);
		util::patch_jmp(0xC0AC12, &crt_recalloc);
		util::patch_jmp(0xC067DA, &crt_msize);
		util::patch_jmp(0xC064B8, &crt_free);

		util::patch_jmp(0x86B930, &game_heap_allocate);
		util::patch_jmp(0x86BAE0, &game_heap_reallocate);
		util::patch_jmp(0x86BB50, &game_heap_reallocate);
		util::patch_jmp(0x86B8C0, &game_heap_msize);
		util::patch_jmp(0x86BA60, &game_heap_free);

		util::patch_ret(0x86D670);
		util::patch_ret(0x6E21F0);
		util::patch_ret(0x6E1E10);

		util::patch_jmp(0x409A80, &enter_light_critical_section); 
		//util::patch_jmp(0x0, &leave_light_critical_section);

		util::patch_ret(0x86C600);
		util::patch_ret(0x6E1CD0);
		util::patch_ret(0x86CA30);

		util::patch_jmp(0x86CB70, util::force_cast<void*>(&scrap_heap_block::init_0x10000));
		util::patch_jmp(0x86CB90, util::force_cast<void*>(&scrap_heap_block::init_var));
		util::patch_jmp(0x86C710, util::force_cast<void*>(&scrap_heap_block::alloc));
		util::patch_jmp(0x86C7B0, util::force_cast<void*>(&scrap_heap_block::free));
		util::patch_jmp(0x86CAA0, util::force_cast<void*>(&scrap_heap_block::purge));

		util::patch_nops(0x86C038, 0x86C086 - 0x86C038);
		util::patch_jmp(0x86BCB0, util::force_cast<void*>(&scrap_heap_block::get_thread_scrap_heap));

		util::mem::patch_nop_call(0x6E9B30);
		util::mem::patch_nop_call(0x7FACDB);
		util::mem::patch_nop_call(0xAA6534);

#endif

		HR_PRINTF("Hooks applied.");

	}

}
