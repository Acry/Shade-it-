//BEGIN HEAD
//BEGIN DESCRIPTION
/* 
 * 
 * 
 */
//END 	DESCRIPTION

//BEGIN INCLUDES
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> 	// Just for the icon - easy to strip out
#include "default_shaders_3a.h" // Contains def vertex and frag shader

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <jansson.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"
#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"
//END 	INCLUDES

//BEGIN DEFINES
#define NEW_FILE "new.stfs.glsl"
#define EDITOR	 "/usr/bin/kate"
#define ST	 "assets/glsl/st/"
#define TEXTURES "assets/textures/"

#define BUFFER_SIZE  	(256 * 1024)  /* 256 KB */
#define URL_SIZE     	512
#define URL_BASE	"https://www.shadertoy.com"
//END 	DEFINES

//BEGIN DATASTRUCTURES
struct write_result
{
	char *data;
	int   pos;
};

GLfloat vertices[] = {
	-1.0f,   -1.0f,
	 1.0f,   -1.0f,
	-1.0f,    1.0f,
	 1.0f,    1.0f,
};
//END 	DATASTRUCTURES

//BEGIN GLOBALS
SDL_Window *Window=NULL;
// prolly need an array[4]
// actually think i may be 4*4
// GLuint texture[4];

GLuint 		texture;

// channel resolution input
float resos[4][3] = { {0.0,0.0,0.0}, {0.0,0.0,0.0}, {0.0,0.0,0.0}, {0.0,0.0,0.0} };

int 		ww			=500;
int 		wh			=281;

char  		Running 		=  1;	// controls main loops
char 		pause_rendering		=  0;	// controls update
GLuint 		argument_provided	=  0;
int 		make_screenshot		=  0;
float 		screenshot_timer	=  0;
char 		*ST_shader_ID;
char 		*ST_shader_name;
int 		NEW_SHADER		=  0;

//BEGIN FONTSTASH
struct FONScontext *fs;
int fontNormal 	= FONS_INVALID;
int fontBold 	= FONS_INVALID;
float dx = 40, dy = 40;
unsigned int white;
unsigned int black;
//BEGIN FONTSTASH

//inotify and epoll
int wd, fd, efd, cfg;
struct epoll_event ev;

const char editor_call[]={ ""EDITOR" "NEW_FILE };

//3 from header, one optionally loaded/new/edit
GLuint current_program = 0;	// tracks current used shading program
GLuint current_programs= 0;	// number of currently compiled programs
GLuint shading_program[4];	// Max GPU programs

GLint  attrib_position;		// Vertex Shader Attribute

// timing
GLint uniform_gtime;
GLint uniform_date;		// Year, month, day, time in seconds in .xyzw
GLint uniform_TimeDelta;
GLint uniform_ChannelTime[4];
float runtime;			// accumulates delta time from fTimeDelta()

GLint uniform_FrameRate;
GLint uniform_FrameCount;	// Current frame counter (int)
//NOTE dunno if fTime() is still needed
//BEGIN MOUSE
SDL_Point	mouse;
SDL_Point	mouseOri;
SDL_bool 	mouse_follow	= SDL_FALSE;
GLint 		uniform_mouse;	// image/buffer xy = current pixel coords (if LMB is down). zw = click pixel
//END 	MOUSE

GLint uniform_res;		// Resolution

//BEGIN CHANNELS
// for( var i=0; i<this.mInputs.length; i++ )
// {
// 	var inp = this.mInputs[i];
// 	if( inp==null )                  	header += "uniform sampler2D iChannel"   + i + ";\n";
// 	else if( inp.mInfo.mType=="cubemap" ) 	header += "uniform samplerCube iChannel" + i + ";\n";
// 	else if( inp.mInfo.mType=="volume"  ) 	header += "uniform sampler3D iChannel"   + i + ";\n";
// 	else                                 	header += "uniform sampler2D iChannel"   + i + ";\n";
// }
GLint sampler_channel[4];
GLint uniform_iChannelResolution[4];
//END 	CHANNELS

GLint uniform_srate;			// The sound sample rate (typically 44100)
//END  GLOBALS

//BEGIN FUNCTIONS
int 	handle_droptext		(char *);
int 	handle_dropfile		(char *);
char 	*get_shader_shorthash	(char *);
void 	save_screenshot		(int);
char   	*request_ST_json	(const char *);
size_t	 write_response		(void *, size_t , size_t , void *);

// load and save texture if needed
int 	 save_image		(char *, char *);

SDL_Surface * flip_vertical	(SDL_Surface *);
SDL_Surface * flip_horizontal	(SDL_Surface *);

//BEGIN NEW SHADER
void load_image(char *, int, const char *, const char *, const char *, const char *, const char *);
// writes def FS from default_fragment_shader_1 to NEW_FILE and calls EDITOR
// first time
int    write_STFS		(void);

struct epoll_event reset_poll	(int);
// tries to compile and use new program if hash was changed
GLuint compile_STFS		(void);
//END 	NEW SHADER

// loads custom Vertex- and Fragment Shader from File
GLuint custom_shaders		(const char *	, const char *);

// uses default Vertex-Shader and loads a ST-Shader
GLuint STFS			(const char *);

// loads a shader from file and returns the compile shader
GLuint GetShader		(GLenum 	, const char *);

// loads default Vertex- and Fragment Shader if no Arguments are provided
GLuint default_shaders		(GLuint);
GLuint default_vertex		(void);

GLuint compile_shader		(GLenum type, GLsizei , const char **);
GLuint program_check		(GLuint);

void   query_vars		(GLuint);

void   shader_switch		(void);
float  fTime			(void);	//overall runtime
float  fTimeDelta		(void); //frame time

