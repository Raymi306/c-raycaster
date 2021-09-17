#include "../engine/engine.h"
#include <math.h>

#define pi (3.141593)

int s_w = 640;
int s_h = 480;
int screen_diagonal;
int gridsquare_size = 32;
enum Mode {EDIT, CAST};
int mode = CAST;

typedef struct i_vert2d {
	int x;
	int y;
} VERT2D;

typedef struct f_vert2d {
	float x;
	float y;
} fVERT2D;

typedef struct line {
	VERT2D v1;
	VERT2D v2;
} LINE;

struct lines {
	int length;
	LINE *array;
};

fVERT2D light1 = {10, 7};

struct lines gridlines;
struct lines generate_gridlines(void);
void destroy_lines(struct lines *target);

struct lines generate_gridlines(void) {
	int total_lines_in_grid = (s_w / gridsquare_size) + (s_h / gridsquare_size) - 2;
	LINE *array = (LINE*) malloc(sizeof(LINE) * total_lines_in_grid);
	int index = 0;
	LINE curline;
	VERT2D v1;
	VERT2D v2;
	struct lines result;

	for (int i = gridsquare_size; i < s_w; i += gridsquare_size) {
		VERT2D v1 = {i, 0};
		VERT2D v2 = {i, s_h};
		LINE curline = {v1, v2};
		array[index] = curline;
		index++;
	}

	for (int i = gridsquare_size; i < s_h; i += gridsquare_size) {
		VERT2D v1 = {0, i};
		VERT2D v2 = {s_w, i};
		LINE curline = {v1, v2};
		array[index] = curline;
		index++;
	}
	
	result.length = total_lines_in_grid;
	result.array = array;
	return result;
}

char *tiles;
int numTiles;

void destroy_lines_struct(struct lines *target) {
	free(target->array);
}

inline void drawGrid(void) {
	for (int i = 0; i < gridlines.length; i++) {
		drawLine(gridlines.array[i].v1.x, 
		gridlines.array[i].v1.y, 
		gridlines.array[i].v2.x,
		gridlines.array[i].v2.y, 
		1, 
		0xaa, 0xaa, 0xaa);
	}
}

inline void drawTiles(void) {
	for (int i = 0; i < s_w/gridsquare_size; i++) {
		for (int j = 0; j < s_h/gridsquare_size; j++) {
			if (tiles[j * (s_w/gridsquare_size) + i]) {
				drawFillRect(i*gridsquare_size, j*gridsquare_size,
				(i*gridsquare_size)+gridsquare_size,
				(j*gridsquare_size)+gridsquare_size,
				0x66, 0x00, 0x33); //660033
			}
			
		}	
	}
}

char getTile(short x, short y) {	
	if (x < 0 || x >= s_w / gridsquare_size || y < 0 || y >= s_h / gridsquare_size) {
		return -1;
	} else {
		return tiles[y * (s_w / gridsquare_size) + x];
	}
}

char mouseCoordsStrBuf[32];

inline void updateMouseCoords(void) {
	memset(mouseCoordsStrBuf, ' ', 32);
	sprintf(mouseCoordsStrBuf, "X: %i", Mouse.xPos);
	sprintf(mouseCoordsStrBuf+8, "Y: %i", Mouse.yPos);
	sprintf(mouseCoordsStrBuf+16, "%i,%i", Mouse.xPos/gridsquare_size, Mouse.yPos/gridsquare_size);
}

inline void drawMouseCoords(void) {	
	drawText(10, 10, mouseCoordsStrBuf, 8, 0, 0xff, 0xff);
	drawText(10, 30, mouseCoordsStrBuf+8, 8, 0, 0xff, 0xff);
	drawText(60, 10, mouseCoordsStrBuf+16, 8, 0, 0xff, 0xff);
}

struct _point {
	VERT2D coords;
	char exists : 1;
} point;

inline void updatePointCoords(void) {
	point.coords.x = Mouse.xPos;
	point.coords.y = Mouse.yPos;
}

struct _ray {
	float angle;
	char exists : 1;
} ray;

char angleStrBuf[32];

inline void updateRay(void) {
	
	float tangent = atan2(Mouse.yPos - point.coords.y, Mouse.xPos - point.coords.x) * -1;
	
	if (tangent < 0) {
		tangent = 2 * pi + tangent;
	}
	
	ray.angle = tangent;
	ray.exists = 1;
	sprintf(angleStrBuf, "A: %f", ray.angle);
}

inline void drawRayAngle(void) {
	drawText(60, 30, angleStrBuf, 16, 0, 0xff, 0xff);
}


inline void drawRay(void) {
	
	float endX;
	float endY;

	endX = point.coords.x + cos(ray.angle) * screen_diagonal;
	endY = point.coords.y - sin(ray.angle) * screen_diagonal;
	
	drawLine(point.coords.x, point.coords.y, (int)endX, (int)endY, 1, 0x22, 0xff, 0);	
}


char distancesStrBuf[32];

