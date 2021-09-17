#include "../engine/engine.h"
#include <math.h>

s_w = 1600;
s_h = 1000;
#define pi (3.141593)

//structs
struct entFlags {
	char facing : 1;
	char view : 1;
	char dda : 1;
	char moveDirect : 1;
	char connect : 1;
};

enum entityTypes {INVALID, PLAYER, LIGHT};

struct entity {	
	int size;
	float posX;
	float posY;
	float angle;
	float fov;
	COLOR32 *palette;
	struct entFlags flags;		
};

//globals
int tilesize = 50;

COLOR32 palette1[3] = {{255, 255, 255}, {0, 0, 255}, {0, 255, 0}};
COLOR32 palette2[3] = {{200, 200, 0}, {0, 0, 200}, {0, 200, 0}};
COLOR32 *palettes[] = {palette1, palette2};

struct entity player;
struct entity object;
#define numEntities (1)
struct entity *entities[numEntities];

//function prototypes
void drawGrid(void);

void cls(void);

void drawLineAtan2(float x, float y, float a, float l, int w, COLOR32 color);

struct entity newEntity(int size, float posX, float posY, float angle, float fov,
						int palette, char facing, char view, char moveDirect);
						
void drawEntity(struct entity *ent);

void handleEntityMovement(struct entity *ent);

void debugEntityVisibility(struct entity *source, struct entity *target);


//overriden functions

void update() {
	cls();
	drawGrid();
	handleEntityMovement(&player);
	
	for (int i = 0; i < numEntities; i++) {
		drawEntity(entities[i]);
		debugEntityVisibility(&player, entities[i]);
	}
	
	drawEntity(&player);	
	blit();
}

void onCreate() {
	player = newEntity(10, s_w / 2, s_h / 2, 0, pi/4, 0, 1, 1, 0);
	object = newEntity(5, s_w / 2 + 25, s_h / 2 + 25, 0, 0, 1, 0, 0, 0);
	entities[0] = &object;	
}


//function definitions

void drawGrid() {	
	static COLOR32 color = {233, 233, 233};	
	for (int x = 0; x < s_w; x += tilesize) {
		drawLine(x, 0, x, s_h, 1, color.r, color.g, color.b);
	}
	
	for (int y = 0; y < s_h; y += tilesize) {
		drawLine(0, y, s_w, y, 1, color.r, color.g, color.b);		
	}
}

struct entity newEntity(int size, float posX, float posY, float angle, float fov,
						int palette, char facing, char view, char moveDirect) {
	struct entity retVal = {size, posX, posY, angle, fov};
	retVal.palette = palettes[palette];
	retVal.flags.facing = (facing) ? 1 : 0;
	retVal.flags.view = (view) ? 1 : 0;
	retVal.flags.moveDirect = (moveDirect) ? 1 : 0;
	return retVal;
}

void handleEntityMovement(struct entity *ent) {
	static movementSpeed = 250;
	static turnSpeed = 2;
	if (ent->flags.moveDirect) {
		if (Keys[VK_UP].down) {
			ent->posY -= movementSpeed * fElapsedTime;
		}
		if (Keys[VK_DOWN].down) {
			ent->posX -= movementSpeed * fElapsedTime;
		}
		if (Keys[VK_LEFT].down) {
			ent->posY += movementSpeed * fElapsedTime;
		}
		if (Keys[VK_RIGHT].down) {
			ent->posX += movementSpeed * fElapsedTime;
		}
	} else {
		if (Keys['W'].down) {
			ent->posX += cos(ent->angle) * fElapsedTime * movementSpeed;
			ent->posY -= sin(ent->angle) * fElapsedTime * movementSpeed;
		}		
		if (Keys['S'].down) {
			ent->posX -= cos(ent->angle) * fElapsedTime * movementSpeed;
			ent->posY += sin(ent->angle) * fElapsedTime * movementSpeed;
		}		
		if (Keys['A'].down) {
			ent->angle += turnSpeed * fElapsedTime;
		}		
		if (Keys['D'].down) {
			ent->angle -= turnSpeed * fElapsedTime;
		}
		
		if (ent->angle > 2 * pi) ent->angle -= 2 * pi;
		else if (ent->angle < 0) ent->angle += 2 * pi;		
	}
}