void   init_glew		(void);
const char  *read_file		(const char *);
//END 	FUNCTIONS
//END 	HEAD

//BEGIN MAIN
int main(int argc, const char *argv[])
{
	//BEGIN INIT
	if (argc>1){
		argument_provided=1;
// 		SDL_Log("%s",argv[1]);
	} else {
// 		SDL_Log("No argument provided.");
	}
	//BEGIN INIT SDL2
	SDL_Init(SDL_INIT_VIDEO);
	Window = SDL_CreateWindow("Shade it! 0.5",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		ww, wh,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL |SDL_WINDOW_RESIZABLE);

		//BEGIN ICON
		SDL_Surface *icon;
		icon=IMG_Load("./assets/gfx/icon.png");
		SDL_SetWindowIcon(Window, icon);
		SDL_FreeSurface(icon);
		//END 	ICON
		
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GLContext glContext = SDL_GL_CreateContext(Window);
	//END 	INIT SDL2

	init_glew();
	//BEGIN FONTSTASH
	fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
	if (fs == NULL) {
		printf("Could not create font stash.\n");
		return -1;
	}

	fontNormal 	= fonsAddFont(fs, "sans", "assets/DroidSerif-Regular.ttf");
	
	if (fontNormal == FONS_INVALID) {
		printf("Could not add font normal.\n");
		return -1;
	}
	fontBold 	= fonsAddFont(fs, "sans-bold", "assets/DroidSerif-Bold.ttf");
	if (fontBold == FONS_INVALID) {
		printf("Could not add font bold.\n");
		return -1;
	}
	white = glfonsRGBA(255,255,255,255);
	black = glfonsRGBA(0,0,0,255);
	fonsSetFont(fs, fontNormal);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,ww,wh,0,-1,1);
	
	glMatrixMode	(GL_MODELVIEW);
	glColor4ub	(255,255,255,255);
	fonsSetSize	(fs, 38.0f);
	fonsSetColor	(fs, white);
	fonsSetBlur	(fs, 0);
	fonsDrawText	(fs, dx-20,dy,"Hang on... downloading data.", NULL);
	SDL_GL_SwapWindow(Window);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//END FONTSTASH

	//BEGIN SHADER INIT

// 	SDL_Log("Trying to build default shaders");
	for (int i=0; i<3; i++){
		shading_program[i] = default_shaders(i);
		current_programs++;
		current_program=1;
// 		SDL_Log("i: %d", i);
	}
	if (shading_program[0] == 0){
		Running = 0;
		if (glGetError()!=0)
			SDL_Log("glError: %#08x\n", glGetError());
	} else {


		switch(argument_provided)
		{
			case 0:

				query_vars(0);
// 				STFS_vars(0);
				glUseProgram(shading_program[0]);
// 				SDL_Log("Using default shader");
				break;
			case 1:
// 				SDL_Log("Trying to build shadertoy shader");
				shading_program[3] = STFS(argv[1]);

// 				SDL_Log("Using shading program %d\n", shading_program[3]);
				query_vars(3);
// 				STFS_vars(3);
				glUseProgram(shading_program[3]);
				current_programs++;
				current_program=4;
				make_screenshot=1;
				break;
			default:
				//some statements to execute when default;
				break;
		}
	}
	//END 	SHADER INIT

	//BEGIN WALLCLOCK
	// Year, month, day, time in seconds in .xyzw
	time_t now;
	now = time(NULL);
	struct tm *tm;
	tm = localtime (&now);
//	SDL_Log ("%04d-%02d-%02d %02d:%02d:%02d\n",
//		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
//	 tm->tm_hour, tm->tm_min, tm->tm_sec);
	//END 	WALLCLOCK

	struct epoll_event ev;
	SDL_Event event;

	//END	INIT

// 	SDL_Log("currently compiled programs: %u",current_programs);
// 	SDL_Log("currently running program: %d", current_program);
// 	return EXIT_SUCCESS;
	//BEGIN MAINLOOP
	runtime=0;
	while (Running){
		float frametime=fTimeDelta();
		//BEGIN EVENTS
		SDL_GetMouseState(&mouse.x, &mouse.y);
		while ( SDL_PollEvent(&event) ){
			if (event.type == SDL_QUIT)
				Running = 0;
			if (event.type == SDL_DROPTEXT)
				handle_droptext(event.drop.file);
			if (event.type == SDL_DROPFILE)
				handle_dropfile(event.drop.file);

			if(event.type == SDL_WINDOWEVENT){
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					ww=event.window.data1;
					wh=event.window.data2;
					glViewport (0, 0, ww, wh);
					glUniform3f(uniform_res, ww, wh, 0.0f);
				}
			}
			if(event.type == SDL_MOUSEMOTION){
				;
			}
			if(event.type == SDL_MOUSEBUTTONDOWN){
				if(event.button.button == SDL_BUTTON_RIGHT){
					;
				}
				if(event.button.button == SDL_BUTTON_MIDDLE){
					shader_switch();
				}
				if( event.button.button==SDL_BUTTON_LEFT){
					if( !mouse_follow ){
						mouseOri.x=mouse.x;
						mouseOri.y=wh-mouse.y;
						mouse_follow=SDL_TRUE;

					}
					
				}
				if( mouse_follow && event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT){
					mouse_follow=SDL_FALSE;
				}
				
			}
			if( mouse_follow && event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT){
				mouse_follow=SDL_FALSE;
			}
			if(event.type == SDL_KEYDOWN ){
				switch(event.key.keysym.sym ){
					case SDLK_ESCAPE:
						Running = 0;
						break;
					case SDLK_n:
						write_STFS();
						ev = reset_poll(0);
						break;
					case SDLK_c:
						compile_STFS();
						break;
					case SDLK_r:
					case SDLK_BACKSPACE:
						break;
						
					case SDLK_p:	
						pause_rendering=pause_rendering^1;
						break;
					case SDLK_SPACE:
						break;
					case SDLK_s:
						save_screenshot(1);
						break;
					default:
						break;
				}
			}

		}
		//END 	EVENTS

		//BEGIN UPDATE
		
