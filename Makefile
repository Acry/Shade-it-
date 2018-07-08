SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

CFLAGS   = -Wall -Wextra -mtune=native `sdl2-config --cflags`
WARN_OFF = -Wno-implicit-function-declaration -Wno-unused-function
CFLAGS1  = $(CFLAGS) $(WARN_OFF)
LDFLAGS  = `sdl2-config --libs` -lm -lGL
LDFLAGS1 = $(LDFLAGS)  -lSDL2_image
LDFLAGS2 = $(LDFLAGS) $(LDFLAGS1) -lSOIL
LDFLAGS3 = $(LDFLAGS1) -lGLEW
LDFLAGS4 = $(LDFLAGS)  -lGLU
LDFLAGS5 = $(LDFLAGS1) $(LDFLAGS3)
LDFLAGS6 = $(LDFLAGS5) -lSDL2_mixer
LDFLAGS7 = $(LDFLAGS5) -lcurl
LDFLAGS8 = $(LDFLAGS7) -ljansson
srcdir	 =src/
builddir =build/ ##not used yet

TARGETS	 =  shade_it## 3b6

.PHONY: all
all: $(TARGETS)

# current Shade it! version
shade_it: $(srcdir)shade_it.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)




# load shadertoy fragment shader optional
3a0: $(srcdir)3a0.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS5)

# make new shader in preferred Editor
3a1: $(srcdir)3a1.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS5)

# make new shader in preferred Editor, check file change via inotify an epoll
3a2: $(srcdir)3a2.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS5)

# getting Textures work 1
3a3: $(srcdir)3a3.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS5)

# autoload json
3a4: $(srcdir)3a4.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# autoload assets
3a5: $(srcdir)3a5.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# auto apply textures
3a6: $(srcdir)3a6.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# refactor mouse 1
3a7: $(srcdir)3a7.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# screenshot function auto (5 seconds after shader loaded) or overwrite with s
3a8: $(srcdir)3a8.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# font stash - show shader names and save in screenshot
3a9: $(srcdir)3a9.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# fix current shader tracking 1
3b0: $(srcdir)3b0.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# refactor mouse 2
3b1: $(srcdir)3b1.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS8)

# implement delta time and pause
3b2: $(srcdir)3b2.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)

# refactor uniforms 1
3b3: $(srcdir)3b3.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)

# DnD text
3b4: $(srcdir)3b4.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)

# DnD files, get new shader back
3b5: $(srcdir)3b5.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)

# edit current shader - saving will be tricky
# but no need to worry until edit-mode/UI is there
3b6: $(srcdir)3b6.c
	$(CC) $(CFLAGS1) -o $@ $+ $(LDFLAGS8)

# fft

# timing, interpolation

# texture channels

# buffers

# bring custom shaders back

# shader gallery

# help - uniforms, functions, equations

# soundcloud

# media/DnD

# media/DnD on certain channel

# multibuffer

.PHONY: clean
clean:
	@rm $(TARGETS) 2>/dev/null || true

