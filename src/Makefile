CC = gcc
CXX = g++
LIBS = -lSDL -lSDL_image -lSDL_gfx -lSDLmain -lSDL_mixer -lconfig -lrt
TARGET=Freenger
TARGET_DINGOO=$(TARGET).dge
OBJS =  \
	Graph.cpp\
	main.cpp\
	Game.cpp\
	System.cpp\
	mylist.cpp\
 	Sound.cpp
	


TOOLCHAIN = /opt/mipsel-linux-uclibc/usr
CROSS_COMPILE = $(TOOLCHAIN)/bin/mipsel-linux-

SDL_BASE = $(TOOLCHAIN)/bin

CFLAGS = -I$(TOOLCHAIN)/include -Wall -march=mips32 -O3 -mtune=r4600\
			        -mno-mips16 -funroll-loops -ffast-math\
				-fomit-frame-pointer -fno-strength-reduce\
				-finline-functions -G0 -mips32\
				-mno-long-calls -mno-shared\
				-fno-exceptions -fno-rtti -Wno-unused-value

CC_GP = $(CROSS_COMPILE)gcc
CXX_GP = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
LIBS_GP = -L$(TOOLCHAIN)/lib -lSDL -lSDL_image -lSDL_gfx -lSDLmain -lSDL_mixer -lconfig -lrt
FLAGS_GP = -DDINGOO


FBSD = /usr/local/
LIBS_FBSD = -L$(FBSD)/lib -lSDL -lSDL_image -lSDL_gfx -lSDLmain -lSDL_mixer -lconfig
CFLAGS_FBSD = -I$(FBSD)/include -Wall

pc: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
dingoo: $(OBJS)
	export SDL_NOMOUSE=1
	$(CXX_GP) $(CFLAGS) $(LIBS_GP) -o $(TARGET_DINGOO) $(OBJS) $(FLAGS_GP)

clean:
	rm -f $(TARGET) $(TARGET_DINGOO) *.o

freebsd:
	$(CXX) $(CFLAGS_FBSD) $(LIBS_FBSD) -o $(TARGET) $(OBJS) -DFREE_BSD