if (!pause_rendering){		
		runtime+=frametime;
		if (make_screenshot){
			screenshot_timer=runtime;
			if (screenshot_timer>5){
				save_screenshot(0);
				make_screenshot=0;
			}
		}
		
		glUseProgram(shading_program[current_program-1]);
		glEnableVertexAttribArray	(attrib_position);
		glVertexAttribPointer		(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindSampler(texture, sampler_channel[0]);
		
// 		if (sampler_channel[0] >-1)
// 			glUniform1i(sampler_channel[0], texture);
// 		if (uniform_iChannelResolution[0]>-1)
// 			glUniform3f(uniform_iChannelResolution[0], resos[0][0],resos[0][1],resos[0][2]);
		
		if (NEW_SHADER){
			if (epoll_wait(efd, &ev, sizeof(ev), 0) > 0){
				// SDL_Log("try compile");
				compile_STFS();
				ev = reset_poll(0);
			}
		}

		if (mouse_follow&&uniform_mouse>-1){
			glUniform4f(uniform_mouse, mouse.x, wh-mouse.y, mouseOri.x, mouseOri.y);
		}
		if (uniform_res>-1)
			glUniform3f			(uniform_res, ww, wh, 0.0f);
		if (uniform_date>-1){
			// Year, month, day, time in seconds in .xyzw
			now 	= time		(NULL);
			tm 	= localtime 	(&now);
			// JAVASCRIPT
			// d.getHours()*60.0*60 + d.getMinutes()*60 + d.getSeconds()  + d.getMilliseconds()/1000.0 ];
			// C - I skip the milliseconds for now
			// (tm->tm_hour*60.0*60)+(tm->tm_min*60)+(tm->tm_sec)
			glUniform4f(uniform_date, (float)tm->tm_year+1900,(float)tm->tm_mon+1, (float)tm->tm_mday, (tm->tm_hour*60.0*60)+(tm->tm_min*60)+(tm->tm_sec));
		}
		if (uniform_gtime>-1)
			glUniform1f(uniform_gtime, runtime);

		if (uniform_TimeDelta>-1)
			glUniform1f(uniform_TimeDelta, frametime);
		//END UPDATE
		//BEGIN RENDERING

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (ST_shader_name!=NULL && make_screenshot && current_program==4){
			glUseProgram(0);
			glDisableVertexAttribArray	(attrib_position);
			fonsSetSize(fs, 38.0f);
			fonsSetFont(fs, fontBold);
			fonsSetColor(fs, black);
			fonsSetSpacing(fs, 0.0f);
			fonsSetBlur(fs, 3.0f);
			fonsDrawText(fs, dx,dy+2,ST_shader_name,NULL);
			
			fonsSetColor(fs, white);
			fonsSetBlur(fs, 0);
			fonsDrawText(fs, dx,dy,ST_shader_name, NULL);
		}

		SDL_GL_SwapWindow(Window);
}
		//END 	RENDERING
	}
	//END 	MAINLOOP

	//BEGIN EXIT
	if (NEW_SHADER==1){
		ev = reset_poll(1);
	}
	for (int i=0; i<5; i++){
		if (glIsProgram(shading_program[i]))
			glDeleteProgram(shading_program[i]);
	}
	if (ST_shader_ID!=NULL)
		free(ST_shader_ID);
	if (ST_shader_name!=NULL)
		free(ST_shader_name);
	SDL_GL_DeleteContext(glContext);
	SDL_Quit();

	return EXIT_SUCCESS;
	//END 	EXIT
}
//END	MAIN

//BEGIN FUNCTIONS

//BEGIN GPU PROGRAM CREATION
GLuint custom_shaders(const char *vsPath, const char *fsPath)
{
	GLuint vertexShader;
	GLuint fragmentShader;

	vertexShader   = GetShader(GL_VERTEX_SHADER,   vsPath);
	fragmentShader = GetShader(GL_FRAGMENT_SHADER, fsPath);

	shading_program[3] = glCreateProgram();

	glAttachShader(shading_program[3], vertexShader);
	glAttachShader(shading_program[3], fragmentShader);

	glLinkProgram(shading_program[3]);

	GLuint status;
	status=program_check(shading_program[3]);
	if (status==GL_FALSE)
		return 0;
	return shading_program[3];

}

