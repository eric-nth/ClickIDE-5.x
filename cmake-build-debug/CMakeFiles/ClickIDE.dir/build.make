# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "D:\Program Files\JetBrains\CLion 2020.1.1\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "D:\Program Files\JetBrains\CLion 2020.1.1\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\GitHub\ClickIDE-5.x

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\GitHub\ClickIDE-5.x\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ClickIDE.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ClickIDE.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ClickIDE.dir/flags.make

CMakeFiles/ClickIDE.dir/main.cpp.obj: CMakeFiles/ClickIDE.dir/flags.make
CMakeFiles/ClickIDE.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\GitHub\ClickIDE-5.x\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ClickIDE.dir/main.cpp.obj"
	D:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\ClickIDE.dir\main.cpp.obj -c D:\GitHub\ClickIDE-5.x\main.cpp

CMakeFiles/ClickIDE.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ClickIDE.dir/main.cpp.i"
	D:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\GitHub\ClickIDE-5.x\main.cpp > CMakeFiles\ClickIDE.dir\main.cpp.i

CMakeFiles/ClickIDE.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ClickIDE.dir/main.cpp.s"
	D:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\GitHub\ClickIDE-5.x\main.cpp -o CMakeFiles\ClickIDE.dir\main.cpp.s

CMakeFiles/ClickIDE.dir/ClickIDE_private.rc.obj: CMakeFiles/ClickIDE.dir/flags.make
CMakeFiles/ClickIDE.dir/ClickIDE_private.rc.obj: ../ClickIDE_private.rc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\GitHub\ClickIDE-5.x\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building RC object CMakeFiles/ClickIDE.dir/ClickIDE_private.rc.obj"
	D:\MinGW\bin\windres.exe -O coff $(RC_DEFINES) $(RC_INCLUDES) $(RC_FLAGS) D:\GitHub\ClickIDE-5.x\ClickIDE_private.rc CMakeFiles\ClickIDE.dir\ClickIDE_private.rc.obj

# Object files for target ClickIDE
ClickIDE_OBJECTS = \
"CMakeFiles/ClickIDE.dir/main.cpp.obj" \
"CMakeFiles/ClickIDE.dir/ClickIDE_private.rc.obj"

# External object files for target ClickIDE
ClickIDE_EXTERNAL_OBJECTS =

ClickIDE.exe: CMakeFiles/ClickIDE.dir/main.cpp.obj
ClickIDE.exe: CMakeFiles/ClickIDE.dir/ClickIDE_private.rc.obj
ClickIDE.exe: CMakeFiles/ClickIDE.dir/build.make
ClickIDE.exe: CMakeFiles/ClickIDE.dir/linklibs.rsp
ClickIDE.exe: CMakeFiles/ClickIDE.dir/objects1.rsp
ClickIDE.exe: CMakeFiles/ClickIDE.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\GitHub\ClickIDE-5.x\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ClickIDE.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ClickIDE.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ClickIDE.dir/build: ClickIDE.exe

.PHONY : CMakeFiles/ClickIDE.dir/build

CMakeFiles/ClickIDE.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ClickIDE.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ClickIDE.dir/clean

CMakeFiles/ClickIDE.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\GitHub\ClickIDE-5.x D:\GitHub\ClickIDE-5.x D:\GitHub\ClickIDE-5.x\cmake-build-debug D:\GitHub\ClickIDE-5.x\cmake-build-debug D:\GitHub\ClickIDE-5.x\cmake-build-debug\CMakeFiles\ClickIDE.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ClickIDE.dir/depend

