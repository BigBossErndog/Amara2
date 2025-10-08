ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
endif

ENTRY_FILES = ./amara2/main/main.cpp

BUILD_NAME = Amara2
BUILD_PATH = build
EXE_OPTIONS = -context ../

BUILD_EXECUTABLE_WIN = $(BUILD_PATH)/$(BUILD_NAME).exe
BUILD_EXECUTABLE_LINUX = $(BUILD_PATH)/$(BUILD_NAME).game

WINDOWS_BUILDMODULE_PATH = build_modules\amara2_windows_build_module

CLANG_LLVM_PATH = $(WINDOWS_BUILDMODULE_PATH)\clang-llvm
COMPILER = $(CLANG_LLVM_PATH)\bin\clang++
RC_COMPILER = $(CLANG_LLVM_PATH)\bin\llvm-rc

# WINDOWS_COMPILER_FLAGS = -w -Wall -m64 -std=c++17 -Wl,/NOIMPLIB -DAMARA_DEBUG_BUILD
WINDOWS_COMPILER_FLAGS = -w -m64 -Wl,/SUBSYSTEM:WINDOWS -Wl,/NOIMPLIB -std=c++17 -O2

LINUX_COMPILER_FLAGS = -w -Wall -m32 -std=c++17

RESOURCES = $(WINDOWS_BUILDMODULE_PATH)/resources

SDL_INCLUDE_PATHS_WIN64 = -I$(RESOURCES)/libs/SDL3-3.2.16/include
SDL_LIBRARY_PATHS_WIN64 = -L$(RESOURCES)/libs/SDL3-3.2.16/lib/x64
SDL_PATHS_WIN64 = $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64)
SDL_LINKER_FLAGS_WIN64 = -lSDL3

SDL_INCLUDE_PATHS_LINUX = `sdl2-config --cflags`

RENDERING_FLAGS = -DAMARA_OPENGL -lopengl32

STDLIB_FLAG = -stdlib=libc++

WINDOWS_SYSTEM_LIBS = -lshell32 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion # Add system libraries for Windows API
LINKER_FLAGS_WIN64 = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread $(RENDERING_FLAGS) $(SDL_LINKER_FLAGS_WIN64) $(WINDOWS_SYSTEM_LIBS) -static

LINKER_FLAGS_LINUX = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread `sdl2-config --libs` # Add rendering libs like -lGL, and other necessary libs like -lm, -ldl

OTHER_LIB_LINKS = 
OTHER_LIB_PATHS = -Isrc -I$(RESOURCES)/libs/nlohmann/include -I$(RESOURCES)/libs/murmurhash3 -I$(RESOURCES)/libs/lua -I$(RESOURCES)/libs/sol2 -I$(RESOURCES)/libs/stb -I$(RESOURCES)/libs/glm -I$(RESOURCES)/libs/minimp3 -I$(RESOURCES)/libs/portable-file-dialogs -I$(RESOURCES)/libs/tinyxml2

OTHER_LIB = $(OTHER_LIB_PATHS)

AMARA_PATH = -Iamara2 -Iplugins

EXTRA_OPTIONS = -DAMARA_TESTING -DAMARA_PLUGINS -DAMARA_ENGINE_TOOLS

CURRENT_PATH := $(CURDIR)

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

ICON_SRC = assets/icons/icon.ico
ICON_RC = assets/icons/icon.rc
ICON_RES = assets/icons/icon.res
build-icon:
	@echo 1 ICON "$(ICON_SRC)" > $(ICON_RC)
	$(RC_COMPILER) $(ICON_RC)

cpAssets_alt:
	if not exist build md build
	if not exist "build\assets" md "build\assets"
	xcopy /s /e /i /y "assets\*.*" "build\assets"

cpdll:
	xcopy /s /e /i /y "$(RESOURCES)\dlls\win64\*.*" "$(BUILD_PATH)\"

cpdirs:
	if not exist $(BUILD_PATH) md $(BUILD_PATH)
	if not exist "$(BUILD_PATH)\assets" md "$(BUILD_PATH)\assets"
	if not exist "$(BUILD_PATH)\files" md "$(BUILD_PATH)\files"
	if not exist "$(BUILD_PATH)\lua_scripts" md "$(BUILD_PATH)\lua_scripts"
	xcopy /s /e /i /y "assets\*.*" "$(BUILD_PATH)\assets"
	xcopy /s /e /i /y "files\*.*" "$(BUILD_PATH)\files"
	xcopy /s /e /i /y "lua_scripts\*.*" "$(BUILD_PATH)\lua_scripts"

