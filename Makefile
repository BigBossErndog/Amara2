SRC_FILES = src/main.cpp

BUILD_NAME = Amara2
BUILD_PATH = build

BUILD_EXECUTABLE_WIN = $(BUILD_PATH)/$(BUILD_NAME).exe
BUILD_EXECUTABLE_LINUX = $(BUILD_PATH)/$(BUILD_NAME).game

COMPILER = g++

SDL_INCLUDE_PATHS_WIN64 = -I resources/libs/SDL2/include/SDL2 -I resources/libs/SDL2_image/include/SDL2 -I resources/libs/SDL2_ttf/include/SDL2 -I resources/libs/SDL2_mixer/include/SDL2 -I resources/libs/SDL2_net/include/SDL2 -I resources/libs/SDL_FontCache
SDL_LIBRARY_PATHS_WIN64 = -L resources/libs/SDL2/lib -L resources/libs/SDL2_image/lib -L resources/libs/SDL2_ttf/lib -L resources/libs/SDL2_mixer/lib -L resources/libs/SDL2_net/lib
SDL_LINKER_FLAGS_WIN64 = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

SDL_INCLUDE_PATHS_LINUX = `sdl2-config --cflags` -I resources/libs/SDL_FontCache

THEORA_INCLUDE_PATHS_WIN64 = -I resources/libs/ogg/include -I resources/libs/vorbis/include -I resources/libs/theora/include -I resources/libs/sdlogv
THEORA_LIBRARY_PATHS_WIN64 = -L resources/libs/ogg/lib -L resources/libs/vorbis/lib -L resources/libs/theora/lib
THEORA_LINKER_FLAGS_WIN64 =  -ltheora -lvorbisenc -lvorbisfile -lvorbis -logg

THEORA_WIN64 = $(THEORA_INCLUDE_PATHS_WIN64) $(THEORA_LIBRARY_PATHS_WIN64) $(THEORA_LINKER_FLAGS_WIN64)

LINKER_FLAGS_WIN64 = -lmingw32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -Wa,-mbig-obj $(SDL_LINKER_FLAGS_WIN64) -DAMARA_WINDOWS

OTHER_LIB_PATHS = -I resources/libs/nlohmann/include -I ./src -I resources/libs/murmurhash3 -I resources/libs/lua -I resources/libs/sol2

AMARA_PATH = -I ./resources/amara2

INCLUDE_DEPTH = 1000
EXTRA_OPTIONS = -fmax-include-depth=$(INCLUDE_DEPTH)

COMPILER_FLAGS = -w -Wall
# COMPILER_FLAGS = -w -Wl,-subsystem,windows

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

win: $(SRC_FILES)
	rm -rf ./build/*
	$(COMPILER) $(SRC_FILES) $(AMARA_PATH) $(OTHER_LIB_PATHS) $(THEORA_WIN) $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLs

win_alt: $(SRC_FILES)
	$(COMPILER) $(SRC_FILES) $(AMARA_PATH) $(OTHER_LIB_PATHS) $(THEORA_WIN) $(SDL_INCLUDE_PATHS_WIN64) $(SDL_LIBRARY_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLsAlt

win32: $(SRC_FILES)
	make clearBuildWin
	cp -R assets/ build/
	cp -R videos/ build/
	$(COMPILER) $(SRC_FILES) -m32 $(AMARA_PATH) $(OTHER_LIB_PATHS) $(THEORA_WIN) $(SDL_INCLUDE_PATHS_WIN32) $(SDL_LIBRARY_PATHS_WIN32) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) -lmingw32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -Wa,-mbig-obj $(LINKER_FLAGS) -I resources/libs/SDL2-32/bin -o $(BUILD_EXECUTABLE_WIN)
	xcopy /s /e /i /y "dlls\win62\*.*" "$(BUILD_PATH)/"
	md "$(BUILD_PATH)\saves"

linux:
	rm -rf /path/to/folder/*
	cp -R assets/ build/
	cp -R videos/ build/
	$(COMPILER) $(SRC_FILES) $(AMARA_PATH) $(OTHER_LIB_PATHS) $(SDL_INCLUDE_PATHS_LINUX) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BUILD_EXECUTABLE_LINUX)
	mkdir $(BUILD_PATH)/saves

play:
	make playwin

playwin:
	$(BUILD_EXECUTABLE_WIN)

playlinux:
	./$(BUILD_EXECUTABLE_LINUX)

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
