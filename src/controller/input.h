typedef struct float_vec2 {
    float x;
    float y;
} float_vec2;

float_vec2 input_get_movement_axis();
float_vec2 input_get_movement_axis_smooth(float speed, float delta_time);