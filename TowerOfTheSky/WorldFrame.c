#pragma once
#include "Entities.c"

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	Matrix4 world_proj;
	Matrix4 world_view;
	bool hover_consumed;
	Entity* player;
};

WorldFrame world_frame;