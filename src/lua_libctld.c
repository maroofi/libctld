#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


#include <libctld.h>


/*
compile with: 
gcc -c -Wall -Werror -shared -fpic -Wl,-E src/lua_libctld.c src/libctld.c src/cstrlib.c src/clist.c src/cdict.c -I./include -I/usr/include/lua5.4 && gcc -shared -o libctld.so *.o -llua5.4 -lm -lidn2 && rm -f *.o
*/
/*
example in lua:
psl = require "libctld"
inspect = require "inspect"
p = psl.init("psl.dat")
result, err, msg = psl.parse(p, "google.com", 0)

result is:
{
  domain = "google",
  registered_domain = "google.com",
  suffix = "com"
}

*/

static int l_parse(lua_State * L){
    // call it in lua like parse(ctx, fqdn_string, use_private=0|1)
    size_t len = 0;
    uint16_t use_private = luaL_checkinteger(L, -1);
    if (use_private != 1 && use_private != 0){
        lua_pushnil(L);
        lua_pushinteger(L, 4);
        lua_pushstring(L, "use_private param must be either 0 or 1");
        return 3;
    }
    const char * data = luaL_checklstring(L, -2, &len);
    if (NULL == data){
        lua_pushnil(L);
        lua_pushinteger(L, 1);
        lua_pushstring(L, "You must pass a FQDN to parse it to more detail");
        return 3;
    }
    
    ctld_ctx ** ctx = (ctld_ctx **)luaL_checkudata(L, -3, "metactldlib");
    if (*ctx == NULL){
        lua_pushnil(L);
        lua_pushinteger(L, 3);
        lua_pushstring(L, "Can not get the context");
        return 3;
    }
    char * str = (char*)malloc(len+1);
    if (NULL == str){
        lua_pushnil(L);
        lua_pushinteger(L, 5);
        lua_pushstring(L, "Can not allocate memory");
        return 3;
    }
    memcpy(str, data, len);
    str[len] = '\0';
    int t = len;
    while (t >= 0){
        if (str[t-1] == 0x0D || str[t-1] == 0x20 || str[t-1] == 0x0A || str[t-1] == '.'){
            str[t-1] = '\0';
            t -= 1;
        }else{
            break;
        }
    }
    if (str[0] == '\0'){
        free(str);
        lua_pushnil(L);
        lua_pushinteger(L, 6);
        lua_pushstring(L, "Error parsing the given string");
        return 3;
    }
    


    ctld_result * res = ctld_parse(*ctx, str, use_private);
    if (NULL == res){
        free(str);
        lua_pushnil(L);
        lua_pushinteger(L, 6);
        lua_pushstring(L, "Error parsing the given string");
        return 3;
    }
    // we want to return a table with four values
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "suffix");
    if (res->suffix != NULL){
        lua_pushstring(L, res->suffix);
    }else{
        lua_pushnil(L);
    }
    lua_settable(L, -3);
    lua_pushstring(L, "fqdn");
    if (res->fqdn != NULL){
        lua_pushstring(L, res->fqdn);
    }else{
        lua_pushnil(L);
    }
    lua_settable(L, -3);
    lua_pushstring(L, "registered_domain");
    if (res->registered_domain != NULL){
        lua_pushstring(L, res->registered_domain);
    }else{
        lua_pushnil(L);
    }
    lua_settable(L, -3);
    lua_pushstring(L, "domain");
    if (res->domain != NULL){
        lua_pushstring(L, res->domain);
    }else{
        lua_pushnil(L);
    }
    lua_settable(L, -3);
    return 1;
}

static int l_ctld_init(lua_State * L){
    // this function init the context
    // by accepting the psl.dat file
    const char * filename = luaL_checkstring(L, -1);
    if (NULL == filename){
        lua_pushnil(L);
        lua_pushinteger(L, 1);
        lua_pushstring(L, "You need to pass the absolute path to psl file (psl.dat)");
        return 0;
    }
    ctld_ctx ** ctx;
    ctx = lua_newuserdata(L, sizeof(ctld_ctx *));
    *ctx = ctld_parse_file((char*)filename);
    if (*ctx == NULL){
        lua_pushnil(L);
        lua_pushinteger(L, 2);
        lua_pushstring(L, "Can not initialize the library....");
        return 3;
    }
    luaL_getmetatable(L, "metactldlib");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_ctld_free_context(lua_State * L){
    ctld_ctx ** ctx = (ctld_ctx **)lua_touserdata(L, -1);
    ctld_free(*ctx);
    *ctx = NULL;
    return 0;
}

static const struct luaL_Reg ctld_lib_expose[] = {
    {"init", l_ctld_init},
    {"parse", l_parse},
    {NULL, NULL}
};

int luaopen_libctld(lua_State * L){
    luaL_newmetatable(L, "metactldlib");
    lua_pushcfunction(L, l_ctld_free_context);
    lua_setfield(L, -2, "__gc");
    luaL_newlib(L, ctld_lib_expose);
    return 1;
}
