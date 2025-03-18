#ifndef LUA_SINGLE_H
#define LUA_SINGLE_H

#define LUA_IMPLEMENTATION  // Ensures function bodies are included

// Define LUA_API as static to prevent linker errors
#ifndef LUA_API
#define LUA_API static
#endif

// Ensure internal macros are defined
#define LUA_CORE
#define LUA_LIB

// Include Lua headers (declarations)
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

// Include all Lua core source files
#include "lapi.c"
#include "lcode.c"
#include "lctype.c"
#include "ldebug.c"
#include "ldo.c"
#include "ldump.c"
#include "lfunc.c"
#include "lgc.c"
#include "llex.c"
#include "lmem.c"
#include "lobject.c"
#include "lopcodes.c"
#include "lparser.c"
#include "lstate.c"
#include "lstring.c"
#include "ltable.c"
#include "ltm.c"
#include "lundump.c"
#include "lvm.c"
#include "lzio.c"

// Include Lua standard libraries
#include "lauxlib.c"
#include "lbaselib.c"  // Base functions (includes `luaopen_coroutine`)
#include "lcorolib.c"  // Coroutine functions (defines `luaopen_coroutine`)
#include "ldblib.c"
#include "liolib.c"
#include "lmathlib.c"
#include "loslib.c"
#include "ltablib.c"
#include "lstrlib.c"
#include "lutf8lib.c"  // UTF-8 functions (defines `luaopen_utf8`)
#include "loadlib.c"
#include "linit.c"

#endif // LUA_SINGLE_H

#ifdef next
#undef next
#endif