void updateDistances(float v, float h) {
	memset(distancesStrBuf, ' ', 32);
	sprintf(distancesStrBuf, "%f", v);
	sprintf(distancesStrBuf+16, "%f", h);
}

void drawDistances(void) {
	drawText(160, 10, distancesStrBuf, 16, 0xff, 0xff, 0);
	drawText(160, 30, distancesStrBuf+16, 16, 0x00, 0xff, 0xff);
}

void bounceRay(const float origin_x, const float origin_y, const float target_x, const float target_y) {
	double theta = atan2(target_y - origin_y, target_x - origin_x) * -1;
	
	if (theta < 0) {
		theta = 2 * pi + theta;
	}
	
	double p_x = origin_x;
	double p_y = origin_y;
	
	double off_x = origin_x - (int)origin_x;
	double off_y = origin_y - (int)origin_y;
	
	int step_x; //do we go left or right?
	int step_y; //up or down?
	
	int vertical_intersection_x;
	double vertical_intersection;
	double horizontal_intersection;
	int horizontal_intersection_y;
	
	int target_x_test = (int)target_x;
	int target_y_test = (int)target_y;
	
	int test_x;
	int test_y;
	
	int reconverted_x;
	int reconverted_y;
	
	char tile;
	
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
		
		reconverted_x = vertical_intersection_x * gridsquare_size;
		reconverted_y = vertical_intersection * gridsquare_size;
				
		drawFillCircle(reconverted_x, reconverted_y, 3, 0xff, 0xff, 0x00);
		

		
/*		if (test_x == target_x_test && test_y == target_y_test) {
			break; //ray reached light!
		}
*/		
		if (step_x == 1) {
			test_x = (int)vertical_intersection_x;
			test_y = (int)vertical_intersection;
			tile = getTile((int) vertical_intersection_x, (int) vertical_intersection);
			if (test_x == target_x_test && test_y  == target_y_test) {
				break; //ray reached light!
			}
		} else {
			test_x = (int) vertical_intersection_x - 1;
			test_y = (int) vertical_intersection;			
			tile = getTile((int) vertical_intersection_x - 1, (int) vertical_intersection);
			if (test_x == target_x_test && test_y == target_y_test) {
				break; //ray reached light!
			}
		}		

		if (tile == 1) { //either hit wall...
			break;
		} else if (tile == -1) {  //or went out of bounds
			break;
		}
		
		vertical_intersection_x += step_x;
		vertical_intersection -= step_x * tan(theta);
	}
	//first horizontal?
	horizontal_intersection = p_x - (step_y / tan(theta) * off_y);
	horizontal_intersection_y = p_y + (step_y * off_y);
	
	while(1) {
		
		reconverted_x = horizontal_intersection * gridsquare_size;
		reconverted_y = horizontal_intersection_y * gridsquare_size;
	
		drawFillCircle(reconverted_x, reconverted_y, 3, 0x00, 0xff, 0xff);
	
/*			if (test_x == target_x_test && test_y == target_y_test) {
				break; //ray reached light!
			}	
*/			
		if (step_y == 1) {
			test_x = (int)horizontal_intersection;
			test_y = (int)horizontal_intersection_y;
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y);
			if (test_x == target_x_test && test_y == target_y_test) {
				break; //ray reached light!
			}
		} else {
			test_x = (int)horizontal_intersection;
			test_y = (int)horizontal_intersection_y - 1;
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y - 1);
			if (test_x == target_x_test && test_y == target_y_test) {
				break; //ray reached light!
			}
		}
		
		if (tile == -1) {
			break;
		} else if (tile == 1) {
			break;
		}
		
		horizontal_intersection -= step_y / tan(theta);
		horizontal_intersection_y += step_y;
	}
	//Dk how we could reach here
}

