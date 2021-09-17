#include "../engine/engine.h"
#include <Math.h>

//https://lodev.org/cgtutor/raycasting2.html

//https://developer.valvesoftware.com/wiki/Constant-Linear-Quadratic_Falloff

//just remember the intensity should also be multiplied by the dot product
//between the surface normal and the direction to the light source -Snurf

#define pi (3.141593)

#define S_W (1920)
#define S_H (1080)

struct vec2 {
	float x;
	float y;
};

struct raySample {
	struct vec2 normal;
	float texture_x;
	float distance;
	float light_intensity;
};



float dotVec2(struct vec2 v1, struct vec2 v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

s_w = S_W;
s_h = S_H;

float rayAngles[S_W];
struct raySample samples[S_W];

BMP_IMAGE wall_texture_1, wall_texture_2, wall_texture_3;
BMP_IMAGE wall_textures[3];
BMP_IMAGE lightbulb_texture_1;

typedef struct ACTOR_STRUCT {
	float x;
	float y;
	float a; //clamp 2*pi	
	float fov;
} ACTOR;

typedef struct light {
	float x;
	float y;
	float intensity;
	float radius;
	COLOR32 color;
} LIGHT;

typedef struct object {
	float x;
	float y;
	BMP_IMAGE img;	
} OBJECT;

const COLOR32 transparency = {255, 0, 255};

ACTOR player = {32, 8, 0, pi/3};
LIGHT light1 = {32, 8, 1.6, 36};
OBJECT light1_sprite;

typedef struct LEVEL_STRUCT {
	const int w;
	const int h;
	const char gridTileSize;
	const char* map;
} LEVEL;

const LEVEL level = {
	64, 
	16, 
	32,
/*
"################################################################\
#..............................................................#\
#..............................................................#\
##################################################.............#\
#....................#...........................#.............#\
#.#.#.#.#.#.#.#.#.#.##..####################.###.#.............#\
#....................#..#................#.....#.#.............#\
##.#.#.#.#.#.#.#.#.#....#.......p..............#.#.............#\
#....................#..#................#.....#.#.............#\
#.#.#.#.#.#.#.#.#.#.##..####################.###.#.............#\
#....................#...........................#.............#\
##################################################.............#\
#..............................................................#\
#..............................................................#\
#..............................................................#\
################################################################"
};
*/
///*
"################################################################\
#..............................................................#\
#..............................................................#\
#..#...........................................................#\
#...#..........................................................#\
#....#.........................................................#\
#.....#.....................#......#...........................#\
#......#........................p..............................#\
#......#....................#......#...........................#\
#.....#........................................................#\
#....#.........................................................#\
#...#..........................................................#\
#..#...........................................................#\
#..............................................................#\
#..............................................................#\
################################################################"
};
//*/
char getTile(int x, int y) {
	if(x >= 0 && x < level.w && y >= 0 && y < level.h) {
		return level.map[y * level.w + x];
	} else {
		return '?';
	}
}

struct CRGA_CACHE {
	float x;
	float y;
	float b;
	float off_x;
	float off_y;
} crga_cache;

float bounceRayToLight(const float origin_x, const float origin_y, const float target_x, const float target_y, struct vec2 normal) {
	
	//char FIRST_ITERATION_GUARD = 0; //Fix a situation where ray collides with origin tile
	float intensity;
	float da = sqrt(((origin_x - target_x) * (origin_x - target_x)) + ((origin_y - target_y) * (origin_y - target_y)));
		
	float dv;
	float dh;
	float min;
	
	float theta = atan2(target_y - origin_y, target_x - origin_x) * -1;	
	
	if (theta < 0) {
		theta = 2 * pi + theta;
	}
	
	struct vec2 dirToLight = {-cos(theta), sin(theta)};
	
	float p_x = origin_x;
	float p_y = origin_y;
	
	float off_x = origin_x - (int)origin_x;
	float off_y = origin_y - (int)origin_y;
	
	int step_x; //do we go left or right?
	int step_y; //up or down?
	
	int vertical_intersection_x;
	float vertical_intersection;
	float horizontal_intersection;
	int horizontal_intersection_y;
	
	int target_x_test = (int)target_x;
	int target_y_test = (int)target_y;
	
	int test_x;
	int test_y;
	
	unsigned char tile;
	
	if (theta <= pi/2)
	{
		//quadrant 1
		off_x = 1 - off_x;
		
		step_x = 1;
		step_y = -1;
	} 
	else if (theta <= pi)
	{
		//quadrant 2				
		step_x = -1;
		step_y = -1;		
	}
	else if (theta <= (3*pi)/2)
	{
		//quadrant 3
		off_y = 1 - off_y;
		
		step_x = -1;
		step_y = 1;		
	}
	else
	{
		//quadrant 4
		off_x = 1 - off_x;
		off_y = 1 - off_y;
		
		step_x = 1;
		step_y = 1;		
	}
	
	//how far are we to first vertical intersection?
	vertical_intersection_x = p_x + (step_x * off_x);
	vertical_intersection = p_y - (step_x * (tan(theta) * off_x));
	
	while(1) {

		
		if (step_x == 1) {
			tile = getTile((int) vertical_intersection_x, (int) vertical_intersection);
			
			test_x = (int)vertical_intersection_x;
			test_y = (int)vertical_intersection;
			
			if (test_x == target_x_test && test_y == target_y_test) {				
				dv = da;
				break;
			}		
		} else {
			tile = getTile((int) vertical_intersection_x-1, (int) vertical_intersection);
			
			test_x = (int)vertical_intersection_x-1;
			test_y = (int)vertical_intersection;
			
			if (test_x == target_x_test && test_y == target_y_test) {				
				dv = da;
				break;
			}
		}

		if (tile == '#') { //either hit wall...				
			dv = sqrt(((vertical_intersection_x - p_x) * (vertical_intersection_x - p_x)) + ((vertical_intersection - p_y) * (vertical_intersection - p_y)));
			break;
		} else if (tile == '?') {  //or went out of bounds
			dv = sqrt(((vertical_intersection_x - p_x) * (vertical_intersection_x - p_x)) + ((vertical_intersection - p_y) * (vertical_intersection - p_y)));
			break;
		}
		vertical_intersection_x += step_x;
		vertical_intersection -= step_x * tan(theta);
	}
	//first horizontal?
	horizontal_intersection = p_x - (step_y / tan(theta) * off_y);
	horizontal_intersection_y = p_y + (step_y * off_y);
	
	while(1) {		
		if (step_y == 1) {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y);
			
			test_x = (int)horizontal_intersection;
			test_y = (int)horizontal_intersection_y;			
			
			if (test_x == target_x_test && test_y == target_y_test) {				
				dh = da;
				break;
			}
		} else {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y - 1);

			test_x = (int)horizontal_intersection;
			test_y = (int)horizontal_intersection_y - 1;
			
			if (test_x == target_x_test && test_y == target_y_test) {
				dh = da;
				break;
			}	
		}
		
		if (tile == '#') {
			dh = sqrt(((horizontal_intersection - p_x) * (horizontal_intersection - p_x)) + ((horizontal_intersection_y - p_y) * (horizontal_intersection_y - p_y)));
			break;
		} else if (tile == '?') {
			dh = sqrt(((horizontal_intersection - p_x) * (horizontal_intersection - p_x)) + ((horizontal_intersection_y - p_y) * (horizontal_intersection_y - p_y)));
			break;
		}
		horizontal_intersection -= step_y / tan(theta);
		horizontal_intersection_y += step_y;
	}	
	/*
	//these comparisons prevent occasional flickering due to bounces intersecting with
	//their originating wall
	
	OR WILL THEY???
	
	*/
	/*
	if (dv * 100 <= 100) { 	
		dv = 99999;
	} else if (dh * 100 <= 100) {
		dh = 99999;
	}*/
	
	min = (dv < dh) ? dv : dh;
