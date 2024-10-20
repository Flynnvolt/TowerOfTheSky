#pragma once
#include "Entities.c"

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	int render_target_w;
	int render_target_h;
	Entity* selected_entity;
	Matrix4 world_proj;
	Matrix4 world_view;
	Matrix4 screen_proj;
	Matrix4 screen_view;
	bool hover_consumed;
	Entity* player;
};

WorldFrame world_frame;