
int entry(int argc, char **argv) {
	
	window.title = STR("Tower of the Sky");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x2a2d3aff);

	float64 last_time = os_get_current_time_in_seconds();

	// load player art
	Gfx_Image * player = load_image_from_disk(fixed_string("player.png"), get_heap_allocator());
	assert(player, "player image failed to load");

	//player starting pos
	Vector2 player_pos = v2(0,0);

	//Game Loop
	while (!window.should_close) 
	{
		reset_temporary_storage();

		float64 now = os_get_current_time_in_seconds();
		float64 delta_t = now - last_time;
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		last_time = now;

		//Press Esc to Close Game
		if(is_key_just_pressed(KEY_ESCAPE))
		{
			window.should_close = true;
		}

		//Wasd Movement
		Vector2 input_axis = v2(0,0);

		if (is_key_down('A')) {
			input_axis.x -= 1.0;
		}
		if (is_key_down('D')) {
			input_axis.x += 1.0;
		}
		if (is_key_down('S')) {
			input_axis.y -= 1.0;
		}
		if (is_key_down('W')) {
			input_axis.y += 1.0;
		}
		
		input_axis = v2_normalize(input_axis);

		player_pos = v2_add(player_pos, v2_mulf(input_axis, 1.0 * delta_t));
		
		//draw player
		Matrix4 xform = m4_scalar(1.0);
		xform         = m4_translate(xform, v3(player_pos.x, player_pos.y, 0));
		draw_image_xform(player, xform, v2(.5f, .5f), COLOR_WHITE);


		os_update(); 
		gfx_update();
	}

	return 0;
}