GLuint STFS(const char *ID)
{
	GLuint vtx;
	vtx = default_vertex();
	if (vtx==0)
		return 0;

	GLuint frag;
	const char *sources[4];
	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
		
	
	ST_shader_ID=strdup(ID);
	if( access( ID, F_OK ) != -1 ){
// 		SDL_Log("file exists");
		// still need to load json for more infos
		
		// 	char *str = strdup(ID);
		// 	char *token = strtok(str, ".");
		// 	char *last;
		// 	while (token != NULL){
		// 		last=token;
		// 		token = strtok(NULL, ".");
		// 	}
		// 	SDL_Log("last: %s", last);
		
		// check type
		
		// non existing
		// code
		// json
		
		// if stfs
		
		sources[2] = read_file(ID);
		if (sources[2]==NULL){
			SDL_Log("something went utterly wrong!");
			return 0;
		}
	}
	else{
// 		SDL_Log("no file, but ID provided");
		// check assets folder
		// glsl/st/ID.json
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%s%s.json",ST,ID);
// 		SDL_Log("%s",buffer);
		const char *text;
		if( access( buffer, F_OK ) != -1 ){
// 			SDL_Log("JSON exists, nothing to download!");
			text=read_file(buffer);
		}
		else{
// 			SDL_Log("need to download!");
			text = request_ST_json(ID);
			// save shader json
			FILE *fp;
			fp = fopen( buffer , "w" );
			fputs(text, fp);
			fclose(fp);
			
			// set file non executable, one probably won't need this
			char cmd[300];
			snprintf(cmd, sizeof(cmd), "chmod -x %s", buffer);
			system(cmd);
		}
		const char *json_file;
		json_file = text;

		if(!json_file)
			return 0;
		// decode json
		json_error_t 	 error;
		json_t 		*root;
		root = json_loads(json_file, 0, &error);
		if(!root){
			fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
			return 1;
		}
		if(json_is_array(root)){
			for(size_t i = 0; i < json_array_size(root); i++){
				json_t *data = json_array_get(root, i);
				// extract shader name
				json_t 	*info, *name;
				const char 	*shader_name;
				info 	= json_object_get(data, "info");
				name 	= json_object_get(info	, "name");
				shader_name = json_string_value(name);
// 				SDL_Log("Shader-Name: %s\n",shader_name);
				ST_shader_name=strdup(shader_name);
				json_t 		*renderpass;
				renderpass 	= json_object_get(data, "renderpass");
				if(json_is_array(renderpass)){
					for(size_t i = 0; i < json_array_size(renderpass); i++){
						json_t *data   = json_array_get(renderpass, i);
						json_t *inputs = json_object_get(data, "inputs");
						
						// Get and dump code
						json_t *code   = json_object_get(data, "code");
						sources[2] = strdup(json_string_value(code));
// 						printf("%s\n",sources[2]);
						if(json_is_array(inputs)){
							for(size_t i = 0; i < json_array_size(inputs); i++){
								json_t *data 	= json_array_get(inputs	, i);
								
								// getting type
								json_t *type;
								type = json_object_get(data, "type");
								
								// only texture support for now
								const char 	*requested_info;
								requested_info  = json_string_value(type);
								if (!strcmp("texture",requested_info)){
// 									SDL_Log("very soon");
// 									printf("type: %s\n",requested_info);
									json_t *channel;
									channel= json_object_get(data, "channel");
									int chan = json_integer_value(channel);
// 									printf("on Channel: %d\n", chan);

									json_t *src;
									src = json_object_get(data, "filepath");
									requested_info  = json_string_value(src);
									char *str = strdup(requested_info);
									char *token = strtok(str, "/");
									char *last;
									while (token != NULL){
										last=token;
										token = strtok(NULL, "/");
									}
// 									SDL_Log("last: %s", last);
									// check/get texture
									char url[URL_SIZE];
									snprintf(url, URL_SIZE, URL_BASE"%s", requested_info);
									save_image(url,last);

									// Get sampler inputs
									json_t *sampler;
									sampler = json_object_get(data, "sampler");
									json_t *filter;
									filter = json_object_get(sampler, "filter");
									const char *filter_arg;
									filter_arg  = json_string_value(filter);
// 									SDL_Log("filter: %s", filter_arg);
									
									json_t *wrap;
									wrap = json_object_get(sampler, "wrap");
									const char *wrap_arg;
									wrap_arg  = json_string_value(wrap);
// 									SDL_Log("wrap: %s", wrap_arg);
									
									json_t *vflip;
									vflip = json_object_get(sampler, "vflip");
									const char * vflip_arg;
									vflip_arg=json_string_value(vflip);
// 									SDL_Log("vlfip: %s", vflip_arg);
									
									json_t *srgb;
									srgb = json_object_get(sampler, "srgb");
									const char * srgb_arg;
									srgb_arg=json_string_value(srgb);
// 									SDL_Log("srgb: %s", srgb_arg);
									
									json_t *internal;
									internal = json_object_get(sampler, "internal");
									const char * internal_arg;
									internal_arg=json_string_value(internal);
// 									SDL_Log("internal: %s", internal_arg);
									// use texture
// 									use_ST_tex(channel, texture, flip and so on);
									load_image(last, chan, filter_arg, wrap_arg, vflip_arg, srgb_arg, internal_arg);
								} else
									SDL_Log("Not a supported type!");
								
							}
						}
					}
				}
			}
		}
		free((char*)text);
		json_decref(root);
	}

	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);

	shading_program[3] = glCreateProgram();
	
	glAttachShader(shading_program[3], vtx);
	glAttachShader(shading_program[3], frag);
	
	glLinkProgram(shading_program[3]);
	
	GLuint status;
	status=program_check(shading_program[3]);
	if (status==GL_FALSE)
		return 0;
	return shading_program[3];
	
}

GLuint GetShader(GLenum eShaderType, const char *filename)
{

	const char *shaderSource=read_file(filename);
	GLuint shader = compile_shader(eShaderType, 1, &shaderSource);
	return shader;

}

GLuint compile_shader(GLenum type, GLsizei nsources, const char **sources)
{

	GLuint  shader;
	GLint   success, len;
	GLsizei i, srclens[nsources];
	
	for (i = 0; i < nsources; ++i)
		srclens[i] = (GLsizei)strlen(sources[i]);
	
	shader = glCreateShader(type);
	glShaderSource(shader, nsources, sources, srclens);
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			char *log;
			log = malloc(len);
			glGetShaderInfoLog(shader, len, NULL, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		SDL_Log("Error compiling shader.\n");
	}
	return shader;
}

