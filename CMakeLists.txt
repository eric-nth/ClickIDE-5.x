project(ClickIDE-5.x)
set(CMAKE_CXX_FLAGS "-mwindows")
set(SOURCES main.cpp main.h ClickIDE_private.rc)
add_executable(ClickIDE ${SOURCES})