/*
	if (dv < da || dh < da) {
		return 0.0;
	} else {
		return da;
	}
*/	

	//Fatt Ip Kd(N' * L')
	float dotp = dotVec2(normal, dirToLight);
	intensity = (da < light1.radius) ? 0.1f + fabs((1.0f - (da / light1.radius )) * light1.intensity * dotp) : 0.1f; //magic number is kind of the radius of the light but hardcoded
	//return intensity;
	//if (samples[x].light_distance > 48 || samples[x].light_distance == 0) shade = 0.1;
	
	return (dv < da || dh < da) ? 0.1 : intensity;
}

struct raySample castRayGridAccel(const float theta) {
	
	struct raySample sample;
	
	char NOT_HORIZONTAL = 0;
	char NOT_VERTICAL = 0;
	
	float p_x = crga_cache.x;
	float p_y = crga_cache.y;
	float beta = crga_cache.b; //for getting non euclidean distance
	
	float off_x = crga_cache.off_x; //get fractional component
	float off_y = crga_cache.off_y;
	
	int step_x; //do we go left or right?
	int step_y; //up or down?
	
	int vertical_intersection_x;
	float vertical_intersection;
	float horizontal_intersection;
	int horizontal_intersection_y;
	
	float dist_vert;
	float dist_horz;
	
