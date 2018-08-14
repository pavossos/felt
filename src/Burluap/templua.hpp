/*
  User Data With Pointer AUXiliary template.

  Mostly inspired by `User Data With Pointer Example' on the lua wiki,
  provides definitions for various almost-always-used functions.  The
  second argument is a trait that has only one method `name', that
  returns a string for the wrapped type.  Note that `alloc' does not
  allocate anything (we do not know if the wrapped type has a default
  constructor), just sets up the passed pointer on the lua stack.
*/

#ifndef TEMPLUA_HPP
#define TEMPLUA_HPP

#include <cassert>
#include <string>
#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>

#ifdef SHOWNEWDELETE
#include <cstdio>
#endif

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//----------------------------------------------------------------------!

// define missing macros, available in older lua versions
// EDIT: redefined as templates, in order to keep type information.

template<typename T>
void tl_boxpointer(lua_State *L, T p)
{
    T *pp = (T *) lua_newuserdata(L, sizeof(T *));
    *pp = p;
}

template<typename T>
T tl_unboxpointer(lua_State *L, int i)
{
    return *(T *)(lua_touserdata(L, i));
}

//----------------------------------------------------------------------!

// all wrapped classes should override the following

template<typename T>
std::string tl_metaname() 
{
    assert(false);
    return "NONE";
}

template<typename T>
int tl_tostring(lua_State *L)
{
    T p = tl_unboxpointer<T>(L, 1);
    lua_pushfstring(L, "[%s @ %p]", tl_metaname<T>().c_str(), p);
    return 1;
}

//----------------------------------------------------------------------!

// check boxed
  
template<typename T>
T tl_check_primitive(lua_State *L, int index, const std::string &name)
{
    luaL_checktype(L, index, LUA_TUSERDATA);
    T* v = (T*) luaL_checkudata(L, index, name.c_str());
    if (!v)
        luaL_typerror(L, index, name.c_str());
    T p = *v;
    if (!p)
        luaL_error(L, "NULL %s", name.c_str());
    return p;
}

template<typename T>
T tl_check(lua_State *L, int index) 
{
    const std::string name = tl_metaname<T>();
    // ATTN: better support for smart pointers
    //return tl_check_primitive<T>(L, index, name);
    return *tl_check_primitive<T*>(L, index, name);
}
    
template<>
unsigned tl_check<unsigned>(lua_State *L, int index)
{
    return (unsigned) lua_tointeger(L, index);
}

template<>
bool tl_check<bool>(lua_State *L, int index)
{
    return (bool) lua_tointeger(L, index);
}

template<>
double tl_check<double>(lua_State *L, int index)
{
    return (double) luaL_checknumber(L, index);
}

//----------------------------------------------------------------------!

// push boxed

template<typename T>
void tl_push(lua_State *L, T p) 
{
    const std::string name = tl_metaname<T>();
    // ATTN: better support for smart pointers
    //tl_boxpointer(L, p);
    tl_boxpointer<T*>(L, new T(p));
    luaL_getmetatable(L, name.c_str());
    lua_setmetatable(L, -2);
#ifdef SHOWNEWDELETE
    std::printf("NEW %s\n", name.c_str());
#endif
}

template<>
void tl_push(lua_State *L, int ii)
{
    lua_pushinteger(L, ii);
}

template<>
void tl_push(lua_State *L, unsigned val)
{
    lua_pushinteger(L, val);
}

template<>
void tl_push(lua_State *L, double val)
{
    lua_pushnumber(L, val);
}

template<>
void tl_push(lua_State *L, float val)
{
    lua_pushnumber(L, val);
}

template<>
void tl_push(lua_State *L, char val)
{
    lua_pushfstring(L,"%c", (int) val);
}

//----------------------------------------------------------------------!

// auxilliary function, needs improvement (also see tl_wrapper<> below)
    
template<typename T>
int tl_setup(lua_State *L, const luaL_reg *m, const luaL_reg *f) 
{
    const std::string name = tl_metaname<T>();
    luaL_newmetatable(L, name.c_str());
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    if (m)
        luaL_register(L, NULL, m);
    if (f)
        luaL_register(L, name.c_str(), f);
    return 1;
}

//----------------------------------------------------------------------!

// low-level arrays wrapper struct 

template<typename T>
struct tl_array
{
    T *data;
    size_t size;
};

// check low-level arrays

template<typename T>
T* tl_checkn(lua_State *L, int index, size_t &size)
{
    const std::string name = tl_metaname<T>() + "Array";
    luaL_checktype(L, index, LUA_TUSERDATA);
    tl_array<T> *obj = (tl_array<T> *) luaL_checkudata(L, index, name.c_str());
    if (!obj)
        luaL_typerror(L, index, name.c_str());
    size = obj->size;
    return obj->data;
}

