# CC:=ccache $(CC)
CFLAGS+= -Wall -Werror -std=gnu99 -g -I. -Isrc
LDFLAGS=-pthread

BINARIES= front_end/prgsem back_end/prgsem-comp_module

CFLAGS+=$(shell sdl2-config --cflags)
LDFLAGS+=$(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf 

COMMON_OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
FRONT_END_OBJS = $(patsubst %.c,%.o,$(wildcard front_end/*.c)) $(COMMON_OBJS)
BACK_END_OBJS = $(patsubst %.c,%.o,$(wildcard back_end/*.c)) $(COMMON_OBJS)

FRAMES := $(wildcard frames/*.png)
OUTPUT := output.mp4

all: $(BINARIES)

front_end/prgsem: $(FRONT_END_OBJS)
	$(CC) $(CFLAGS) $(FRONT_END_OBJS) $(LDFLAGS) -o $@

back_end/prgsem-comp_module: $(BACK_END_OBJS)
	$(CC) $(CFLAGS) $(BACK_END_OBJS) $(LDFLAGS) -o $@


$(OUTPUT): $(FRAMES)
	ffmpeg -r 30 -i frames/frame%04d.png -c:v libx264 -vf "fps=25,format=yuv420p" $@

.PHONY: video
video: $(OUTPUT)

clean:
	rm -f $(BINARIES) src/*.o front_end/*.o back_end/*.o