	float delta_x1;
	float delta_y1;
	float delta_x2;
	float delta_y2;
		
	int test_x;
	int test_y;
	
	unsigned char tile;
	
	if (theta <= pi/2)
	{
		//quadrant 1
		off_x = 1 - off_x;
		
		step_x = 1;
		step_y = -1;		
	} 
	else if (theta <= pi)
	{
		//quadrant 2				
		step_x = -1;
		step_y = -1;		
	}
	else if (theta <= (3*pi)/2)
	{
		//quadrant 3
		off_y = 1 - off_y;
		
		step_x = -1;
		step_y = 1;		
	}
	else
	{
		//quadrant 4
		off_x = 1 - off_x;
		off_y = 1 - off_y;
		
		step_x = 1;
		step_y = 1;
	}
	
	//how far are we to first vertical intersection?
	vertical_intersection_x = p_x + (step_x * off_x);
	vertical_intersection = p_y - (step_x * (tan(theta) * off_x));
	
	while(1) {
		test_x = (int)vertical_intersection_x;
		test_y = (int)vertical_intersection;
		
		if (step_x == 1) {
			tile = getTile((int) vertical_intersection_x, (int) vertical_intersection);
		} else {
			tile = getTile((int) vertical_intersection_x-1, (int) vertical_intersection);
		}		

		if (tile == '#') { //either hit wall...
			break;
		} else if (tile == '?') {  //or went out of bounds
			NOT_VERTICAL = 1;
			break;
		}
		vertical_intersection_x += step_x;
		vertical_intersection -= step_x * tan(theta);
	}
	//first horizontal?
	horizontal_intersection = p_x - (step_y / tan(theta) * off_y);
	horizontal_intersection_y = p_y + (step_y * off_y);
	
