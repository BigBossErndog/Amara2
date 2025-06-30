ENTRY_FILES = ./amara2/main/main.cpp

BUILD_NAME = Amara2
BUILD_PATH = build
EXE_OPTIONS = -context ../

BUILD_EXECUTABLE_WIN = $(BUILD_PATH)/$(BUILD_NAME).exe
BUILD_EXECUTABLE_LINUX = $(BUILD_PATH)/$(BUILD_NAME).game

CLANG_LLVM_PATH = resources/clang-llvm
COMPILER = $(CLANG_LLVM_PATH)/bin/clang++
RC_COMPILER = $(CLANG_LLVM_PATH)/bin/llvm-rc

# WINDOWS_COMPILER_FLAGS = -w -Wall -m64 -std=c++17 -Wl,/NOIMPLIB -DAMARA_DEBUG_BUILD
WINDOWS_COMPILER_FLAGS = -w -m64 -Wl,/SUBSYSTEM:WINDOWS -Wl,/NOIMPLIB -std=c++17

# LINUX_COMPILER_FLAGS = -w -Wall -m32 -std=c++17

SDL_INCLUDE_PATHS_WIN64 = -Iresources/libs/SDL3-3.2.16/include
SDL_LIBRARY_PATHS_WIN64 = -Lresources/libs/SDL3-3.2.16/lib/x64
SDL_PATHS_WIN64 = $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64) 
SDL_LINKER_FLAGS_WIN64 = -lSDL3

SDL_INCLUDE_PATHS_LINUX = `sdl2-config --cflags`

RENDERING_FLAGS = -DAMARA_OPENGL -lopengl32

STDLIB_FLAG = -stdlib=libc++

WINDOWS_SYSTEM_LIBS = -lshell32 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion # Add system libraries for Windows API
LINKER_FLAGS_WIN64 = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread $(RENDERING_FLAGS) $(SDL_LINKER_FLAGS_WIN64) $(WINDOWS_SYSTEM_LIBS) -static

LINKER_FLAGS_LINUX = -fuse-ld=lld $(STDLIB_FLAG) -L$(CLANG_LLVM_PATH)/lib -pthread `sdl2-config --libs` # Add rendering libs like -lGL, and other necessary libs like -lm, -ldl

OTHER_LIB_LINKS = resources\libs\Chipmunk2D_build\win\chipmunk.lib
OTHER_LIB_PATHS = -Isrc -Iresources\libs\Chipmunk2D_build\include -Iresources/libs/box2d/include -Iresources/libs/nlohmann/include -Iresources/libs/murmurhash3 -Iresources/libs/lua -Iresources/libs/sol2 -Iresources/libs/stb -Iresources/libs/glm -Iresources/libs/tinyxml2/include -Iresources/libs/minimp3 -Iresources/libs/portable-file-dialogs -Iresources/libs/tinyxml2 -Iresources/libs/utf8cpp -Iresources/libs/utf8cpp/include -Iresources/libs/utf8cpp/src

OTHER_LIB = $(OTHER_LIB_LINKS) $(OTHER_LIB_PATHS)

AMARA_PATH = -Iamara2 -Iplugins

# INCLUDE_DEPTH = 1000
# EXTRA_OPTIONS = -fmax-include-depth=$(INCLUDE_DEPTH)
EXTRA_OPTIONS = -DAMARA_TESTING -DAMARA_PLUGINS -DAMARA_ENGINE_TOOLS

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
	@echo '1 ICON "$(ICON_SRC)"' > $(ICON_RC)
	$(RC_COMPILER) $(ICON_RC)

cpAssets:
	cp -R assets/ $(BUILD_PATH)/

cpAssets_alt:
	if not exist build md build
	if not exist "build\assets" md "build\assets"
	xcopy /s /e /i /y "assets\*.*" "build\assets"

cpdll:
	cp resources/dlls/win64/* $(BUILD_PATH)/

cpdll-alt:
	xcopy /s /e /i /y "resources\dlls\win64\*.*" "$(BUILD_PATH)\"

cpdirs:
	cp -R assets/ $(BUILD_PATH)/
	cp -R data/ $(BUILD_PATH)/
	cp -R lua_scripts/ $(BUILD_PATH)/
	rm -f $(BUILD_PATH)/data/settings.json

cpdirs-alt:
	if not exist $(BUILD_PATH) md $(BUILD_PATH)
	if not exist "$(BUILD_PATH)\assets" md "$(BUILD_PATH)\assets"
	if not exist "$(BUILD_PATH)\data" md "$(BUILD_PATH)\data"
	if not exist "$(BUILD_PATH)\lua_scripts" md "$(BUILD_PATH)\lua_scripts"
	xcopy /s /e /i /y "assets\*.*" "$(BUILD_PATH)\assets"
	xcopy /s /e /i /y "data\*.*" "$(BUILD_PATH)\data"
	xcopy /s /e /i /y "lua_scripts\*.*" "$(BUILD_PATH)\lua_scripts"
	if exist "$(BUILD_PATH)/data/settings.json: del "$(BUILD_PATH)/data/settings.json"

# Using clang from $(CLANG_LLVM_PATH)
win: $(ENTRY_FILES)
	mkdir -p ./$(BUILD_PATH)
	rm -rf ./$(BUILD_PATH)/*
	make build-icon
	$(COMPILER) $(ENTRY_FILES) $(ICON_RES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(WINDOWS_COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpdll

win-release:
	make win
	make cpdirs
	cp -R amara2/ $(BUILD_PATH)/
	cp -R resources/ $(BUILD_PATH)/

# Using clang from $(CLANG_LLVM_PATH)
win-alt: $(ENTRY_FILES)
	mkdir -p ./$(BUILD_PATH)
	make build-icon
	$(COMPILER) $(ENTRY_FILES) $(ICON_RES) $(AMARA_PATH) $(OTHER_LIB) $(SDL_PATHS_WIN64) $(WINDOWS_COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpdll-alt

win-release-alt:
	make win-alt
	make cpdirs-alt
	if not exist "$(BUILD_PATH)\amara2\" md "$(BUILD_PATH)\amara2\"
	xcopy /s /e /i /y "amara2\*.*" "$(BUILD_PATH)\amara2"
	if not exist "$(BUILD_PATH)\resources\" md "$(BUILD_PATH)\resources\"
	xcopy /s /e /i /y "resources\*.*" "$(BUILD_PATH)\resources"
	
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