// (possibly) collect low-level arrays 

template<typename T>
int tl_gcn(lua_State *L)
{
    size_t size;
    T *data = tl_checkn<T>(L, 1, size);
#ifdef SHOWNEWDELETE
    printf("DELETE[]: collecting array of %u elements\n", size);
#endif
    delete[] data;
}

// push low-level arrays

template<typename T>
void tl_pushn(lua_State *L, T *p, size_t size, bool gc = false)
{
    const std::string name = tl_metaname<T>() + "Array";
    tl_array<T> *obj = (tl_array<T> *) lua_newuserdata(L, sizeof(tl_array<T>));
    obj->data = p;
    obj->size = size;
    luaL_getmetatable(L, name.c_str());
    if (gc) {
        lua_pushliteral(L, "__gc");
        lua_pushcfunction(L, tl_gcn<T>);
        lua_settable(L, -3);
    }
    lua_setmetatable(L, -2);
}

//----------------------------------------------------------------------!

// arrays meta-functions

template<typename T>
int tl_array_tostring(lua_State *L)
{
    size_t size;
    T *data = tl_checkn<T>(L, 1, size);
    std::string mname = tl_metaname<T>();
    lua_pushfstring(L, "[Array of %s @ %p]", mname.c_str(), data);
    return 1;
}

template<typename T>
int tl_array_index(lua_State *L)
{
    size_t size;
    T *data = tl_checkn<T>(L, 1, size);
    int idx = luaL_checknumber(L, 2);
    if (idx > size || idx <= 0) {
        lua_pushnil(L);
        return 1;
    }
    tl_push<T>(L, data[idx-1]);
    return 1;
}

template<typename T>
int tl_array_newindex(lua_State *L)
{
    size_t size;
    T *data = tl_checkn<T>(L, 1, size);
    int idx = luaL_checknumber(L, 2);
    T val = tl_check<T>(L, 3);
    if (idx > size || idx <= 0) {
        return luaL_argerror(L, 3, "out of bounds");
    }
    data[idx-1] = val;
    return 0;
}

template<typename T>
int tl_array_size(lua_State *L)
{
    size_t size;
    T *data = tl_checkn<T>(L, 1, size);
    lua_pushnumber(L, size);
    return 1;
}

//----------------------------------------------------------------------!

// wrap arrays in one line.

template<typename T>
struct tl_array_wrapper
{
    typedef std::map<std::string, lua_CFunction> metamap;

    tl_array_wrapper(bool readonlyp = false)
        {
            name_ = tl_metaname<T>() + "Array";
            meta_["__len"] = tl_array_size<T>;
            meta_["__index"] = tl_array_index<T>;
            if (!readonlyp)
                meta_["__newindex"] = tl_array_newindex<T>;
            meta_["__tostring"] = tl_array_tostring<T>;
        }

    void registerm(lua_State *L) const
        {
            const size_t n = meta_.size();
            luaL_reg *meta = new luaL_reg[n+1];
            size_t i = 0;
            metamap::const_iterator it;
            for (it = meta_.begin(); it != meta_.end(); ++it) {
                meta[i].name = it->first.c_str();
                meta[i].func = it->second;
                i++;
            }
            meta[i].name = NULL;
            meta[i].func = NULL;
            luaL_newmetatable(L, name_.c_str());
            luaL_register(L, NULL, meta);
        }

    metamap meta_;
    std::string name_;
};

//----------------------------------------------------------------------!

// properties helper, use for __index & __newindex

// super-loaded __index metamethods: supports numeric indices for
// array-like operation, properties for easy access to get_* methods
// and if these fail, normal lookup in metatable.
template<typename T>
int tl_index_wprop(lua_State *L)
{
    const std::string name = tl_metaname<T>();

    // if numeric, check __numeric_index in metatable
    if (LUA_TNUMBER == lua_type(L, 2)) {
        lua_getmetatable(L, 1);
        lua_pushliteral(L, "__numeric_index");
        lua_rawget(L, -2);
        if (!lua_isnil(L, -1)) {
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 2);
            lua_call(L, 2, 1);
            return 1;
        }
        lua_pop(L, 1);
    }
    
    // check if key actually exists in metatable
    luaL_getmetatable(L, name.c_str());
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1))
        return 1;
    lua_pop(L, 1);

    // check if getter exists in metatable
    const char *key = luaL_checkstring(L, 2);
    lua_pushfstring(L, "get_%s", key);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
        return 1;
    }

    // nothing found
    lua_pushnil(L);
    return 1;
}