	while(1) {
		test_x = (int)horizontal_intersection;
		test_y = (int)horizontal_intersection_y;
		
		if (step_y == 1) {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y);
		} else {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y - 1);
		}		
		
		if (tile == '#') {
			break;
		} else if (tile == '?') {
			NOT_HORIZONTAL = 1;
			break;
		}
		horizontal_intersection -= step_y / tan(theta);
		horizontal_intersection_y += step_y;
	}
	
	delta_x1 = vertical_intersection_x - p_x;
	delta_y1 = vertical_intersection - p_y;
	delta_x2 = horizontal_intersection - p_x;
	delta_y2 = horizontal_intersection_y - p_y;
	
	dist_vert = (delta_x1 * cos(beta)) - (delta_y1 * sin(beta));
	dist_horz = (delta_x2 * cos(beta)) - (delta_y2 * sin(beta));
	
	if (NOT_VERTICAL) {
		sample.texture_x = (step_y != -1) ? 1 - (horizontal_intersection - (int)horizontal_intersection) : (horizontal_intersection - (int)horizontal_intersection);
		sample.normal.x = 0;
		sample.normal.y = -step_y;
		sample.distance = dist_horz;
		if ((step_y == -1 && light1.y < horizontal_intersection_y) || (step_y == 1 && light1.y > horizontal_intersection_y)) {
			sample.light_intensity = 0.1;
		} else {		
			sample.light_intensity = bounceRayToLight(horizontal_intersection, horizontal_intersection_y, light1.x, light1.y, sample.normal);
		}
	} else if (NOT_HORIZONTAL) {
		sample.texture_x = (step_x != 1) ? 1 - (vertical_intersection - (int)vertical_intersection) : (vertical_intersection - (int)vertical_intersection);
		sample.normal.x = -step_x;
		sample.normal.y = 0;
		sample.distance = dist_vert;
		if ((step_x == 1 && light1.x > vertical_intersection_x) || (step_x == -1 && light1.x < vertical_intersection_x)) {
			sample.light_intensity = 0.1;
		} else {		
			sample.light_intensity = bounceRayToLight(vertical_intersection_x, vertical_intersection, light1.x, light1.y, sample.normal);
		}
	} else if (dist_vert < dist_horz) {
		sample.texture_x = (step_x != 1) ? 1 - (vertical_intersection - (int)vertical_intersection) : (vertical_intersection - (int)vertical_intersection);
		sample.normal.x = -step_x;
		sample.normal.y = 0;
		sample.distance = dist_vert;
		if ((step_x == 1 && light1.x > vertical_intersection_x) || (step_x == -1 && light1.x < vertical_intersection_x)) {
			sample.light_intensity = 0.1;
		} else {		
			sample.light_intensity = bounceRayToLight(vertical_intersection_x, vertical_intersection, light1.x, light1.y, sample.normal);
		}
	} else {
		sample.texture_x = (step_y != -1) ? 1 - (horizontal_intersection - (int)horizontal_intersection) : (horizontal_intersection - (int)horizontal_intersection);
		sample.normal.x = 0;
		sample.normal.y = -step_y;
		sample.distance = dist_horz;
		if ((step_y == -1 && light1.y < horizontal_intersection_y) || (step_y == 1 && light1.y > horizontal_intersection_y)) {
			sample.light_intensity = 0.1;
		} else {
			sample.light_intensity = bounceRayToLight(horizontal_intersection, horizontal_intersection_y, light1.x, light1.y, sample.normal);
		}
	}
	return sample;
}


