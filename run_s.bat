g++ ./src/scatter.cpp -o ./build/scatter.exe -I./include/ -L./lib -lglfw3 -lopengl32 -lgdi32 -lglew32 -lglu32 --std=c++17 -Wall -Wextra -pedantic -O3 -ffast-math
.\build\scatter.exe