GLuint default_shaders(GLuint choice)
{
// 	SDL_Log("choice def: %d", choice);
	GLuint vtx;
	vtx = default_vertex();
	if (vtx==0)
		return 0;
	
	GLuint frag;
	const char *sources[4];
	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
	switch(choice)
	{
		case 0:
			sources[2] = default_fragment_shader_0;
			break;
		case 1:
			sources[2] = default_fragment_shader_1;
			break;
		case 2:
			sources[2] = default_fragment_shader;
			break;
		default:
			//some statements to execute when default;
			break;
	}
	
	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);

	shading_program[choice] = glCreateProgram();
	glAttachShader(shading_program[choice], vtx);
	glAttachShader(shading_program[choice], frag);
	glLinkProgram(shading_program[choice]);


	GLuint status;
	status=program_check(shading_program[choice]);
	if (status==GL_FALSE)
		return 0;

	return shading_program[choice];
}


GLuint default_vertex(void)
{
	GLuint vtx;
	const char *sources[2];
	sources[0] = common_shader_header;
	sources[1] = vertex_shader_body;
	vtx = compile_shader(GL_VERTEX_SHADER, 2, sources);
	return vtx;
	
}

GLuint program_check(GLuint program)
{

	GLint status;
	glValidateProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status){
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1){
			char *log;
			log = malloc(len);
			glGetProgramInfoLog(program, len, &len, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		SDL_Log("Error linking shader default program.\n");
		return GL_FALSE;
	}
	return GL_TRUE;
}
//END 	GPU PROGRAM CREATION

void query_vars(GLuint choice)
{
//1 	vec3		iResolution		image/buffer		The viewport resolution (z is pixel aspect ratio, usually 1.0)
//2 	float		iTime			image/sound/buffer	Current time in seconds
//3 	float		iTimeDelta		image/buffer		Time it takes to render a frame, in seconds
//4	int		iFrame			image/buffer		Current frame
//5 	float		iFrameRate		image/buffer		Number of frames rendered per second
//6 	float		iChannelTime[4]		image/buffer		Time for channel (if video or sound), in seconds
//7 	vec3		iChannelResolution[4]	image/buffer/sound	Input texture resolution for each channel
//8 	vec4		iMouse			image/buffer		xy = current pixel coords (if LMB is down). zw = click pixel
//9 	sampler2D	iChannel{i}		image/buffer/sound	Sampler for input textures i
//10 	vec4		iDate			image/buffer/sound	Year, month, day, time in seconds in .xyzw
//11 	float		iSampleRate		image/buffer/sound	The sound sample rate (typically 44100)

// Type: GLint
//1  uniform_res			window resolution
//2  uniform_gtime			overall runtime
//3  uniform_TimeDelta			frame time
//4  uniform_Frame				Current frame int-counter
//5  uniform_FrameRate
//6  uniform_ChannelTime[4]
//7  uniform_iChannelResolution[4]
//8  uniform_mouse			click and origin coordinates
//9  sampler_channel[4]
//10 uniform_date			Wallclock - Year, month, day, time in seconds in xyzw
//11 uniform_srate



// JAVASCRIPT
// 	header += "uniform vec3      iResolution;\n" +		5
// 	"uniform float     iTime;\n" +				1
// 	"uniform float     iChannelTime[4];\n" +		9
// 	"uniform vec4      iMouse;\n" +				4
// 	"uniform vec4      iDate;\n" +				2
// 	"uniform float     iSampleRate;\n" +			8
// 	"uniform vec3      iChannelResolution[4];\n" +		7
// 	"uniform int       iFrame;\n" +				
// 	"uniform float     iTimeDelta;\n" +
// 	"uniform float     iFrameRate;\n";
	
	// In OpenGL is there a way to get a list of all uniforms & attribs used by a shader program?
	// https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade

	GLint i;
	GLint count;
	
	GLint size;
	GLenum type;
	
	const GLsizei charbuffer = 32;
	GLchar name[charbuffer];
	GLsizei length;
	attrib_position=-1;
	glGetProgramiv(shading_program[choice], GL_ACTIVE_ATTRIBUTES, &count);
	for (i = 0; i < count; i++){
		glGetActiveAttrib(shading_program[choice], (GLuint)i, charbuffer, &length, &size, &type, name);
		if (!strcmp("iPosition",name))
			attrib_position = i;
	}

	uniform_gtime			=-1;
	uniform_date			=-1;
	uniform_mouse			=-1;
	sampler_channel[0]		=-1;
	uniform_iChannelResolution[0]	=-1;
	uniform_TimeDelta		=-1;
	uniform_res			=-1;
	
	glGetProgramiv(shading_program[choice], GL_ACTIVE_UNIFORMS, &count);
	for (i = 0; i < count; i++){
		glGetActiveUniform(shading_program[choice], (GLuint)i, charbuffer, &length, &size, &type, name);
// 		SDL_Log("Uniform: %d Type: %u\t Name: %s\n", i, type, name);

		if (!strcmp("iTime",name)||!strcmp("iGlobalTime",name))
			uniform_gtime = i;
		if (!strcmp("iDate",name))
			uniform_date = i;
		if (!strcmp("iMouse",name))
			uniform_mouse = i;
		if (!strcmp("iChannel0",name))
			sampler_channel[0] = i;
		if (!strcmp("iChannelResolution[0]",name))
			uniform_iChannelResolution[0]= i;
		if (!strcmp("iTimeDelta",name))
			uniform_TimeDelta = i;
		if (!strcmp("iResolution",name))
			uniform_res = i;
	}
	// TYPES:
	// 35666 FLOAT_VEC4
	// 35665 FLOAT_VEC3
	// 35678 2D texture
// 	sampler_channel[0]=35678
// 	sampler2D-> 2D texture
// 	this.mRenderer.SetShaderConstant3F(  "iChannel[0].resolution", resos[0], resos[ 1], resos[ 2] );
// 	this.mRenderer.SetShaderConstant3F(  "iChannel[1].resolution", resos[3], resos[ 4], resos[ 5] );
// 	this.mRenderer.SetShaderConstant3F(  "iChannel[2].resolution", resos[6], resos[ 7], resos[ 8] );
// 	this.mRenderer.SetShaderConstant3F(  "iChannel[3].resolution", resos[9], resos[10], resos[11] );
}