void update (void) {
	
	if (Keys['W'].down) {
		player.x += cos(player.a) * fElapsedTime * 4;
		player.y -= sin(player.a) * fElapsedTime * 4;
	}
	
	if (getTile((int)player.x, (int)player.y) == '#') {
		player.x -= cos(player.a) * fElapsedTime * 4;
		player.y += sin(player.a) * fElapsedTime * 4;
	}
	
	if (Keys['S'].down) {
		player.x -= cos(player.a) * fElapsedTime * 4;
		player.y += sin(player.a) * fElapsedTime * 4;
	}
	
	if (getTile((int)player.x, (int)player.y) == '#') {
		player.x += cos(player.a) * fElapsedTime * 4;
		player.y -= sin(player.a) * fElapsedTime * 4;
	}
	
	if (Keys['A'].down) { //A
		player.a += 2 * fElapsedTime;
	}
	
	if (Keys['D'].down) { //D
		player.a -= 2 * fElapsedTime;
	}
	
	if (Keys[VK_UP].down) {		
		light1.y -= fElapsedTime * 2;
		light1_sprite.y = light1.y;
	}
	
	if (getTile((int)light1.x, (int)light1.y) == '#') {
		light1.y += fElapsedTime * 2;
		light1_sprite.y = light1.y;
	}
	
	if (Keys[VK_DOWN].down) {		
		light1.y += fElapsedTime * 2;
		light1_sprite.y = light1.y;
	}
	
	if (getTile((int)light1.x, (int)light1.y) == '#') {
		light1.y -= fElapsedTime * 2;
		light1_sprite.y = light1.y;
	}
	
	if (Keys[VK_LEFT].down) {		
		light1.x -= fElapsedTime * 2;
		light1_sprite.x = light1.x;
	}
	
	if (getTile((int)light1.x, (int)light1.y) == '#') {
		light1.x += fElapsedTime * 2;
		light1_sprite.x = light1.x;
	}
	
	if (Keys[VK_RIGHT].down) {		
		light1.x += fElapsedTime * 2;
		light1_sprite.x = light1.x;
	}
	
	if (getTile((int)light1.x, (int)light1.y) == '#') {
		light1.x -= fElapsedTime * 2;
		light1_sprite.x = light1.x;
	}
	
	if (Keys['L'].down) {
		light1.x = player.x;
		light1.y = player.y;
	}
	
	
	if (player.a >= 2*pi) player.a -= 2*pi; //range of 0 - 2pi
	else if (player.a <= 0) player.a += 2*pi;

	const float rayAngleStep = player.fov / (float)s_w;
	float rayAngle = player.a + (player.fov / 2.0f);
	
	crga_cache.x = player.x;
	crga_cache.y = player.y;
	crga_cache.b = player.a;
	crga_cache.off_x = player.x - (int)player.x;
	crga_cache.off_y = player.y - (int)player.y;
	
	for (int i = 0 ; i < s_w; i++) {
		if (rayAngle > 2*pi) rayAngles[i] = rayAngle - 2 * pi;
		else if (rayAngle < 0) rayAngles[i] = rayAngle + 2 * pi;		
		else rayAngles[i] = rayAngle;
		rayAngle -= rayAngleStep;
	}

	for (int i = 0; i < s_w; i++) {
		samples[i] = castRayGridAccel(rayAngles[i]);
	}

/*	drawFillRect(0, 0, s_w, s_h/2, 0xce, 0xf4, 0xfd); //#cef4fd
	drawFillRect(0, s_h/2, s_w, s_h, 0xc7, 0x97, 0x6b); //#c7976b
	

	int shadeMono; //just shading darker based on distance from player and some arbitrary constant
	for (int i = 0; i < s_w; i++) {
		shadeMono = (samples[i].light_distance == 0) ? 0 : (int)((1.0 - (samples[i].light_distance)/48) * 255);
		
		if (samples[i].light_distance > 48) {
			shadeMono = 0;
		}
		
		//shadeMono = (samples[i].light_distance > 0) ? 255 : 0;		
		//shadeMono += 100; //ambient
		
		if (shadeMono > 255) {
			shadeMono = 255;
		} else if (shadeMono < 0 ) {
			shadeMono = 0;
		}
		
		float projected_wall_height = s_h / samples[i].distance;
		float y_top = s_h/2 - projected_wall_height;
		float y_bottom = s_h/2 + projected_wall_height;
		
		drawFillRect(i, y_bottom, i+1, y_top, shadeMono, shadeMono, shadeMono);
	}
*/
///*	
	unlockBits();
	memset(pixels, 0, dwBmpSize);
	for (int x = 0; x < s_w; x++) {
		const float projected_wall_height = (1 / samples[x].distance) * ((s_w / 2) / tan(player.fov / 2));
		const float y_top_unclamped = s_h / 2 - projected_wall_height;
		const float y_bottom_unclamped = s_h / 2 + projected_wall_height;

		const int drawStart = (y_top_unclamped < 0) ? 0 : y_top_unclamped;
		const int drawEnd = (y_bottom_unclamped > s_h) ? s_h : y_bottom_unclamped;	

		
		for(int y = drawStart; y<drawEnd; y++) {
			float intensity = samples[x].light_intensity;
			int texture_y = ((float)(y - y_top_unclamped) / (projected_wall_height)) * 32; //magic number 32 is the height of the texture...
			if (texture_y >= 32) texture_y -= 32;
			//unsigned int color = *(unsigned int*)(wall_texture_1.buffer + (int)((int)texture_y * wall_texture_1.width * 4 + (int)samples[x].texture_x * 4));
			COLOR32 color = GETPIXEL((int)(samples[x].texture_x * 32), (texture_y), wall_textures[1].buffer, wall_textures[1].width);
			color.r = ((((float)color.r / 255.0f) * intensity) * 255 > 255) ? 255 : (((float)color.r / 255.0f) * intensity) * 255;
			color.g = ((((float)color.g / 255.0f) * intensity) * 255 > 255) ? 255 : (((float)color.g / 255.0f) * intensity) * 255;
			color.b = ((((float)color.b / 255.0f) * intensity) * 255 > 255) ? 255 : (((float)color.b / 255.0f) * intensity) * 255;
			
			//COLOR32 color = GETPIXELRGB32((int)(texture_y * 32), (int)(samples[x].texture_x * 32), wall_texture_1.buffer, wall_texture_1.width);
			SETPIXEL(x, y, color, pixels, s_w);
		}
	}
//*/
	//------begin sprite drawing code-------
	//unlockBits();
	
	char visible = 0;	
	float x1, y1, x2, y2;
	float difX, difY;
	float length_hyp;
	float angle_to;
	float anglebtwn;
	float dotproduct;
	
	struct vec2 v1;
	struct vec2 v2;
	
	x1 = player.x;
	y1 = player.y;
	
	x2 = light1_sprite.x;
	y2 = light1_sprite.y;
	
	difX = x2 - x1;
	difY = y2 - y1;

	length_hyp = sqrt((difX * difX) + (difY * difY));

	angle_to = atan2(difY, difX) * -1;
	if (angle_to <= 0) angle_to += 2 * pi;
	
	v1.x = cos(player.a);
	v1.y = sin(player.a);
	
	v2.x = cos(angle_to);
	v2.y = sin(angle_to);
	
	dotproduct = dotVec2(v1, v2);
	//anglebtwn = acos(dotproduct);
	if (dotproduct > cos(player.fov / 2)) visible = 1; //faster! we can precalculate the cosine of fov/2
	
	if (visible && length_hyp >= 0.5f) {
		
		const float projected_height = (0.5 / length_hyp) * ((s_w / 2) / tan(player.fov / 2));
		const float y_top_unclamped = s_h/2 - projected_height;
		const float y_bottom_unclamped = s_h/2 + projected_height;
		
		const int drawStart = (y_top_unclamped < 0) ? 0 : y_top_unclamped;
		const int drawEnd = (y_bottom_unclamped > s_h) ? s_h : y_bottom_unclamped;
		const float aspect_ratio = (float)light1_sprite.img.height / (float)light1_sprite.img.width;
		const float width = projected_height / aspect_ratio * 2;
		
		if (angle_to < 0.8 && player.a > 5.7) {
			angle_to += 2 * pi;
		} else if (angle_to > 5.7 && player.a < 0.8) {
			angle_to -= 2 * pi;
		}
		
		float midpoint = (0.5 * ((player.a - angle_to) / (player.fov / 2)) + 0.5) * (float)s_w;
		//if (midpoint < 0) midpoint = (0.5 * ((player.a - angle_to + 2 * pi) / (player.fov / 2)) + 0.5) * (float)s_w;
		//else if (midpoint > s_w) midpoint = (0.5 * ((player.a - angle_to - 2 * pi) / (player.fov / 2)) + 0.5) * (float)s_w;
			
		for (float fx = 0; fx < width; fx++) {
			int objectColumn = midpoint + fx - (width / 2);
			for (float fy = drawStart; fy < drawEnd; fy++) {
				const float texture_x = fx / width;
				const float texture_y = (float)(fy - y_top_unclamped) / (2*projected_height);
				
				if (objectColumn >= 0 && objectColumn < s_w) {
					if (samples[objectColumn].distance >= length_hyp) {
						COLOR32 color = GETPIXEL((int)(texture_x * 32), (int)(texture_y * 32), light1_sprite.img.buffer, light1_sprite.img.width);
						
						if (*(int*)&color != *(int*)&transparency ) {
							SETPIXEL(objectColumn, (int)(fy), color, pixels, s_w);
						}
						//SETPIXELRGB32(objectColumn, (int)(fy), color, pixels, s_w);
					}
				}
			}
		}
	}
	//------end sprite drawing code---------
	lockBits();	
	blit();
}

int onCreate(void) {
	int error = 0;
	//player.fov = atan2(s_w / 2, DISTANCE_TO_PROJECTION_PLANE);
	error = loadImage("wall_debug.bmp", &wall_texture_1);
	error = loadImage("wall_2.bmp", &wall_texture_2);
	error = loadImage("wall_1.bmp", &wall_texture_3);
	wall_textures[0] = wall_texture_1;
	wall_textures[1] = wall_texture_2;
	wall_textures[2] = wall_texture_3;
	
	error = loadImage("lightbulb.bmp", &lightbulb_texture_1);
	light1_sprite.x = 32;
	light1_sprite.y = 8;
	light1_sprite.img = lightbulb_texture_1;
	return 0;	
}