inline void findIntersections(void) {
	//first, need to convert from client screen coordinates to grid with fractional component
	float p_x = point.coords.x / (float)gridsquare_size;
	float p_y = point.coords.y / (float)gridsquare_size;
	
	float off_x = p_x - (int)p_x; //get fractional component
	float off_y = p_y - (int)p_y;
	
	int step_x; //do we go left or right?
	int step_y; //up or down?
	
	float vertical_intersection_x;
	float vertical_intersection;	
	float horizontal_intersection;
	float horizontal_intersection_y;
	
	float theta = ray.angle;
	
	int reconverted_x;
	int reconverted_y;
	
	float dv;
	float dh;
	
	float dx;
	float dy;
	
	char tile;
	
	//we will need to know which quadrant the angle is in to determine what direction to step x and y in
	//remember that in the level array, 0,0 is top left, so going 'up' will require subtraction
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
	
	//how far are we to first intersections?
	
	vertical_intersection_x = p_x + (step_x * off_x);
	vertical_intersection = p_y - (step_x * (tan(theta) * off_x));
	horizontal_intersection = p_x - (step_y / tan(theta) * off_y);
	horizontal_intersection_y = p_y + (step_y * off_y);	
	
	while(1) {
		
		reconverted_x = vertical_intersection_x * gridsquare_size;
		reconverted_y = vertical_intersection * gridsquare_size;
				
		drawFillCircle(reconverted_x, reconverted_y, 3, 0xff, 0xff, 0x00);
		
		if (step_x == 1) {
			tile = getTile((int) vertical_intersection_x, (int) vertical_intersection);
		} else {
			tile = getTile((int) vertical_intersection_x-1, (int) vertical_intersection);
		}
		
		if (tile == -1) {//oob
			dv = -1;
			break;
		} else if (tile == 1) {//hit
			break;
		}
		

		vertical_intersection_x += step_x;
		vertical_intersection -= step_x * tan(theta);
	}
	

	while(1) {
		
		reconverted_x = horizontal_intersection * gridsquare_size;
		reconverted_y = horizontal_intersection_y * gridsquare_size;
	
		drawFillCircle(reconverted_x, reconverted_y, 3, 0x00, 0xff, 0xff);
		
		if (step_y == 1) {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y);
		} else {
			tile = getTile((int) horizontal_intersection, (int) horizontal_intersection_y - 1);
		}		
		
		if (tile == -1) {//oob
			dh = -1;
			break;
		} else if (tile == 1) {//hit
			break;
		}
		

		horizontal_intersection -= step_y / tan(theta);
		horizontal_intersection_y += step_y;
	}
	
	if (dv != -1) {	
		dx = vertical_intersection_x - p_x;
		dy = vertical_intersection - p_y;
		
		dv = sqrt((dx * dx) + (dy * dy));
	}
	if (dh != -1) {
		dx = horizontal_intersection - p_x;
		dy = horizontal_intersection_y - p_y;
		
		dh = sqrt((dx * dx) + (dy * dy));
	}
	updateDistances(dv, dh);
/*
	if (dv == -1) {
		bounceRay(horizontal_intersection, horizontal_intersection_y, light1.x, light1.y);
	} else if (dh == -1) {
		bounceRay(vertical_intersection_x, vertical_intersection, light1.x, light1.y);
	} else if (dv < dh) {
		bounceRay(vertical_intersection_x, vertical_intersection, light1.x, light1.y);
	} else {
		bounceRay(horizontal_intersection, horizontal_intersection_y, light1.x, light1.y);
	}
*/
}

void drawLight() {
	drawFillCircle(light1.x * gridsquare_size, light1.y * gridsquare_size, 6, 0xff, 0xff, 0xff);
}

void update() {	
	
	char MKEYDOWN = 0;
	
	drawFillRect(0, 0, s_w, s_h, 0x00, 0x00, 0x00); //bg
	drawTiles();
	drawGrid();
	drawLight();
	
	if (Keys[0x4D].up) { //M 
		mode = (mode == CAST) ? EDIT : CAST;
		Keys[0x4D].up = 0;
	}
	
	switch(mode) {
		case CAST:
			if (Mouse.leftDown) {
				point.exists = 1;
				ray.exists = 0;
				updatePointCoords();
			} else if (Mouse.rightDown) {
				if (abs(Mouse.xPos - point.coords.x) <= 4 && abs(Mouse.yPos - point.coords.y) <= 4) {
					point.exists = 0;
				} else {
					updateRay();
				}
			}
			if (ray.exists && point.exists) {
				drawRay();
				drawFillCircle(point.coords.x, point.coords.y, 4, 0xff, 0x33, 0);
				findIntersections();
			} else if (point.exists) {
				drawFillCircle(point.coords.x, point.coords.y, 4, 0xff, 0x33, 0);
			}

			//tan = opp / adj
			//tan(ray.angle) * adj = opp;
			//tan(ray.angle) * s_w = y;
			//tan(ray.angle) = y / x;
			drawFillRect(8, 8, s_w - 8, 48, 0, 0x33, 0xcc); //bg for debug text
			updateMouseCoords();
			drawMouseCoords();
			drawRayAngle();
			drawDistances();
			break;
		case EDIT:
			if (Mouse.leftDown) {
				tiles[(Mouse.yPos / gridsquare_size) * (s_w / gridsquare_size) + Mouse.xPos / gridsquare_size] = 1;
			} else if (Mouse.rightDown) {							
				tiles[(Mouse.yPos / gridsquare_size) * (s_w / gridsquare_size) + Mouse.xPos / gridsquare_size] = 0;
			}
			break;
			
	}
	blit();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	gridlines = generate_gridlines();
	screen_diagonal = sqrt(((long)s_w*(long)s_w) + ((long)s_h*(long)s_h));
	memset(mouseCoordsStrBuf, 0, 16);
	memset(angleStrBuf, 0, 16);
	memset(&point, 0, sizeof (struct _point));
	memset(&ray, 0, sizeof (struct _ray));
	numTiles = (s_w / gridsquare_size) * (s_h / gridsquare_size);
	tiles = (char*)calloc(numTiles, sizeof(char));
	//tiles[] = 1;
	start(hInstance, hPrevInstance, lpCmdLine, nCmdShow, s_w, s_h);
	destroy_lines_struct(&gridlines);
	free(tiles);
	printf("Finish!\n");
	fflush(stdout);
	return 0;
}