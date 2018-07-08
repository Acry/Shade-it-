//BEGIN HEAD
//BEGIN DESCRIPTION
//END 	DESCRIPTION

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // Just for the icon - easy to strip out
#include "default_shaders.h"
#define FRAG	"assets/shader/frag/5.frag"
#define VERT	"assets/shader/vert/2.vert"

//BEGIN GLOBALS
int ww=800;
int wh=600;

GLuint shading_program;
char Running = 1;
char switch_counter=0;

GLint attrib_position;
GLint sampler_channel[4];
GLint uniform_cres;
GLint uniform_ctime;
GLint uniform_date;
GLint uniform_gtime;
GLint uniform_mouse;
GLint uniform_res;
GLint uniform_srate;
//END  GLOBALS


GLuint BuildProgram		(const char *	, const char *);
GLuint GetShader		(GLenum 	, const char *);
GLuint default_shaders		(void);
GLuint default_vertex		(void);
GLuint compile_shader		(GLenum type, GLsizei , const char **);
GLuint program_check		(GLuint);
void   query_shader_vars	(void);

float  fTime			(void);
void   init_glew		(void);
const char  *read_file		(const char *);
//END 	HEAD

//BEGIN MAIN
int main(int argc, char *argv[])
{
	//BEGIN INIT
	(void)argv; // not needed yet

	char argument_provided=0;
	if (argc==2){
		argument_provided=1;
	} else {
		SDL_Log("No argument provided.");
	}
	//BEGIN INIT SDL2
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *Window = SDL_CreateWindow("0e - TESTING",
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

	SDL_GLContext glContext = SDL_GL_CreateContext(Window);
	//END 	INIT SDL2

	init_glew();

	//BEGIN SHADER INIT

	if (argument_provided == 2){
		shading_program = BuildProgram(VERT, FRAG);
		SDL_Log("Trying to build custom shader");
	} else {
		shading_program = default_shaders();
		SDL_Log("Trying to build default shader");
	}

	//Test if GPU-Programm is available
	if (shading_program == 0){
		Running = 0;
		if (glGetError()!=0)
			SDL_Log("glError: %#08x\n", glGetError());
	} else {
		glUseProgram(shading_program);
		SDL_Log("Using shading program %d\n", shading_program);
	}
	
	//END 	SHADER INIT
	GLfloat vertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f,
	};
	if (argument_provided<2){
		query_shader_vars();
		glEnableVertexAttribArray(attrib_position);
		glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		SDL_Log("Using default shader");
	}
	glUniform3f(uniform_res, (float)ww, (float)wh, 0.0f);
	SDL_Event event;
	//END	INIT

	//BEGIN MAINLOOP
	while (Running){
		//BEGIN EVENTS
		while ( SDL_PollEvent(&event) ){
			if (event.type == SDL_QUIT)
				Running = 0;
			if(event.type == SDL_WINDOWEVENT){
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					glViewport (0, 0, event.window.data1, event.window.data2);
					glUniform3f(uniform_res, (float)event.window.data1, (float)event.window.data2, 0.0f);
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
					;
				}
				if(event.button.button==SDL_BUTTON_LEFT){
					switch_counter++;
					if (switch_counter>1)
						switch_counter=0;
				}
			}
			if(event.type == SDL_KEYDOWN ){
				switch(event.key.keysym.sym ){
					case SDLK_ESCAPE:
						Running = 0;
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
		//END 	EVENTS

		//BEGIN UPDATE
		switch(switch_counter)
		{
			case 1:
				//some statements to execute when 1
				break;
			case 2:
				//some statements to execute when 2
				break;
			case 3:
				//some statements to execute when 3
				break;
			default:
				//some statements to execute when default;
				break;
		}
		//END UPDATE
		//BEGIN RENDERING
		glClear(GL_COLOR_BUFFER_BIT);
		if (argument_provided<2)
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		else
			glRectf(-1.0, -1.0, 1.0, 1.0);
		glUniform1f(uniform_gtime, fTime());
		SDL_GL_SwapWindow(Window);
		//END 	RENDERING
	}
	//END 	MAINLOOP

	//BEGIN EXIT
	if (glIsProgram(shading_program))
		glDeleteProgram(shading_program);

	SDL_GL_DeleteContext(glContext);
	SDL_Quit();

	return EXIT_SUCCESS;
	//END 	EXIT
}
//END	MAIN

//BEGIN FUNCTIONS

//BEGIN GPU PROGRAM CREATION
GLuint BuildProgram(const char *vsPath, const char *fsPath)
{
	GLuint vertexShader;
	GLuint fragmentShader;

	vertexShader   = GetShader(GL_VERTEX_SHADER,   vsPath);
	fragmentShader = GetShader(GL_FRAGMENT_SHADER, fsPath);

	shading_program = glCreateProgram();

	glAttachShader(shading_program, vertexShader);
	glAttachShader(shading_program, fragmentShader);

	glLinkProgram(shading_program);

	//Error Checking
	GLuint status;
	status=program_check(shading_program);
	if (status==GL_FALSE)
		return 0;
	return shading_program;

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
	SDL_Log("shader: %u",shader);
	return shader;
}

//if no arguments provided use vertex and fragment shader from header
GLuint default_shaders(void)
{

	GLuint vtx;
	
// 	sources[0] = common_shader_header;
// 	sources[1] = vertex_shader_body;
// 	vtx = compile_shader(GL_VERTEX_SHADER, 2, sources);
	vtx = default_vertex();
	if (vtx==0)
		return 0;
	
	GLuint frag;
	const char *sources[4];
	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
	sources[2] = default_fragment_shader;
	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);

	shading_program = glCreateProgram();
	glAttachShader(shading_program, vtx);
	glAttachShader(shading_program, frag);
	glLinkProgram(shading_program);

	//Error Checking
	GLuint status;
	status=program_check(shading_program);
	if (status==GL_FALSE)
		return 0;

	return shading_program;
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
	//Error Checking
	GLint status;
	glValidateProgram(program);
	glGetProgramiv(shading_program, GL_LINK_STATUS, &status);
	if (!status){
		GLint len;
		glGetProgramiv(shading_program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1){
			char *log;
			log = malloc(len);
			glGetProgramInfoLog(shading_program, len, &len, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		SDL_Log("Error linking shader default program.\n");
		return GL_FALSE;
	}
	return GL_TRUE;
}
//END 	GPU PROGRAM CREATION

void query_shader_vars(void)
{
	attrib_position = glGetAttribLocation(shading_program, "iPosition");
	
	sampler_channel[0] = glGetUniformLocation(shading_program, "iChannel0");
	sampler_channel[1] = glGetUniformLocation(shading_program, "iChannel1");
	sampler_channel[2] = glGetUniformLocation(shading_program, "iChannel2");
	sampler_channel[3] = glGetUniformLocation(shading_program, "iChannel3");
	
	uniform_cres  = glGetUniformLocation(shading_program, "iChannelResolution");
	uniform_ctime = glGetUniformLocation(shading_program, "iChannelTime");
	uniform_date  = glGetUniformLocation(shading_program, "iDate");
	uniform_gtime = glGetUniformLocation(shading_program, "iGlobalTime");
	uniform_mouse = glGetUniformLocation(shading_program, "iMouse");
	uniform_res   = glGetUniformLocation(shading_program, "iResolution");
	uniform_srate = glGetUniformLocation(shading_program, "iSampleRate");
}

//BEGIN MISC
//Running Time in float
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

void init_glew(void)
{
	GLenum status;
	status = glewInit();

	if (status != GLEW_OK){
		SDL_Log("glewInit error: %s\n", glewGetErrorString (status));
		Running = 0;
	}

	SDL_Log("\nGL_VERSION   : %s\nGL_VENDOR    : %s\nGL_RENDERER  : %s\n"
		 "GLEW_VERSION : %s\nGLSL VERSION : %s\n",
	  glGetString (GL_VERSION), glGetString (GL_VENDOR),
		 glGetString (GL_RENDERER), glewGetString (GLEW_VERSION),
		 glGetString (GL_SHADING_LANGUAGE_VERSION));

	int maj;
	int min;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,   &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,   &min);
	SDL_Log("Using OpenGL %d.%d", maj, min);

	if (!GLEW_VERSION_2_0){
		SDL_Log("At least OpenGL 2.0 with GLSL 1.10 required.");
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
//END 	MISC

//END FUNCTIONS
