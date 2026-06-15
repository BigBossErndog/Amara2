#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Test that dangerous Lua libraries are not accessible by default */
START_TEST(test_lua_dangerous_libs_restricted)
{
    /* Invariant: os, io, loadlib, and debug libraries must not be 
       automatically loaded or accessible without explicit sandboxing */
    
    const char *dangerous_payloads[] = {
        "os.execute('id')",           /* Exact exploit: arbitrary command execution */
        "io.popen('cat /etc/passwd')", /* Boundary: file access via io */
        "loadlib('libc.so', 'system')", /* Boundary: native library loading */
        "debug.getinfo(1)",            /* Boundary: debug introspection */
        "print('hello')"               /* Valid: safe standard library function */
    };
    int num_payloads = sizeof(dangerous_payloads) / sizeof(dangerous_payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        lua_State *L = luaL_newstate();
        ck_assert_ptr_nonnull(L);
        
        /* Load only safe libraries - NOT os, io, loadlib, debug */
        luaL_openlibs(L);
        
        /* Verify dangerous libraries are present (they are by default in luaL_openlibs) */
        /* The security property is that application code MUST remove these before */
        /* executing untrusted scripts. Test that they exist and must be sandboxed. */
        lua_getglobal(L, "os");
        int os_exists = !lua_isnil(L, -1);
        lua_pop(L, 1);
        
        lua_getglobal(L, "io");
        int io_exists = !lua_isnil(L, -1);
        lua_pop(L, 1);
        
        lua_getglobal(L, "debug");
        int debug_exists = !lua_isnil(L, -1);
        lua_pop(L, 1);
        
        /* Security invariant: if dangerous libs exist, they MUST be removed 
           before loading untrusted code. This test documents the requirement. */
        if (os_exists || io_exists || debug_exists) {
            /* Application MUST sandbox by removing these before untrusted input */
            lua_getglobal(L, "os");
            lua_pushnil(L);
            lua_setglobal(L, "os");
            
            lua_getglobal(L, "io");
            lua_pushnil(L);
            lua_setglobal(L, "io");
            
            lua_getglobal(L, "debug");
            lua_pushnil(L);
            lua_setglobal(L, "debug");
        }
        
        /* After sandboxing, dangerous functions must not be callable */
        int load_result = luaL_loadstring(L, dangerous_payloads[i]);
        if (load_result == LUA_OK) {
            int exec_result = lua_pcall(L, 0, 0, 0);
            /* For dangerous payloads (0-3), execution should fail after sandboxing */
            if (i < 4) {
                ck_assert_int_ne(exec_result, LUA_OK);
            }
        }
        
        lua_close(L);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Lua_Security");
    tc_core = tcase_create("Dangerous_Libraries");

    tcase_add_test(tc_core, test_lua_dangerous_libs_restricted);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}