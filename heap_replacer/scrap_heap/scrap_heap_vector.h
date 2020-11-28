#pragma once

#include "main/util.h"

namespace ScrapHeap
{

	struct scrap_heap;

	struct mt_sh
	{
		DWORD id;
		scrap_heap* sh;
	};

	class scrap_heap_vector
	{

	private:

		size_t size;
		size_t alloc;
		mt_sh* data;

	private:

		DWORD lock_id;

	public:

		scrap_heap_vector(size_t size) : size(0), alloc(size)
		{
			this->data = (mt_sh*)NVHR::nvhr_calloc(this->alloc, sizeof(mt_sh));
			this->lock_id = NULL;
		}

		~scrap_heap_vector()
		{
			NVHR::nvhr_free(this->data);
		}

		void insert(DWORD id, scrap_heap* sh)
		{
			ECS(&this->lock_id);
			if (this->size >= this->alloc)
			{
				this->alloc <<= 1;
				mt_sh* temp = (mt_sh*)NVHR::nvhr_realloc(this->data, this->alloc * sizeof(mt_sh));
				memmove(temp, this->data, this->size * sizeof(mt_sh));
				NVHR::nvhr_free(this->data);
				this->data = temp;
			}
			this->data[size++] = mt_sh({ id, sh });
			LCS(&this->lock_id);
		}

		scrap_heap* find(DWORD id)
		{
			ECS(&this->lock_id);
			for (size_t i = 0; i < this->size; i++)
			{
				if (this->data[i].id == id)
				{
					LCS(&this->lock_id);
					return this->data[i].sh;
				}
			}
			LCS(&this->lock_id);
			return nullptr;
		}

	};

}
