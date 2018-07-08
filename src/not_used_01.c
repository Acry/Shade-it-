//BEGIN HEAD
//BEGIN DESCRIPTION
/* DEFINED PROGRESS GOALS
 * Draw 4 rectangles and rotate them individually
 * around their center.
 * 
 * implement helper functions to create the rects:
 * init_rect
 */

//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
#include <math.h>
//local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define WW 		550
#define WH 		(WW/16)*12
#define GL_GREY 	.5, .5, .5, 1
#define GL_WHITE 	1,1,1,1
#define GL_PURPLE 	0.992, 0.501, 1,1
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
struct vec2{
	double x;
	double y;
};

struct rect{
	struct vec2 tl;
	struct vec2 br;
	
	struct vec2 tr;
	struct vec2 bl;
	
	struct vec2 center;
};
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww=WW;
int wh=WH;
struct rect rect[4];
//BEGIN VISIBLES
//END 	VISIBLES

//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
void init_rect(struct rect *, int, int, int, int);
void init_light(void);
double norm_RGB		(double);
double rescale_RGB	(double);

double norm_NDC_x	(double);
double norm_NDC_y	(double);
double rescale_NDC_x	(double);
double rescale_NDC_y	(double);
//END	FUNCTION PROTOTYPES

//END 	HEAD

//BEGIN MAIN FUNCTION
int main(int argc, char *argv[])
{

(void)argc;
(void)argv;

//BEGIN INIT
init();
//BEGIN WINDOW
SDL_SetWindowPosition(Window,0,0);
SDL_SetWindowSize(Window,ww,wh);
SDL_SetWindowTitle(Window, "Rotate Rects");
SDL_ShowWindow(Window);
//END WINDOW

glClearColor(GL_GREY);
glClear(GL_COLOR_BUFFER_BIT);

glViewport (0, 0, ww, wh);

//BEGING INIT RECTS
//rect top left
init_rect(&rect[0], 10, wh-10, ww/2-5, wh/2+5);

//rect bot left
init_rect(&rect[1], 10, wh/2-5, ww/2-5, 10);

//rect top right
init_rect(&rect[2], ww/2+5, wh-10, ww-10, wh/2+5);

//rect bot right
init_rect(&rect[3], ww/2+5, wh/2-5, ww-10, 10);

//END INIT RECTS
init_light();
#if 0
// Light
glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);

GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

// Create light components
GLfloat ambientLight[] 	= { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat diffuseLight[] 	= { 0.8f, 0.8f, 0.8, 1.0f };
GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat position[] 	= { -1.5f, 1.0f, -4.0f, 1.0f };

// Assign created components to GL_LIGHT0
glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
glLightfv(GL_LIGHT0, GL_POSITION, position);

float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
glMateriali(GL_FRONT, GL_SHININESS, 96);

float colorBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorBlue);
#endif
glColor4f(GL_WHITE);
glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

glShadeModel (GL_SMOOTH);

glOrtho (-1.5*(GLfloat)ww/(GLfloat)wh,
	 1.5*(GLfloat)ww/(GLfloat)wh, -1.5, 1.5, -10.0, 10.0);

glMatrixMode(GL_MODELVIEW);

SDL_Event event;
int running = 1;
//END   INIT
double a=1;
//BEGIN MAIN LOOP
while(running){

	//BEGIN EVENT LOOP
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running =0;
		}
		if(event.type == SDL_MOUSEMOTION){
			;
		}
		if(event.type == SDL_MOUSEBUTTONDOWN){
			if(event.button.button == SDL_BUTTON_RIGHT){
				;
			}
			if(event.button.button == SDL_BUTTON_MIDDLE){
				;
			}
			if(event.button.button==SDL_BUTTON_LEFT){
				;
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running =0;
					break;

				case SDLK_r:
				case SDLK_BACKSPACE:
					break;

				case SDLK_p:	
				case SDLK_SPACE:
					break;
					
				default:
					break;
			}
		}
	}
	//END   EVENT LOOP
	//BEGIN RENDERING
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i=0; i<4; i++){
		glPushMatrix();
		glTranslated(rect[i].center.x, rect[i].center.y, 0); 
		glRotated(-a, 0, 1, 0);
		glTranslated(-rect[i].center.x, -rect[i].center.y, 0); //move object to center
			
		glBegin(GL_QUADS);                          	// Draw A Quad
		glVertex3d(rect[i].tl.x, rect[i].tl.y, 0.0f);   // Top Left Corner
		glVertex3d(rect[i].tr.x, rect[i].tr.y, 0.0f);   // Top Right Corner
		glVertex3d(rect[i].br.x, rect[i].br.y, 0.0f);   // Bottom Right Corner
		glVertex3d(rect[i].bl.x, rect[i].bl.y, 0.0f);   // Bottom Left Corner
		glEnd();
		glPopMatrix();
	}
	a+=0.1;
	//optional
	if (a>180)
		a=0;

	SDL_GL_SwapWindow(Window);
	//END   RENDERING
}
//END   MAIN LOOP


exit_();
return EXIT_SUCCESS;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS
void init_rect(struct rect *rect, int tlx, int tly, int brx, int bry)
{
	rect->tl.x=norm_NDC_x(tlx);
	rect->tl.y=norm_NDC_y(tly);
	rect->br.x=norm_NDC_x(brx);
	rect->br.y=norm_NDC_y(bry);
	
	rect->tr.x=norm_NDC_x(brx);
	rect->tr.y=norm_NDC_y(tly);
	rect->bl.x=norm_NDC_x(tlx);
	rect->bl.y=norm_NDC_y(bry);
	
	rect->center.x=norm_NDC_x((brx - tlx)/2+tlx);
	SDL_Log("c.x: %d", (brx-tlx)/2+tlx);
	rect->center.y=norm_NDC_y((bry - tly)/2+tly);
	SDL_Log("c.y: %d", (bry - tly)/2+tly);

}
double norm_RGB(double x)
{
	return (x - 0) / (255 - 0);
}
double rescale_RGB(double xr)
{
	return xr * 255;
}
double norm_NDC_x(double x)
{
	return (2 * (x - 0) / (ww - 0))-1;
}
double norm_NDC_y(double y)
{
	return (2 * (y - 0) / (wh - 0))-1;
}
double rescale_NDC_x(double xr)
{
	return ww * (xr - -1) / 2;
}
double rescale_NDC_y(double xr)
{
	return wh * (xr - -1) / 2;
}

void init_light(void)
{
	static float ambient[] =
	{0.1, 0.1, 0.1, 1.0};
	static float diffuse[] =
	{1.0, 1.0, 1.0, 1.0};
	static float position[] =
	{0.0, 1.0, 0.0, 0.0};
	static float mat_shininess[] =
	{90.0};
	static float mat_specular[] =
	{0.8, 0.8, 0.8, 1.0};
	static float mat_diffuse[] =
	{0.46, 0.66, 0.795, 1.0};
	static float mat_ambient[] =
	{0.0, 0.1, 0.2, 1.0};
	static float lmodel_ambient[] =
	{0.4, 0.4, 0.4, 1.0};
	static float lmodel_localviewer[] =
	{0.0};
	
	glFrontFace(GL_CW);
	
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_localviewer);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
		
	glClearColor(0.0, 0.5, 0.9, 0.0);
}
//END   FUNCTIONS
