//BEGIN HEAD
//BEGIN DESCRIPTION

/* DEFINED PROGRESS GOALS
 * Render Quad with shader
 */
//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
// #define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glext.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

//local headers
#include "helper.h"
#include "config.h"
#include "shaders.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define GL_WHITE 	1,1,1,1
#define GL_GREY 	.5, .5, .5, 1
#define WW 		550
#define WH 		(WW/16)*12
#define POSIX_C_SOURCE 200112L
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww=WW;
int wh=WH;

//BEGIN GL
static GLsizei viewport_width = -1;
static GLsizei viewport_height = -1;
static GLuint  shader_program;
static GLint   attrib_position;
static GLint   sampler_channel[4];
static GLint   uniform_cres;
static GLint   uniform_ctime;
static GLint   uniform_date;
static GLint   uniform_gtime;
static GLint   uniform_mouse;
static GLint   uniform_res;
static GLint   uniform_srate;
//END GL

//BEGIN VISIBLES
//END 	VISIBLES

//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
static char* read_file_into_str(const char *);
static void render(float);
static void info(const char *, ...);
static void die (const char *, ...);
static double timespec_diff(const struct timespec *start, const struct timespec *stop);
static void monotonic_time(struct timespec *tp);

static GLuint compile_shader(GLenum type, GLsizei nsources, const char **sources);
static void resize_viewport(GLsizei w, GLsizei h);
static void make_shader(void);
//END	FUNCTION PROTOTYPES

//END 	HEAD

//BEGIN MAIN FUNCTION
int main(int argc, char *argv[])
{
init();
SDL_GL_SetSwapInterval(1);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
// SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 	8);
// SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 	8);
// SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 	8);
int temp_width = 0;
int temp_height = 0;

//shader program
char *program_source = NULL;

//Parse command line selected_options
int selected_option = -1;
int selected_index = 0;

while((selected_option = getopt_long (argc, argv, options_string, long_options, &selected_index)) != -1)
{
	switch(selected_option) {
		case 'f':
			;
			break;
		case 'w':
			temp_width = atoi(optarg);
			if(temp_width > 0) {
				ww = temp_width;
			}
			break;
		case 'h':
			temp_height = atoi(optarg);
			if(temp_height > 0) {
				wh = temp_height;
			}
			break;
		case 's':
			info("Loading shader program: %s\n", optarg);
			program_source = read_file_into_str(optarg);
			if(program_source == NULL) {
				die("Could not read shader program %s\n", optarg);
			}
			default_fragment_shader = program_source;
			break;
		case '?':
			info(   "\nUsage: esshader [OPTIONS]\n"
			"Example: esshader --width 1280 --height 720\n\n"
			"Options:\n"
			" -f, --fullscreen \truns the program in (fake) fullscreen mode.\n"
			" -?, --help \t\tshows this help.\n"
			" -w, --width [value] \tsets the window width to [value].\n"
			" -h, --height [value] \tsets the window height to [value].\n"
			" -s, --source [path] \tpath to shader program\n"
			);
			return 0;
	}
}

info("Press [ESC] or [q] to exit.\n");
info("Run with --help flag for more information.\n\n");

//BEGIN INIT
// SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

//BEGIN WINDOW
SDL_SetWindowPosition(Window,0,0);
SDL_SetWindowSize(Window,ww,wh);
SDL_SetWindowTitle(Window, "Draw Texture");
SDL_ShowWindow(Window);
//END WINDOW
glewExperimental = GL_TRUE;
glewInit();

// glClearColor(GL_GREY);
// glClear(GL_COLOR_BUFFER_BIT);
// glEnable (GL_DEPTH_TEST); /* enable depth-testing */
// glDepthFunc (GL_LESS);/*depth-testing interprets a smaller value as "closer"*/


// const GLubyte* renderer;
// const GLubyte* version;
// renderer = glGetString (GL_RENDERER); /* get renderer string */
// version = glGetString (GL_VERSION); /* version as a string */
// printf ("Renderer: %s\n", renderer);
// printf ("OpenGL version supported %s\n", version);
// 
// int maj;
// int min;
// 
// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,   &maj);
// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,   &min);
// SDL_Log("Maj: %d Min: %d", maj, min);

glEnable (GL_DEPTH_TEST); /* enable depth-testing */
glDepthFunc (GL_LESS);/*depth-testing interprets a smaller value as "closer"*/
resize_viewport(ww,wh);
// make_shader();
SDL_Event event;
int running = 1;
struct timespec start, cur;
monotonic_time(&start);

//TEST
GLuint vao;
GLuint vbo;

GLfloat points[] = {
	0.0f,	0.5f,	0.0f,
	0.5f, -0.5f,	0.0f,
	-0.5f, -0.5f,	0.0f
};

static const GLfloat vertices[] = {
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f,
};
glGenBuffers (1, &vbo);
glBindBuffer (GL_ARRAY_BUFFER, vbo);
// glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);
glBufferData (GL_ARRAY_BUFFER, 8 * sizeof (GLfloat), vertices, GL_STATIC_DRAW);
glGenVertexArrays (1, &vao);
glBindVertexArray (vao);
glEnableVertexAttribArray (0);
glBindBuffer (GL_ARRAY_BUFFER, vbo);
// glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
glBindVertexArray (vao);


/* these are the strings of code for the shaders
 the* vertex shader positions each vertex point */
const char* vertex_shader =
"void main()"
"{"
"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
"gl_FrontColor = gl_Color;"
"}";
// 
// const char* fragment_shader =
// "#version 130\n"
// "out vec4 frag_colour;"
// "void main () {"
// "	frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
// "}";

const char* fragment_shader =
"uniform vec4 color;"
"uniform float expand;"
"uniform vec2 center;"
"uniform float radius;"
"uniform float windowHeight;"
"void main(void)"
"{"
"vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
"vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;"
"float r = sqrt(dot(p, p));"
"if (r < 1.0)"
"{"
"gl_FragColor = mix(color, gl_Color, (r - expand) / (1 - expand));"
"}"
"else"
"{"
"gl_FragColor = gl_Color;"
"}"
"}";

GLuint vs, fs;
GLuint shader_programme;
vs = glCreateShader (GL_VERTEX_SHADER);
glShaderSource (vs, 1, &vertex_shader, NULL);
glCompileShader (vs);
fs = glCreateShader (GL_FRAGMENT_SHADER);
glShaderSource (fs, 1, &fragment_shader, NULL);
glCompileShader (fs);
shader_programme = glCreateProgram ();
glAttachShader (shader_programme, fs);
glAttachShader (shader_programme, vs);
glLinkProgram (shader_programme);
glUseProgram (shader_programme);



//END   INIT

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
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render((float)timespec_diff(&start, &cur));
	monotonic_time(&cur);
// 	glDrawArrays (GL_TRIANGLES, 0, 3);
// 	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
// 	SDL_GL_SwapWindow(Window);
	//END   RENDERING
}
//END   MAIN LOOP
glDeleteProgram(shader_program);
if(program_source != NULL) {
	free(program_source);
	program_source = NULL;
}
exit_();
return EXIT_SUCCESS;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS

