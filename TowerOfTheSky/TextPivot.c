typedef enum Pivot Pivot;

enum Pivot 
{
	PIVOT_bottom_left,
	PIVOT_bottom_center,
	PIVOT_bottom_right,
	PIVOT_center_left,
	PIVOT_center_center,
	PIVOT_center_right,
	PIVOT_top_left,
	PIVOT_top_center,
	PIVOT_top_right,
};

Gfx_Text_Metrics draw_text_with_pivot(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Pivot pivot, Draw_Frame *frame) 
{
	Gfx_Text_Metrics metrics = measure_text(font, text, raster_height, scale);
	position = v2_sub(position, metrics.visual_pos_min);
	Vector2 pivot_mul = {0};
	switch (pivot) 
	{
		case PIVOT_bottom_left: pivot_mul = v2(0.0, 0.0); break;
		case PIVOT_bottom_center: pivot_mul = v2(0.5, 0.0); break;
		case PIVOT_bottom_right: pivot_mul = v2(1.0, 0.0); break;
		case PIVOT_center_left: pivot_mul = v2(0.0, 0.5); break;
		case PIVOT_center_center: pivot_mul = v2(0.5, 0.5); break;
		case PIVOT_center_right: pivot_mul = v2(1.0, 0.5); break;
		case PIVOT_top_center: pivot_mul = v2(0.5, 1.0); break;
		case PIVOT_top_left: pivot_mul = v2(0.0, 1.0); break;
		case PIVOT_top_right: pivot_mul = v2(1.0, 1.0); break;
		default:
		log_error("pivot not supported yet. fill in case at draw_text_with_pivot");
		break;
	}
	position = v2_sub(position, v2_mul(metrics.visual_size, pivot_mul));
	draw_text_in_frame(font, text, raster_height, position, scale, color, frame);
	return metrics;
}