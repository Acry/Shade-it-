//BEGIN HEAD
//BEGIN DESCRIPTION

/* DEFINED PROGRESS GOALS
 * Render Quad with shader
 * What we want is OpenGL 2.0
 * Which is around since September 2004
 * This is when GLSL was introduced.
 * Pretty aged, but that is where I start.
 */
//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
#include <math.h>
//local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define GL_WHITE 	1,1,1,1
#define GL_GREY 	.5, .5, .5, 1
#define WW 		550
#define WH 		(WW/16)*12
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww=WW;
int wh=WH;
//BEGIN VISIBLES
//END 	VISIBLES

//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
char *read_file_into_str(char *);

//END	FUNCTION PROTOTYPES

//END 	HEAD

//BEGIN MAIN FUNCTION
int main(int argc, char *argv[])
{

(void)argc;
(void)argv;

//BEGIN INIT
// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
// SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); the default
SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

init();

//BEGIN WINDOW
SDL_SetWindowPosition(Window,0,0);
SDL_SetWindowSize(Window,ww,wh);
SDL_SetWindowTitle(Window, "Draw Texture");
SDL_ShowWindow(Window);
//END WINDOW


glClearColor(GL_GREY);
glClear(GL_COLOR_BUFFER_BIT);
glEnable (GL_DEPTH_TEST); /* enable depth-testing */
glDepthFunc (GL_LESS);/*depth-testing interprets a smaller value as "closer"*/

const GLubyte* renderer;
const GLubyte* version;
renderer = glGetString (GL_RENDERER); /* get renderer string */
version = glGetString (GL_VERSION); /* version as a string */
printf ("Renderer: %s\n", renderer);
printf ("OpenGL version supported %s\n", version);


int maj;
int min;

SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,   &maj);
SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,   &min);

SDL_Log("Maj: %d Min: %d", maj, min);
glViewport (0, 0, ww, wh);
SDL_Event event;
int running = 1;
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
	glDrawArrays(GL_TRIANGLES, 0, 3);

	SDL_GL_SwapWindow(Window);
	//END   RENDERING
}
//END   MAIN LOOP
exit_();
return EXIT_SUCCESS;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS
char * read_file_into_str( char *filename)
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

GLuint BuildShaderProgram(const char *vsPath, const char *fsPath)
{
	
	GLuint vertexShader;
	GLuint fragmentShader;
	
	vertexShader = CreateShader(GL_VERTEX_SHADER, vsPath);
	fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fsPath);
	
	GLuint tempProgram;
	tempProgram = glCreateProgram();
	
	glAttachShader(tempProgram, vertexShader);
	glAttachShader(tempProgram, fragmentShader);
	
	glLinkProgram(tempProgram);
	
	
	GLint status;
	glGetProgramiv(tempProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)	{
		GLint infoLogLength;
		glGetProgramiv(tempProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		GLchar strInfoLog[4096];
		glGetProgramInfoLog(tempProgram, infoLogLength, NULL, strInfoLog);
		printf("Shader linker failure: %s\n", strInfoLog);
		return -1;
	}
	else
		puts("Shader linked sucessfully!");
	
	glDetachShader(tempProgram, vertexShader);
	glDetachShader(tempProgram, fragmentShader);
	
	return tempProgram;
}

GLuint CreateShader(GLenum eShaderType, const char *strShaderFile)
{
	char shaderSource[4096];
	char inChar;
	FILE *shaderFile;
	int i = 0;
	
	shaderFile = fopen(strShaderFile, "r");
	while(fscanf(shaderFile,"%c",&inChar) > 0)
	{
		shaderSource[i++] = inChar; //loading the file's chars into array
	}
	shaderSource[i - 1] = '\0';
	fclose(shaderFile);
	puts(shaderSource); //print to make sure the string is loaded
	
	GLuint shader = glCreateShader(eShaderType);
	const char *ss = shaderSource;
	glShaderSource(shader, 1, &ss, NULL);
	
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		GLchar strInfoLog[4096];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		
		char strShaderType[16];
		switch(eShaderType)
		{
			case GL_VERTEX_SHADER: sprintf(strShaderType, "vertex"); break;
			case GL_GEOMETRY_SHADER: sprintf(strShaderType, "geometry"); break;
			case GL_FRAGMENT_SHADER: sprintf(strShaderType, "fragment"); break;
		}
		
		printf("Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		return -1;
	}
	else
		puts("Shader compiled sucessfully!");
	
	return shader;
}
//END   FUNCTIONS
