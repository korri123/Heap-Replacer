#pragma once

#include "main/util.h"

class mem_cell;

class cell_node
{

public:

	cell_node* next;
	cell_node* prev;
	mem_cell* cell;
	size_t array_index;

	cell_node(mem_cell* cell);
	~cell_node();

	cell_node* link(cell_node* prev, cell_node* next);

	bool is_valid();

};