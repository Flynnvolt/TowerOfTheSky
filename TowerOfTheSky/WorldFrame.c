#pragma once
#include "Entities.c"

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	int render_target_w;
	int render_target_h;
	Entity* selected_entity;
	Vector2 camera_pos_copy; // this is kinda jank, but it's here so we can pass down the camera position to the rendering, instead of extracting it from the view matrix
	Matrix4 world_proj;
	Matrix4 world_view;
	Matrix4 screen_proj;
	Matrix4 screen_view;
	bool hover_consumed;
	Entity* player;
};

WorldFrame world_frame;