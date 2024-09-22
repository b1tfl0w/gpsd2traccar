#Makefile

#OBJS specifies which files to compile as part of the project
OBJS = gpsd2traccar.c

#CC specifies which compiler we're using
CC = gcc

#Preprocessor flags
#CPPFLAGS = -D__LINUX_ALSA__

#COMPILER_FLAGS specifies the additional compilation options we're using
# -g debug, -Wall suppress warnings
COMPILER_FLAGS = -O0 -Wall

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lgps

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = gpsd2traccar

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

