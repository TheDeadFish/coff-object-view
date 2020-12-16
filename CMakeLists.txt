project(win32-res-lib)
cmake_minimum_required(VERSION 3.12)
set( EXECUTABLE_OUTPUT_PATH ../bin)
set(CMAKE_CXX_FLAGS_RELEASE "-s -Wl,-gc-sections -mwindows")

add_executable(coff-view src/object.cc src/section.cc 
	src/symbol.cc src/main.cc src/resource.rc
	src/stuff.cc
)
	
	
target_link_libraries(coff-view -lwin32hlp -limgLib -lexshit 
	-lstdshit -lcomdlg32 -limagehlp -lgdi32)
