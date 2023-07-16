c:\mingw\mingw64\mingw.bat

cmake .. -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++

mingw32-make

g++ basic.cpp -o b.exe -I../external/glew-cmake/include/ -I../include/ -I../external/glfw/include/

g++ basic.cpp -o b.exe -I../external/glew-cmake/include/ -I../include/ -I../external/glfw/include/ -L../build/external/glew-cmake/lib -L../build/external/glfw/src -lglfw3 -lgdi32 -lopengl32 -lglew

g++ test.cpp -o t.exe -I../include/ -I../external/glfw/include/ -L../build/external/glfw/src -L. -I. -lglfw3 -lopengl32 -lgdi32 -lglew32

g++ line.cpp -o l.exe -I../include/ -I../external/glfw/include/ -L../build/external/glfw/src -L. -I. -lglfw3 -lopengl32 -lgdi32 -lglew32 -lglu32


----------

g++ test.cpp -o t.exe -I./include/ -L./lib -lglfw3 -lopengl32 -lgdi32 -lglew32 -lglu32