cpbuildmodules:
	if exist "$(BUILD_PATH)\build_modules" rmdir /s /q "$(BUILD_PATH)\build_modules"
	xcopy /s /e /i /y "build_modules\*.*" "$(BUILD_PATH)\build_modules\"
	if exist "$(BUILD_PATH)\build_modules\amara2_windows_build_module\emsdk\upstream\emscripten\cache\sanity.txt" del /f /q "$(BUILD_PATH)\build_modules\amara2_windows_build_module\emsdk\upstream\emscripten\cache\sanity.txt"

# Using clang from $(CLANG_LLVM_PATH)
win: $(ENTRY_FILES)
	make build-icon
	$(COMPILER) $(ENTRY_FILES) $(ICON_RES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(WINDOWS_COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpdll

win-test-copy:
	make cpdirs
	if not exist "$(BUILD_PATH)\amara2\" md "$(BUILD_PATH)\amara2\"
	xcopy /s /e /i /y "amara2\*.*" "$(BUILD_PATH)\amara2"

win-test:
	make win
	make win-test-copy

win-release:
	if exist $(BUILD_PATH) ( rmdir /s /q $(BUILD_PATH) )
	if not exist $(BUILD_PATH) md $(BUILD_PATH)
	make win
	make cpdirs
	if not exist "$(BUILD_PATH)\amara2\" md "$(BUILD_PATH)\amara2\"
	xcopy /s /e /i /y "amara2\*.*" "$(BUILD_PATH)\amara2"
	if exist "$(BUILD_PATH)\files\settings.json" del "$(BUILD_PATH)\files\settings.json"

linux:
	$(COMPILER) $(ENTRY_FILES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_INCLUDE_PATHS_LINUX) $(LINUX_COMPILER_FLAGS) $(STDLIB_FLAG) $(LINKER_FLAGS_LINUX) -o $(BUILD_EXECUTABLE_LINUX)
	mkdir $(BUILD_PATH)/saves

EMSCRIPTEN_COMPILER = "$(WINDOWS_BUILDMODULE_PATH)\emsdk\upstream\emscripten\em++"
EMSCRIPTEN_SERVER = "$(WINDOWS_BUILDMODULE_PATH)\emsdk\upstream\emscripten\emrun"
EMSCRIPTEN_BUILD_NAME = $(BUILD_NAME).html
EMSCRIPTEN_BUILD_PATH = $(BUILD_PATH)/$(EMSCRIPTEN_BUILD_NAME)
EMSCRIPTEN_INCLUDE = -I$(WINDOWS_BUILDMODULE_PATH)\emsdk\upstream\emscripten\system\include
EMSCRIPTEN_SDL = "$(WINDOWS_BUILDMODULE_PATH)\emsdk\upstream\emscripten\SDl3\lib\libSDL3.a" -I$(WINDOWS_BUILDMODULE_PATH)\emsdk\upstream\emscripten\SDL3\include
EMSCRIPTEN_COMPILER_FLAGS = -w -std=c++17 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s EXCEPTION_CATCHING_ALLOWED='["std::exception"]' -O2
EMSCRIPTEN_PRELOADS = --preload-file assets --preload-file lua_scripts --preload-file data
EMSCRIPTEN_EXTRA_OPTIONS = -DAMARA_ENGINE_TOOLS
web:
	if exist $(BUILD_PATH) ( rmdir /s /q $(BUILD_PATH) )
	if not exist $(BUILD_PATH) md $(BUILD_PATH)
	set EMSDK_PYTHON=$(RESOURCES)\emsdk\python\3.13.3_64bit\python.exe && \
    $(EMSCRIPTEN_COMPILER) $(ENTRY_FILES) $(EMSCRIPTEN_SDL) $(AMARA_PATH) $(OTHER_LIB) $(EMSCRIPTEN_COMPILER_FLAGS) $(EMSCRIPTEN_EXTRA_OPTIONS) $(EMSCRIPTEN_PRELOADS) -o $(EMSCRIPTEN_BUILD_PATH)

play-web:
	set EMSDK_PYTHON=$(CURRENT_PATH)\build_modules\amara2_windows_build_module\emsdk\python\3.13.3_64bit\python.exe && \
	$(EMSCRIPTEN_SERVER) --port 8080 .

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