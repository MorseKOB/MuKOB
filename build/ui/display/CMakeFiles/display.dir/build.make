# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ed/code/mukob/MuKOB/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ed/code/mukob/MuKOB/build

# Include any dependencies generated for this target.
include ui/display/CMakeFiles/display.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include ui/display/CMakeFiles/display.dir/compiler_depend.make

# Include the progress variables for this target.
include ui/display/CMakeFiles/display.dir/progress.make

# Include the compile flags for this target's objects.
include ui/display/CMakeFiles/display.dir/flags.make

ui/display/CMakeFiles/display.dir/display.c.obj: ui/display/CMakeFiles/display.dir/flags.make
ui/display/CMakeFiles/display.dir/display.c.obj: /home/ed/code/mukob/MuKOB/src/ui/display/display.c
ui/display/CMakeFiles/display.dir/display.c.obj: ui/display/CMakeFiles/display.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ed/code/mukob/MuKOB/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object ui/display/CMakeFiles/display.dir/display.c.obj"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT ui/display/CMakeFiles/display.dir/display.c.obj -MF CMakeFiles/display.dir/display.c.obj.d -o CMakeFiles/display.dir/display.c.obj -c /home/ed/code/mukob/MuKOB/src/ui/display/display.c

ui/display/CMakeFiles/display.dir/display.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/display.dir/display.c.i"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ed/code/mukob/MuKOB/src/ui/display/display.c > CMakeFiles/display.dir/display.c.i

ui/display/CMakeFiles/display.dir/display.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/display.dir/display.c.s"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ed/code/mukob/MuKOB/src/ui/display/display.c -o CMakeFiles/display.dir/display.c.s

ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj: ui/display/CMakeFiles/display.dir/flags.make
ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj: /home/ed/code/mukob/MuKOB/src/ui/display/font_9_10_h.c
ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj: ui/display/CMakeFiles/display.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ed/code/mukob/MuKOB/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj -MF CMakeFiles/display.dir/font_9_10_h.c.obj.d -o CMakeFiles/display.dir/font_9_10_h.c.obj -c /home/ed/code/mukob/MuKOB/src/ui/display/font_9_10_h.c

ui/display/CMakeFiles/display.dir/font_9_10_h.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/display.dir/font_9_10_h.c.i"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ed/code/mukob/MuKOB/src/ui/display/font_9_10_h.c > CMakeFiles/display.dir/font_9_10_h.c.i

ui/display/CMakeFiles/display.dir/font_9_10_h.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/display.dir/font_9_10_h.c.s"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ed/code/mukob/MuKOB/src/ui/display/font_9_10_h.c -o CMakeFiles/display.dir/font_9_10_h.c.s

ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj: ui/display/CMakeFiles/display.dir/flags.make
ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj: /home/ed/code/mukob/MuKOB/src/ui/display/ili9341_spi.c
ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj: ui/display/CMakeFiles/display.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ed/code/mukob/MuKOB/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj -MF CMakeFiles/display.dir/ili9341_spi.c.obj.d -o CMakeFiles/display.dir/ili9341_spi.c.obj -c /home/ed/code/mukob/MuKOB/src/ui/display/ili9341_spi.c

ui/display/CMakeFiles/display.dir/ili9341_spi.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/display.dir/ili9341_spi.c.i"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ed/code/mukob/MuKOB/src/ui/display/ili9341_spi.c > CMakeFiles/display.dir/ili9341_spi.c.i

ui/display/CMakeFiles/display.dir/ili9341_spi.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/display.dir/ili9341_spi.c.s"
	cd /home/ed/code/mukob/MuKOB/build/ui/display && /usr/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ed/code/mukob/MuKOB/src/ui/display/ili9341_spi.c -o CMakeFiles/display.dir/ili9341_spi.c.s

display: ui/display/CMakeFiles/display.dir/display.c.obj
display: ui/display/CMakeFiles/display.dir/font_9_10_h.c.obj
display: ui/display/CMakeFiles/display.dir/ili9341_spi.c.obj
display: ui/display/CMakeFiles/display.dir/build.make
.PHONY : display

# Rule to build all files generated by this target.
ui/display/CMakeFiles/display.dir/build: display
.PHONY : ui/display/CMakeFiles/display.dir/build

ui/display/CMakeFiles/display.dir/clean:
	cd /home/ed/code/mukob/MuKOB/build/ui/display && $(CMAKE_COMMAND) -P CMakeFiles/display.dir/cmake_clean.cmake
.PHONY : ui/display/CMakeFiles/display.dir/clean

ui/display/CMakeFiles/display.dir/depend:
	cd /home/ed/code/mukob/MuKOB/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ed/code/mukob/MuKOB/src /home/ed/code/mukob/MuKOB/src/ui/display /home/ed/code/mukob/MuKOB/build /home/ed/code/mukob/MuKOB/build/ui/display /home/ed/code/mukob/MuKOB/build/ui/display/CMakeFiles/display.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : ui/display/CMakeFiles/display.dir/depend

