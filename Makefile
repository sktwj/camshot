#	GZe_MP3_v2 Makefile
#
#
#
#
# Author: Bc. Gabriel Zabusek		25.11.2008

#compiler definitions
CROSS_COMPILE = 

CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS  = $(CROSS_COMPILE)as
LD  = $(CROSS_COMPILE)ld

BASE_DIR = ./

CCWARNINGS = -Wall -Wextra -Wno-unused-parameter -Wmissing-prototypes
CXXWARNINGS = -Wall -Wextra -Wno-unused-parameter

#compiler settings for C files
CFLAGS  = -O2 -pipe -ansi -pedantic -pthread \
	   -I$(BASE_DIR) \
        $(CCWARNINGS)

#compiler settings for C++ files
CXXFLAGS = -O2 -pipe \
	   -I$(BASE_DIR) \
	     $(CXXWARNINGS) \

#source files
CORE_SRC = \
	$(BASE_DIR)main.c \
	$(BASE_DIR)arguments.c \
	$(BASE_DIR)camera.c \
    $(BASE_DIR)image.c \
    $(BASE_DIR)shmem.c \

SHM_TEST_SRC = \
	$(BASE_DIR)shmem_test.c \
	$(BASE_DIR)shmem.c \
	$(BASE_DIR)image.c \
	$(BASE_DIR)arguments.c \


CORE_BIN = camshot 
SHM_TEST_BIN = shmem_test

#objects
CORE_OBJ := $(addsuffix .o,$(basename $(CORE_SRC)))
SHM_TEST_OBJ := $(addsuffix .o,$(basename $(SHM_TEST_SRC)))

all: $(CORE_BIN)

shm_test: $(SHM_TEST_BIN)

%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cxx
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<


$(CORE_BIN): $(CORE_OBJ)
	@echo
	@echo "Linking final binary $(CORE_BIN)"
	@$(CC) $(CFLAGS) -o $@ $(CORE_OBJ)

$(SHM_TEST_BIN): $(SHM_TEST_OBJ)
	@echo
	@echo "Linking final binary $(SHM_TEST_BIN)"
	@$(CC) $(CFLAGS) -o $@ $(SHM_TEST_OBJ)

clean:
	rm -f $(CORE_BIN)
	rm -f $(BASE_DIR)*.o

install:
	cp -f $(CORE_BIN) /usr/bin/