//BEGIN MISC
float fTime(void)
{

	static Uint64 start 	 = 0;
	static Uint64 frequency  = 0;
	
	if (start==0){
		start		=	SDL_GetPerformanceCounter();
		frequency	=	SDL_GetPerformanceFrequency();
		return 0.0f;
	}
	Uint64 counter    = 0;
	       counter    = SDL_GetPerformanceCounter();
	Uint64 accumulate = counter - start;
	return   (float)accumulate / (float)frequency;

}

float fTimeDelta(void)
{
	
	static Uint64 frequency  = 0;
	static Uint64 last;
	static Uint64 current = 0;
	
	if (current==0){
		current    	= 	SDL_GetPerformanceCounter();
		frequency	=	SDL_GetPerformanceFrequency();
		return 0.0f;
	}
	last  			=	SDL_GetPerformanceCounter();
	Uint64 dt = last-current;
	current=last;
	return   (float)dt/ (float)frequency;
	
}
void init_glew(void)
{
	GLenum status;
	status = glewInit();

	if (status != GLEW_OK){
		SDL_Log("glewInit error: %s\n", glewGetErrorString (status));
		Running = 0;
	}

/* 	SDL_Log("\nGL_VERSION   : %s\nGL_VENDOR    : %s\nGL_RENDERER  : %s\n"
		 "GLEW_VERSION : %s\nGLSL VERSION : %s\n",
	  glGetString (GL_VERSION), glGetString (GL_VENDOR),
		 glGetString (GL_RENDERER), glewGetString (GLEW_VERSION),
		 glGetString (GL_SHADING_LANGUAGE_VERSION));
		 */

	int maj;
	int min;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,   &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,   &min);
// 	SDL_Log("Using OpenGL %d.%d", maj, min);
	
	if (!GLEW_VERSION_3_0){
		SDL_Log("At least OpenGL 3.0 with GLSL 1.30 required.");
		Running = 0;
	}
}

const char * read_file(const char *filename)
{
	long length = 0;
	char *result = NULL;
	FILE *file = fopen(filename, "r");
	if(file) {
		int status = fseek(file, 0, SEEK_END);
		if(status != 0) {
			fclose(file);
			return NULL;
		}
		length = ftell(file);
		status = fseek(file, 0, SEEK_SET);
		if(status != 0) {
			fclose(file);
			return NULL;
		}
		result = malloc((length+1) * sizeof(char));
		if(result) {
			size_t actual_length = fread(result, sizeof(char), length , file);
			result[actual_length++] = '\0';
		} 
		fclose(file);
		return result;
	}
	SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Couldn't read %s", filename);
	return NULL;
}

int write_STFS(void)
{

	FILE *fp;
	fp = fopen( ""NEW_FILE , "w" );
	fputs(default_fragment_shader_1, fp);
	fclose(fp);
	char cmd[512];
	snprintf(cmd, sizeof(cmd), "chmod -x %s", NEW_FILE);
	system(cmd);
	NEW_SHADER=1;
	system(editor_call);	
	current_program=4;
	if (glIsProgram(shading_program[3]))
		glDeleteProgram(shading_program[3]);
	compile_STFS();
	if (current_programs <4)
		current_programs++;
	runtime=0;
	return(0);

}

struct epoll_event reset_poll(int halt)
{
	
	static struct epoll_event ev;
	if (halt){
		// release epoll
		// NOTE One can't rely on close to clean up epoll subscriptions. 
		// NOTE One always must always explicitly call epoll_ctl(EPOLL_CTL_DEL)
		//      before calling close().
		epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
		close(efd);
		
		// release inotify
		inotify_rm_watch( fd, wd );
		close(wd);
		close(fd);
		return ev;
	}
	
	fd = inotify_init();
	wd = inotify_add_watch(fd, ""NEW_FILE, IN_CLOSE_WRITE);
	ev.events = EPOLLIN|EPOLLOUT|EPOLLONESHOT;
	efd = epoll_create(sizeof(fd));
	epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
	return ev;
}

GLuint compile_STFS(void)
{
	GLuint vtx;
	vtx = default_vertex();
	if (vtx==0)
		return 0;
	GLuint frag;
	const char *sources[4];
	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
	sources[2] = read_file(""NEW_FILE);
	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);
	shading_program[3] = glCreateProgram();
	
	glAttachShader(shading_program[3], vtx);
	glAttachShader(shading_program[3], frag);
	
	glLinkProgram(shading_program[3]);
	
	GLuint status;
	status=program_check(shading_program[3]);
	if (status==GL_FALSE){
		return 0;
	}else{
		glUseProgram(shading_program[3]);
// 		SDL_Log("own_shading_program: %d", shading_program[3]);
		query_vars(3);
		glEnableVertexAttribArray	(attrib_position);
		glVertexAttribPointer		(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glViewport (0, 0, ww, wh);
		glUniform3f(uniform_res, ww, wh, 0.0f);
		return 1;
	}

return 0;

}

void shader_switch(void)
{
	current_program++;
// 	SDL_Log("current_program: %d", current_program);
	if (current_program>current_programs){
		current_program=1;
// 		SDL_Log("adjust current_program: %d", current_program);
	}
	glUseProgram(shading_program[current_program-1]);
	query_vars(current_program-1);
	glEnableVertexAttribArray	(attrib_position);
	glVertexAttribPointer		(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);

	glViewport (0, 0, ww, wh);
	glUniform3f(uniform_res, ww, wh, 0.0f);
}

