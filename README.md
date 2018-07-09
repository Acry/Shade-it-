### Shade it! - Shadertoy viewer/editor

Start with shader hash to autodownload json and assets  
`./shade_it 4dKfDV`  
**or** Drag and Drop from Shadertoy.com **or** Drag and Drop json from assets/glsl/st/  
Stores json and screenshots in assets folder.  

Middle mouse button to switch shader.  
Press n to create new shader, will open in Editor  
If file is changed and saved it should autocompile, press c to do that manually.  
Adjust your editor before compiling:  
`#define EDITOR	 "/usr/bin/kate"`  
P to toggle rendering/pause.  
S to override screenshot.  
ESC to quit.  

Screenshots:  
![Screenshot](shots/shot_1.png)  

**Dependencies:**  
[SDL2](https://www.libsdl.org/)/[SDL2 Image](https://www.libsdl.org/projects/SDL_image/)/[glew](https://github.com/nigels-com/glew)/[curl](https://curl.haxx.se/libcurl/)/[jansson](http://www.digip.org/jansson/)/[fontstash](https://github.com/memononen/fontstash) 

[Get in touch on SDL-Discourse](https://discourse.libsdl.org/u/Acry/summary)

**More useful links:**  
[SDL2](https://www.libsdl.org/) | [SDL-Discourse](https://discourse.libsdl.org) | [SDL2-Example Code](https://gist.github.com/Acry/baa861b8e370c6eddbb18519c487d9d8) | [N. Nopper-Code](https://github.com/McNopper/OpenGL) | [McKesson-Book](https://paroj.github.io/gltut/) | [Íñigo Quílez](http://www.iquilezles.org/prods/index.htm)  |  [shadertoy](https://www.shadertoy.com/)

**Building/Development-Details:**  
SDL2 Image is very easy to replace, like with [Stb_image](https://github.com/nothings/stb/blob/master/stb_image.h).  

For a cross-platform build:  
I don't really aim for a multi-platform build, but I am not against that either.  
Due the lack of time/money (development ressources) I don't do that. Feel free to commit/fork or raise an issue. 

Since [inotify](http://man7.org/linux/man-pages/man7/inotify.7.html) and [epoll](http://man7.org/linux/man-pages/man7/epoll.7.html) are used for the autocompile feature, you either need to get rid of the feature or look for a proper substitution.  
I think libcurl and jansson are pretty much portable, some POSIX-features are used and I call system with chmod to set attributes, which isn't neccesary under most circumstances.  

**Roadmap:**  
At the moment I work on [FFT](https://en.wikipedia.org/wiki/Fast_Fourier_transform) to make the fragment shaders audio responsive.  

[WebGL 2.0 / OpenGL ES 3.0](https://www.khronos.org/assets/uploads/developers/library/2017-webgl-webinar/Khronos-Webinar-WebGL-20-is-here_What-you-need-to-know_Apr17.pdf) has tons of new features and the shadertoy-team has done a lot of work, I am unsure if I can keep up with all the features in the future, but I think this code isn't the worst boilerplate.  

A detailed ToDo-List will be submitted. 