template<typename T>
int tl_newindex_wprop(lua_State *L)
{
    const std::string name = tl_metaname<T>();
            
    // check if key actually exists in metatable
    luaL_getmetatable(L, name.c_str());
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1))
        return 1;
    lua_pop(L, 1);
    
    // check if setter exists in metatable
    const char *key = luaL_checkstring(L, 2);
    lua_pushfstring(L, "set_%s", key);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 0);
        return 0;
    }
    
    // nothing found, signal an error
    luaL_error(L, "%s: property not found", key);
    return 0;
}

//----------------------------------------------------------------------!

// experimental functions

template<typename T>
int tl_gc(lua_State *L) 
{
    // ATTN: better support for smart pointers
    delete tl_unboxpointer<T*>(L, 1);
    return 0;
}

// alt version, also works on non-POD types
template<typename S, typename V, V S::*pp>
int tl_getter(lua_State *L)
{
    typedef S* T;
    T p = tl_check<T>(L, 1);
    tl_push<V>(L, p->*pp);
    return 1;
}

// shared_ptr version
template<typename S, typename V, V S::*pp>
int tl_getter_shared(lua_State *L)
{
    typedef boost::shared_ptr<S> T;
    T p = tl_check<T>(L, 1);
    tl_push<V>(L, p.get()->*pp);
    return 1;
}

// alt version, also works on non-POD types
template<typename S, typename V, V S::*pp>
int tl_setter(lua_State *L)
{
    typedef S* T;
    T p = tl_check<T>(L, 1);
    V nu = tl_check<V>(L, 2);
    p->*pp = nu;
    return 0;
}

// shared_ptr version
template<typename S, typename V, V S::*pp>
int tl_setter_shared(lua_State *L)
{
    typedef boost::shared_ptr<S> T;
    T p = tl_check<T>(L, 1);
    V nu = tl_check<V>(L, 2);
    p.get()->*pp = nu;
    return 0;
}

template<typename S, typename V, V S::*pp>
std::pair<lua_CFunction, lua_CFunction> tl_getset()
{
    return
        std::make_pair(tl_getter<S, V, pp>,
                       tl_setter<S, V, pp>);
}

// shared_ptr version
template<typename S, typename V, V S::*pp>
std::pair<lua_CFunction, lua_CFunction> tl_getset_shared()
{
    return
        std::make_pair(tl_getter_shared<S, V, pp>,
                       tl_setter_shared<S, V, pp>);
}

//----------------------------------------------------------------------!

// wrap structs easier

template<typename T>
struct tl_wrapper
{
    typedef std::map<std::string, lua_CFunction> metamap;
    
    tl_wrapper(bool wprops = true)
        {
            name_ = tl_metaname<T>();
            meta_["__tostring"] = tl_tostring<T>;
            meta_["__gc"] = tl_gc<T>;
            
            if (wprops) {
                meta_["__index"] = tl_index_wprop<T>;
                meta_["__newindex"] = tl_newindex_wprop<T>;
            }
        }

    lua_CFunction& operator[](const metamap::key_type &key)
        {
            return meta_[key];
        }

    tl_wrapper& prop(const std::string &prop, lua_CFunction getter)
        {
            std::string getk = "get_" + prop;
            meta_[getk] = getter;
            return *this;
        }

    tl_wrapper& prop(const std::string &prop, lua_CFunction getter, lua_CFunction setter)
        {
            std::string getk = "get_" + prop;
            meta_[getk] = getter;
            std::string setk = "set_" + prop;
            meta_[setk] = setter;
            return *this;
        }
    
    tl_wrapper& prop(const char *prop, const std::pair<lua_CFunction, lua_CFunction> &funs)
        {
            std::string getk = std::string("get_") + prop;
            meta_[getk] = funs.first;
            std::string setk = std::string("set_") + prop;
            meta_[setk] = funs.second;
            return *this;
        }
    
    void registerm(lua_State *L) const
        {
            const size_t n = meta_.size();
            luaL_reg *meta = new luaL_reg[n+1];
            size_t i = 0;
            metamap::const_iterator it;
            for (it = meta_.begin(); it != meta_.end(); ++it) {
                meta[i].name = it->first.c_str();
                meta[i].func = it->second;
                i++;
            }
            meta[i].name = NULL;
            meta[i].func = NULL;
            tl_setup<T>(L, meta, NULL);
        }

    metamap meta_;
    std::string name_;
};
    
//----------------------------------------------------------------------!

#endif