void load_image(char *filename, int channel, const char *filter, const char *wrap, const char *vflip, const char *srgb, const char *bpp)
{
	// set path
	char fullpath[256];
	snprintf(fullpath, sizeof(fullpath), "%s%s",TEXTURES,filename);
// 	SDL_Log("%s",fullpath);
	
	SDL_Surface* surface = IMG_Load(fullpath);
	//FIXME
	// effect.js
	// texID[i] = inp.globject;
	// resos[3*i+0] = inp.image.width;
	// resos[3*i+1] = inp.image.height;
	// resos[3*i+2] = 1;
	resos[0][0]=surface->w;
	resos[0][1]=surface->h;
	resos[0][2]=1;
// 	SDL_Log("foo: %f", resos[0][0]);
// 	SDL_Log("foo: %f", resos[0][1]);
// 	SDL_Log("foo: %f", resos[0][2]);
	surface=flip_horizontal(surface);
	
	// VFLIP
	if (!strcmp("true",vflip)){
// 		SDL_Log("want: VFLIP");
	// 	surface = flip_vertical(surface);
		// dunno, looks like vflip doesn't work in ST
	} else
// 		SDL_Log("want: NO VFLIP");
	// BPP
	(void)bpp;
	// SDL_Log("BytesPerPixel: %d",surface->format->BytesPerPixel);
	(void)srgb;

	glEnable(GL_TEXTURE_2D);
	
	// Transparency
	// 	glEnable(GL_BLEND);
	// 	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	(void)channel;
// 	glGenTextures(1, &texture[channel]);
// 	glBindTexture(GL_TEXTURE_2D, texture[channel]);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
// 	glBindSampler(texture​, sampler_channel[0]​);
	// Alignment
	// 	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//BEGIN FILTERING
	if (!strcmp("nearest",filter)){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
// 		SDL_Log("want: nearest");
	}
	if (!strcmp("linear",filter)){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
// 		SDL_Log("want: linear");
	}
	if (!strcmp("mipmap",filter)){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
// 		glGenerateMipmap(GL_TEXTURE_2D);
// 		SDL_Log("want: mipmap");
	}
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//END FILTERING
	
	
	//BEGIN WRAPPING / Texture Layout
	if (!strcmp("clamp",wrap)){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
// 		SDL_Log("want: clamp");
	}
	if (!strcmp("repeat",wrap)){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 		SDL_Log("want: repeat");
	}
	//END 	WRAPPING / Texture Layout

	glTexImage2D	(GL_TEXTURE_2D, 	// target
			 0, 			// level, 0 = base, no minimap,
	       GL_RGB, 				// internalformat
	       surface->w, 			// width
	       surface->h, 			// height
	       0, 				// This value must be 0.
	       GL_RGB,				// format
	       GL_UNSIGNED_BYTE, 		// data type of the pixel datatype
	       surface->pixels);
	
	SDL_FreeSurface(surface);
	
}

char *request_ST_json(const char *ID)
{
	char buffer[128];
	CURL *curl 			= NULL;
	CURLcode status;
	struct curl_slist *headers 	= NULL;
	char *data 			= NULL;
	long code;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl)
		goto error;
	
	data = malloc(BUFFER_SIZE);
	if(!data)
		goto error;
	
	struct write_result write_result = {
		.data = data,
		.pos = 0
	};
	
	curl_easy_setopt(curl, CURLOPT_URL, "https://www.shadertoy.com/shadertoy");
	
	headers = curl_slist_append(headers, "Referer: https://www.shadertoy.com/");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
	// set postfields
	snprintf(buffer, sizeof(buffer), "s={\"shaders\":+[\"%s\"]}", ID);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS	, buffer);
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION	, write_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA	, &write_result);
	
	status = curl_easy_perform(curl);
	if(status != 0){
		fprintf(stderr, "error: unable to request data:\n");
		fprintf(stderr, "%s\n", curl_easy_strerror(status));
		goto error;
	}
	
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200){
		fprintf(stderr, "error: server responded with code %ld\n", code);
		goto error;
	}
	
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();
	
	/* zero-terminate the result */
	data[write_result.pos] = '\0';
	
	return data;
	
	error:
	if(data)
		free(data);
	if(curl)
		curl_easy_cleanup(curl);
	if(headers)
		curl_slist_free_all(headers);
	curl_global_cleanup();
	return NULL;
}

size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct write_result *result = (struct write_result *)stream;
	
	if(result->pos + size * nmemb >= BUFFER_SIZE - 1){
		fprintf(stderr, "error: too small buffer\n");
		return 0;
	}
	
	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;
	
	return size * nmemb;
}

int save_image( char *url, char *filename)
{
	// set path
	char fullpath[256];
	snprintf(fullpath, sizeof(fullpath), "%s%s",TEXTURES,filename);
// 	SDL_Log("%s",fullpath);
	
	// check if file exists:
	if( access( fullpath, F_OK ) != -1 ) {
// 		SDL_Log("Texture exists, nothing to do!\n");
		return 0;
	} else {
		// SDL_Log("Texture does not exist, trying to download...\n");
		CURL *image; 
		image = curl_easy_init();
		
		if( image ){ 
			// Open file 
			FILE *fp;
			fp = fopen(fullpath, "wb");
			
			// set file non executable, one probably won't need this
			char cmd[512];
			snprintf(cmd, sizeof(cmd), "chmod -x %s", fullpath);
			system(cmd);
			
			if( fp == NULL )
				printf("no Texture for you");
			
			CURLcode imgresult;
			curl_easy_setopt(image, CURLOPT_URL, url);
			curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL); 
			curl_easy_setopt(image, CURLOPT_WRITEDATA, fp); 
			
			// Grab image 
			imgresult = curl_easy_perform(image); 
			if( imgresult ){ 
				printf("still no Texture for you"); 
			}
			
			curl_easy_cleanup(image); 
			fclose(fp); 
			return 0;
		}
		
	}
	
	return 1; 
}