void drawEntity(struct entity *ent) {
	drawFillCircle(ent->posX, ent->posY, ent->size, ent->palette[0].r, ent->palette[0].g, ent->palette[0].b);	
	if (ent->flags.facing) {
		drawLineAtan2(ent->posX, ent->posY, ent->angle, 256, 2, ent->palette[1]);
	}			
	if (ent->flags.view) {
		drawLineAtan2(ent->posX, ent->posY, ent->angle - (ent->fov/2), 256, 2, ent->palette[2]);
		drawLineAtan2(ent->posX, ent->posY, ent->angle + (ent->fov/2), 256, 2, ent->palette[2]);
	}
}

struct vec2 {
	float x;
	float y;
};

float dotVec2(struct vec2 v1, struct vec2 v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

void debugEntityVisibility(struct entity *source, struct entity *target) {
	char VISIBLE_ = 0;
	char VISIBLE__ = 0;
	float x1, y1, x2, y2;
	float difX, difY;
	float length_hyp;
	float angle_to, angle_to_dbg, anglebtwn, anglebtwn_dbg;
	float dotproduct, dotproduct_dbg;
	
	struct vec2 v1 = {0};
	struct vec2 v2 = {0};
	struct vec2 v3 = {0};
	
	static COLOR32 x_debug, y_debug, h_debug;
	static int debug_line_width = 1;
	
	x_debug.r = 255;
	x_debug.g = 0;
	x_debug.b = 0;
	y_debug.r = 0;
	y_debug.g = 0;
	y_debug.b = 255;
	h_debug.r = 255;
	h_debug.g = 0;
	h_debug.b = 255;
	
	x1 = source->posX;
	y1 = source->posY;
	x2 = target->posX;
	y2 = target->posY;
	
	difX = x2 - x1;
	difY = y2 - y1;

	length_hyp = sqrt((difX * difX) + (difY * difY));
	angle_to_dbg = atan2(difY, difX);
	angle_to = atan2(difY, difX) * -1;
	if (angle_to < 0) angle_to += 2 * pi;
	
	v1.x = cos(source->angle);
	v1.y = sin(source->angle);
	
	v2.x = cos(angle_to);
	v2.y = sin(angle_to);
	
	//v3.x = cos(angle_to_dbg);
	//v3.y = sin(angle_to_dbg);
	
	dotproduct = dotVec2(v1, v2);
	//dotproduct_dbg = dotVec2(v1, v3);
	anglebtwn = acos(dotproduct);
	//anglebtwn_dbg = acos(dotproduct_dbg);
	
	if (anglebtwn < source->fov / 2) VISIBLE_ = 1;
	if (dotproduct > cos(source->fov / 2)) VISIBLE__ = 1; //faster! we can precalculate the cosine of fov/2
	
	//draw horizontal component of triangle
	drawLine((int)x1, (int)y1, (int)(x1 + difX), y1, debug_line_width, x_debug.r, x_debug.g, x_debug.b);
	//draw vertical component of triangle
	drawLine((int)(x1 + difX), (int)y1, (int)(x1 + difX), (int)(y1 + difY), debug_line_width, y_debug.r, y_debug.g, y_debug.b);
	//draw connecting line
	drawLine((int)x1, (int)y1, (int)x2, (int)y2, debug_line_width, h_debug.r, h_debug.g, h_debug.b);
	
	if (VISIBLE_) drawFillRect(0, 0, 50, 50, 0x00, 0x00, 0xFF);
	else drawFillRect(0, 0, 50, 50, 0xFF, 0x00, 0x00);
	if (VISIBLE__) drawFillRect(0, 100, 50, 150, 0x00, 0x00, 0xFF);
	else drawFillRect(0, 100, 50, 150, 0xFF, 0x00, 0x00);
	
}

int is_angle_between(int target, int angle1, int angle2) 
{
	float temp;
	// make the angle from angle1 to angle2 to be <= 180 degrees
	float rAngle = fmod(fmod((angle2 - angle1+2*pi), 2*pi), 2*pi);
	if (rAngle >= pi) {
		temp = angle1;
		angle1 = angle2;
		angle2 = temp;
	}

	// check if it passes through zero
	if (angle1 <= angle2)
	return target >= angle1 && target <= angle2;
	else
	return target >= angle1 || target <= angle2;
}  

void drawLineAtan2(float x, float y, float a, float l, int w, COLOR32 color) {
	float endX;
	float endY;
	endX = x + cos(a) * l;
	endY = y - sin(a) * l;
	drawLine((int)x, (int)y, (int)endX, (int)endY, w, color.r, color.g, color.b);	
}


void cls() {
	unlockBits();
	memset(pixels, 0x44, dwBmpSize);
	lockBits();
}