//Reads a file into a string
//Return string or NULL on failure
static char* read_file_into_str(const char *filename)
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
	return NULL;
}

static void render(float abstime)
{
	if(uniform_gtime >= 0)
		glUniform1f(uniform_gtime, abstime);
	
	glClearColor(0.5f, 0.0f, 0.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	SDL_GL_SwapWindow(Window);

}

static void info(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}

static void die(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	
	exit(EXIT_FAILURE);
}

static double timespec_diff(const struct timespec *start, const struct timespec *stop)
{
	struct timespec d;
	if ((stop->tv_nsec - start->tv_nsec) < 0){
		d.tv_sec = stop->tv_sec - start->tv_sec - 1;
		d.tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000l;
	}
	else {
		d.tv_sec = stop->tv_sec - start->tv_sec;
		d.tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
	
	return (double)d.tv_sec + (double)d.tv_nsec / 1000000000.0;
}

static void monotonic_time(struct timespec *tp)
{
	if (clock_gettime(CLOCK_MONOTONIC, tp) == -1)
		die("clock_gettime on CLOCK_MONOTIC failed.\n");
}

static GLuint compile_shader(GLenum type, GLsizei nsources, const char **sources)
{
	SDL_Log("Hello from compile shader");
	GLuint shader;
	GLint success, len;
	GLsizei i, srclens[nsources];
	char *log;
	
	for (i = 0; i < nsources; ++i)
		srclens[i] = (GLsizei)strlen(sources[i]);
	
	shader = glCreateShader(type);
	glShaderSource(shader, nsources, sources, srclens);
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			log = malloc(len);
			glGetShaderInfoLog(shader, len, NULL, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		die("Error compiling shader.\n");
	}
	
	return shader;
}

static void resize_viewport(GLsizei w, GLsizei h)
{
	if (viewport_width != w || viewport_height != h) {
		glUniform3f(uniform_res, (float)w, (float)h, 0.0f);
		glViewport(0, 0, w, h);
		viewport_width = w;
		viewport_height = h;
		info("Setting window size to (%d,%d).\n", w, h);
	}
}

static void make_shader(void)
{
	SDL_Log("Hello from make shader");
	GLint len, success;
	GLuint vtx, frag;
	const char *sources[4];
	char* log;
	
	sources[0] = common_shader_header;
	sources[1] = vertex_shader_body;
	vtx = compile_shader(GL_VERTEX_SHADER, 2, sources);
	
	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
	sources[2] = default_fragment_shader;
	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);
	
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vtx);
	glAttachShader(shader_program, frag);
	
	glLinkProgram(shader_program);
	
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			log = malloc(len);
			glGetProgramInfoLog(shader_program, len, &len, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		die("Error linking shader program.\n");
	}else
		SDL_Log("Shader linked sucessfully!");
	
// 	glDetachShader(shader_program, vtx);
// 	glDetachShader(shader_program, frag);
	glDeleteShader(vtx);
	glDeleteShader(frag);
	glReleaseShaderCompiler();
	
	glUseProgram(shader_program);
	glValidateProgram(shader_program);
	
	attrib_position = glGetAttribLocation(shader_program, "iPosition");
	sampler_channel[0] = glGetUniformLocation(shader_program, "iChannel0");
	sampler_channel[1] = glGetUniformLocation(shader_program, "iChannel1");
	sampler_channel[2] = glGetUniformLocation(shader_program, "iChannel2");
	sampler_channel[3] = glGetUniformLocation(shader_program, "iChannel3");
	uniform_cres = glGetUniformLocation(shader_program, "iChannelResolution");
	uniform_ctime = glGetUniformLocation(shader_program, "iChannelTime");
	uniform_date = glGetUniformLocation(shader_program, "iDate");
	uniform_gtime = glGetUniformLocation(shader_program, "iGlobalTime");
	uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
	uniform_res = glGetUniformLocation(shader_program, "iResolution");
	uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");
	
}
//END   FUNCTIONS
