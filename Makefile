ENTRY_FILES = ./amara2/main/main.cpp

BUILD_NAME = Amara2
BUILD_PATH = build
EXE_OPTIONS = -context ../

BUILD_EXECUTABLE_WIN = $(BUILD_PATH)/$(BUILD_NAME).exe
BUILD_EXECUTABLE_LINUX = $(BUILD_PATH)/$(BUILD_NAME).game

CLANG_LLVM_PATH = resources/clang-llvm
COMPILER = $(CLANG_LLVM_PATH)/bin/clang++

# WINDOWS_COMPILER_FLAGS = -w -Wall -m64 -std=c++17 -DAMARA_DEBUG_BUILD
WINDOWS_COMPILER_FLAGS = -w -m64 -Wl,/SUBSYSTEM:WINDOWS -std=c++17

# LINUX_COMPILER_FLAGS = -w -Wall -m32 -std=c++17

SDL_INCLUDE_PATHS_WIN64 = -Iresources/libs/SDL3-3.2.8/x86_64-w64-mingw32/include
SDL_LIBRARY_PATHS_WIN64 = -Lresources/libs/SDL3-3.2.8/x86_64-w64-mingw32/lib
SDL_PATHS_WIN64 = $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64) 
SDL_LINKER_FLAGS_WIN64 = resources/libs/SDL3-3.2.8/x86_64-w64-mingw32/lib/libSDL3.dll.a

SDL_INCLUDE_PATHS_LINUX = `sdl2-config --cflags`

RENDERING_FLAGS = -DAMARA_OPENGL -lopengl32

STDLIB_FLAG = -stdlib=libc++

WINDOWS_SYSTEM_LIBS = -lshell32 -luser32 # Add system libraries for Windows API
LINKER_FLAGS_WIN64 = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread $(RENDERING_FLAGS) $(SDL_LINKER_FLAGS_WIN64) $(WINDOWS_SYSTEM_LIBS) -static

LINKER_FLAGS_LINUX = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread `sdl2-config --libs` # Add rendering libs like -lGL, and other necessary libs like -lm, -ldl

OTHER_LIB_LINKS = -Lresources/libs/tinyxml2/lib/win resources/libs/tinyxml2/lib/win/libtinyxml2.a
OTHER_LIB_PATHS = -I./src -Iresources/libs/nlohmann/include -Iresources/libs/murmurhash3 -Iresources/libs/lua -Iresources/libs/sol2 -Iresources/libs/stb -Iresources/libs/glm -Iresources/libs/tinyxml2/include -Iresources/libs/minimp3 -Iresources/libs/portable-file-dialogs

OTHER_LIB = $(OTHER_LIB_LINKS) $(OTHER_LIB_PATHS)

AMARA_PATH = -I ./amara2 -I ./plugins

# INCLUDE_DEPTH = 1000
# EXTRA_OPTIONS = -fmax-include-depth=$(INCLUDE_DEPTH)
EXTRA_OPTIONS = -DAMARA_TESTING -DAMARA_PLUGINS

playwin:
	$(BUILD_EXECUTABLE_WIN) $(EXE_OPTIONS)

playlinux:
	./$(BUILD_EXECUTABLE_LINUX)

play:
	make playwin

all:
	@echo "Usage: make (option)"
	@echo "Options: win linux"
	@echo "Options: play(default windows) playwin playlinux"

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
	cp -R assets/ $(BUILD_PATH)/

cpAssets_alt:
	if not exist build md build
	if not exist "build\assets" md "build\assets"
	xcopy /s /e /i /y "assets\*.*" "build\assets"

cpDLLs:
	cp resources/dlls/win64/* $(BUILD_PATH)/

cpDLLs_alt:
	xcopy /s /e /i /y "resources\dlls\win64\*.*" "$(BUILD_PATH)\"

cpDirs:
	cp -R assets/ $(BUILD_PATH)/
	cp -R data/ $(BUILD_PATH)/
	cp -R lua_scripts/ $(BUILD_PATH)/

# Using clang from $(CLANG_LLVM_PATH)
win: $(ENTRY_FILES)
	mkdir -p ./$(BUILD_PATH)
	rm -rf ./$(BUILD_PATH)/*
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(WINDOWS_COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLs

# Using clang from $(CLANG_LLVM_PATH)
win_alt: $(ENTRY_FILES)
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(WINDOWS_COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLs_alt
	
linux:
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_INCLUDE_PATHS_LINUX) $(LINUX_COMPILER_FLAGS) $(STDLIB_FLAG) $(LINKER_FLAGS_LINUX) -o $(BUILD_EXECUTABLE_LINUX)
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