SDL_Surface * flip_vertical(SDL_Surface *sfc)
{
	SDL_Surface *result= SDL_CreateRGBSurfaceWithFormat(0,sfc->w,sfc->h, sfc->format->BytesPerPixel, sfc->format->format);
	
	int pitch 	= sfc->pitch;
	void *pixels 	= sfc->pixels;
	void *rpixels 	= result->pixels+pitch;
	
	size_t lenght = sfc->format->BytesPerPixel;
	
	for(int line = 0; line < sfc->h; ++line){
		for (int col = 0; col < sfc->w; col++){
			memcpy(rpixels,pixels,lenght);
			pixels+=lenght;
			rpixels-=lenght;
		}
		rpixels+=2*pitch;
	}
	return result;
	
}

SDL_Surface * flip_horizontal(SDL_Surface* sfc)
{
	SDL_Surface *result= SDL_CreateRGBSurfaceWithFormat(0,sfc->w,sfc->h, sfc->format->BytesPerPixel, sfc->format->format);
	size_t pitch 	= sfc->pitch;
	int pxlength 	= pitch*sfc->h;
	
	void *pixels 	= sfc->pixels + pxlength;	// last pixel
	void *rpixels 	= result->pixels;		// first pixel	
	
	for(int line 	= 0; line < sfc->h; ++line){
		memcpy(rpixels,pixels,pitch);		// copy the line
		pixels -= pitch;			// count down
		rpixels += pitch;			// count up
	}
	return result;
	
}

void save_screenshot(int overwrite)
{
	// set path
	char fullpath[256];
	snprintf(fullpath, sizeof(fullpath), "%s%s.jpg",ST,ST_shader_ID);
// 	SDL_Log("%s",fullpath);
	
	if (overwrite==0){
		// check if file exists:
		if( access( fullpath, F_OK ) != -1 ) {
// 			printf("SS exists, nothing to do!\n");
			return;
		}
	}
	unsigned char *pixels;
	pixels = malloc(ww*wh*3);
	glReadPixels(0,0, ww, wh, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	Uint32 fmt=SDL_PIXELFORMAT_RGB24;
	SDL_Surface *source = SDL_CreateRGBSurfaceWithFormatFrom(pixels, ww, wh,24,ww*3,fmt);
	source=flip_horizontal(source);
	
	// scale, so screenshots have the same size
	SDL_Surface *target = SDL_CreateRGBSurfaceWithFormat(0,500, 281,24,fmt);
	
	// https://wiki.libsdl.org/SDL_BlitScaled
	SDL_BlitScaled(source, NULL,target, NULL);

// 	printf("SS does not exist, trying to save...\n");
	// 	if (!IMG_SavePNG(target, "out.png"))
	// 		SDL_Log("written");
	if (!IMG_SaveJPG(target, fullpath,90)){
// 		SDL_Log("screenshot written");
		// set file non executable, one probably won't need this
		char cmd[512];
		snprintf(cmd, sizeof(cmd), "chmod -x %s", fullpath);
		system(cmd);
	}

}

int handle_dropfile(char *path)
{
	// get shader shorthash
	if (path==NULL)
		return 1;
// 	SDL_Log("%s",path);
	char *shorthash=get_shader_shorthash(path);
// 	SDL_Log("%s",shorthash);
	char *token = strtok(shorthash, ".");
	char *id;
	id=token;
// 	SDL_Log("%s",id);
	glUseProgram(0);
	glUseProgram(0);
	glDisableVertexAttribArray	(attrib_position);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	fonsSetSize	(fs, 38.0f);
	fonsSetColor	(fs, white);
	fonsSetBlur	(fs, 0);
	fonsDrawText	(fs, dx-20,dy,"Hang on... compiling.", NULL);
	SDL_GL_SwapWindow(Window);
	if (glIsProgram(shading_program[3]))
		glDeleteProgram(shading_program[3]);
	shading_program[3] = STFS(id);
	
	// 				SDL_Log("Using shading program %d\n", shading_program[3]);
	query_vars(3);
	// 				STFS_vars(3);
	glUseProgram(shading_program[3]);
	if (current_programs <4)
		current_programs++;
	runtime=0;
	current_program=4;
	make_screenshot=1;
	return 0;
}

int handle_droptext(char *url)
{
	// get shader shorthash
	char *shorthash=get_shader_shorthash(url);
	if (shorthash==NULL)
		return 1;
// 	SDL_Log("%s",shorthash);
	glUseProgram(0);
	glDisableVertexAttribArray	(attrib_position);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	fonsSetSize	(fs, 38.0f);
	fonsSetColor	(fs, white);
	fonsSetBlur	(fs, 0);
	fonsDrawText	(fs, dx-20,dy,"Hang on... downloading data.", NULL);
	SDL_GL_SwapWindow(Window);
	if (glIsProgram(shading_program[3]))
		glDeleteProgram(shading_program[3]);
	shading_program[3] = STFS(shorthash);
	
	// 				SDL_Log("Using shading program %d\n", shading_program[3]);
	query_vars(3);
	// 				STFS_vars(3);
	glUseProgram(shading_program[3]);
	if (current_programs <4)
		current_programs++;
	runtime=0;
	current_program=4;
	make_screenshot=1;
	return 0;
}
char * get_shader_shorthash(char *url)
{
	
	char *token = strtok(url, "/");
	char *shorthash;
	
	while (token != NULL){
		shorthash=token;
		token = strtok(NULL, "/");
	}
	
	if (shorthash == NULL)
		return NULL;
	
	return shorthash;
	
}

//END 	MISC

//END FUNCTIONS
