SRC_FILES = src/main.cpp

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

LINKER_FLAGS_WIN64 = -Wl,-Bstatic -Wl,-Bdynamic -static-libstdc++ -static-libgcc -pthread $(SDL_LINKER_FLAGS_WIN64) -static

OTHER_LIB_PATHS = -I./src -Iresources/libs/nlohmann/include -Iresources/libs/murmurhash3 -Iresources/libs/lua -Iresources/libs/sol2

AMARA_PATH = -I ./resources/amara2

# INCLUDE_DEPTH = 1000
# EXTRA_OPTIONS = -fmax-include-depth=$(INCLUDE_DEPTH)

COMPILER_FLAGS = -w -Wall
# COMPILER_FLAGS = -w -Wl,-subsystem,windows

# Define required dependencies
DEPENDENCIES = $(COMPILER) 

# Detect package manager
PKG_MANAGER := $(shell if command -v apt-get >/dev/null 2>&1; then echo "apt-get"; \
	else if command -v dnf >/dev/null 2>&1; then echo "dnf"; \
	else if command -v pacman >/dev/null 2>&1; then echo "pacman"; \
	else echo "unknown"; fi; fi; fi)

# Check if dependencies are installed and install them if missing
linux_check_deps:
	@echo "Checking dependencies..."
	@missing=0; \
	for dep in $(DEPENDENCIES); do \
	    if ! command -v $$dep >/dev/null 2>&1; then \
	        echo "$$dep is not installed."; \
	        missing=1; \
	    fi; \
	done; \
	if [ $$missing -eq 1 ]; then \
	    echo "Installing missing dependencies..."; \
	    $(MAKE) install_deps; \
	fi

# Install missing dependencies based on detected package manager
linux_install_deps:
	@if [ "$(PKG_MANAGER)" = "apt-get" ]; then \
	    sudo apt-get update && sudo apt-get install -y $(DEPENDENCIES); \
	elif [ "$(PKG_MANAGER)" = "dnf" ]; then \
	    sudo dnf install -y $(DEPENDENCIES); \
	elif [ "$(PKG_MANAGER)" = "pacman" ]; then \
	    sudo pacman -Sy --noconfirm $(DEPENDENCIES); \
	else \
	    echo "Unsupported package manager. Install dependencies manually."; \
	    exit 1; \
	fi

# Compile the project (Example: main.c)
main: check_deps
	gcc -o main main.c

# Clean build files
clean:
	rm -f main


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
	$(COMPILER) $(SRC_FILES) $(AMARA_PATH) $(OTHER_LIB_PATHS) $(SDL_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLs

win_alt: $(SRC_FILES)
	$(COMPILER) $(SRC_FILES) $(AMARA_PATH) $(OTHER_LIB_PATHS) $(THEORA_WIN) $(SDL_PATHS_WIN64) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) $(LINKER_FLAGS_WIN64) -o $(BUILD_EXECUTABLE_WIN)
	make cpDLLsAlt

win32: $(SRC_FILES)
	make clearBuildWin
	cp -R assets/ build/
	cp -R videos/ build/
	$(COMPILER) $(SRC_FILES) -m32 $(AMARA_PATH) $(OTHER_LIB_PATHS) $(THEORA_WIN) $(SDL_INCLUDE_PATHS_WIN32) $(SDL_LIBRARY_PATHS_WIN32) $(COMPILER_FLAGS) $(EXTRA_OPTIONS) -lmingw32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -Wa,-mbig-obj $(LINKER_FLAGS) -I resources/libs/SDL2-32/bin -o $(BUILD_EXECUTABLE_WIN)
	xcopy /s /e /i /y "dlls\win62\*.*" "$(BUILD_PATH)/"
	md "$(BUILD_PATH)\saves"

linux:
	
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
