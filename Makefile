ENTRY_FILES = resources/amara2/main/main.cpp

BUILD_NAME = Amara2
BUILD_PATH = build

BUILD_EXECUTABLE_WIN = $(BUILD_PATH)/$(BUILD_NAME).exe
BUILD_EXECUTABLE_LINUX = $(BUILD_PATH)/$(BUILD_NAME).game

COMPILER = clang++

SDL_INCLUDE_PATHS_WIN64 = -Iresources\libs\SDL3-3.2.8\x86_64-w64-mingw32\include
SDL_LIBRARY_PATHS_WIN64 = -Lresources\libs\SDL3-3.2.8\x86_64-w64-mingw32\lib
SDL_PATHS_WIN64 = $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64) 
SDL_LINKER_FLAGS_WIN64 = -lSDL3

SDL_INCLUDE_PATHS_LINUX = `sdl2-config --cflags`

RENDERING_FLAGS = -DAMARA_OPENGL -lopengl32

LINKER_FLAGS_WIN64 = -Wl,-Bstatic -Wl,-Bdynamic -static-libstdc++ -static-libgcc -pthread $(RENDERING_FLAGS) $(SDL_LINKER_FLAGS_WIN64) -static

OTHER_LIB_LINKS = -Lresources/libs/tinyxml2/lib -ltinyxml2
OTHER_LIB_PATHS = -I./src -Iresources/libs/nlohmann/include -Iresources/libs/murmurhash3 -Iresources/libs/lua -Iresources/libs/sol2 -Iresources/libs/stb -Iresources/libs/glm -Iresources/libs/tinyxml2/include

OTHER_LIB = $(OTHER_LIB_LINKS) $(OTHER_LIB_PATHS)

AMARA_PATH = -I ./resources/amara2

# INCLUDE_DEPTH = 1000
# EXTRA_OPTIONS = -fmax-include-depth=$(INCLUDE_DEPTH)

COMPILER_FLAGS = -w -Wall -m64 -std=c++17
# COMPILER_FLAGS = -w -Wl,-subsystem,windows

playwin:
	$(BUILD_EXECUTABLE_WIN)

playlinux:
	./$(BUILD_EXECUTABLE_LINUX)

play:
	make playwin

all:
	@echo "Usage: make (option)"
	@echo "Options: win64 win32 linux"
	@echo "Options: playwin playlinux"

cpamara:
	rm -rf amara/*
	cp -T -r ../amara/amara ./amara
	rm -rf amaraWeb/*
	cp -T -r ../amara/amaraWeb ./amaraWeb
	rm -rf amaraRPG/*
	cp -T -r ../amaraRPG/amaraRPG ./amaraRPG

cplibs:
	rm -rf resources/libs/*
	cp -T -r ../amara/libs ./libs
	rm -rf resources/dlls/*
	cp -T -r ../amara/dlls ./dlls

cpAssets:
	cp -R assets/ build/

cpAssets_alt:
	if not exist build md build
	if not exist "build\assets" md "build\assets"
	xcopy /s /e /i /y "assets\*.*" "build\assets"

cpDLLs:
	cp resources/dlls/win64/* $(BUILD_PATH)/

cpDLLsAlt:
	xcopy /s /e /i /y "dlls\win64\*.*" "$(BUILD_PATH)\"
	if not exist "$(BUILD_PATH)\saves" md "$(BUILD_PATH)\saves"

win: $(ENTRY_FILES)
	rm -rf ./build/*
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLs

win_alt: $(ENTRY_FILES)
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(THEORA_WIN) $(SDL_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLsAlt

linux:
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_INCLUDE_PATHS_LINUX) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BUILD_EXECUTABLE_LINUX)
	mkdir $(BUILD_PATH)/saves

valgrind:
	rm -rf build/assets/*
	cp -R assets/ build/
	valgrind --leak-check=yes ./$(BUILD_EXECUTABLE_LINUX)

setup-apt64:
	sudo apt-get install libsdl2-2.0-0
	sudo apt-get install libsdl2-dev
	sudo apt-get install libsdl2-image-dev
	sudo apt-get install libsdl2-ttf-dev
	sudo apt-get install libsdl2-mixer-dev
	sudo apt-get install libsdl2-net-dev

setup-yum64:
	sudo yum install SDL2
	sudo yum install SDL2_image
	sudo yum install SDL2_ttf
	sudo yum install SDL2_mixer
	sudo yum install SDL2_net
	sudo yum install SDL2_image-devel
	sudo yum install SDL2_ttf-devel
	sudo yum install SDL2_mixer-devel
	sudo yum install SDL2_net-devel

setup-pacman64:
	sudo pacman -S sdl2
	sudo pacman -S sdl2_image
	sudo pacman -S sdl2_ttf
	sudo pacman -S sdl2_mixer
	sudo pacman -S sdl2_net
