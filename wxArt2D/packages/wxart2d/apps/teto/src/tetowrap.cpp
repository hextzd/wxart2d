// ---------------------------------------------------------------------------
// C:/Users/db348/art2d/trunk/wxArt2D/packages/wxart2d/apps/teto/src/tetowrap.cpp was generated by genwxbind.lua 
//
// Any changes made to this file will be lost when the file is regenerated.
// ---------------------------------------------------------------------------

#include "a2dprec.h"

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
     #include "wx/wx.h"
#endif

#include "wxlua/wxlstate.h"
#include "tetowrap.h"

#ifdef __GNUC__
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif // __GNUC__

// ---------------------------------------------------------------------------
// Bind class TetoLuaProcCommandProcessor
// ---------------------------------------------------------------------------

// Lua MetaTable Tag for Class 'TetoLuaProcCommandProcessor'
int wxluatype_TetoLuaProcCommandProcessor = WXLUA_TUNKNOWN;

static wxLuaArgType s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_ShowDlg[] = { &wxluatype_TetoLuaProcCommandProcessor, &wxluatype_a2dCommandId, &wxluatype_TBOOLEAN, &wxluatype_TBOOLEAN, NULL };
static int LUACALL wxLua_TetoLuaProcCommandProcessor_ShowDlg(lua_State *L);
static wxLuaBindCFunc s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_ShowDlg[1] = {{ wxLua_TetoLuaProcCommandProcessor_ShowDlg, WXLUAMETHOD_METHOD, 2, 4, s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_ShowDlg }};
//     bool ShowDlg( const a2dCommandId* comID, bool modal = false, bool onTop = true )
static int LUACALL wxLua_TetoLuaProcCommandProcessor_ShowDlg(lua_State *L)
{
    // get number of arguments
    int argCount = lua_gettop(L);
    // bool onTop = true
    bool onTop = (argCount >= 4 ? wxlua_getbooleantype(L, 4) : true);
    // bool modal = false
    bool modal = (argCount >= 3 ? wxlua_getbooleantype(L, 3) : false);
    // const a2dCommandId comID
    const a2dCommandId * comID = (const a2dCommandId *)wxluaT_getuserdatatype(L, 2, wxluatype_a2dCommandId);
    // get this
    TetoLuaProcCommandProcessor * self = (TetoLuaProcCommandProcessor *)wxluaT_getuserdatatype(L, 1, wxluatype_TetoLuaProcCommandProcessor);
    // call ShowDlg
    bool returns = (self->ShowDlg(comID, modal, onTop));
    // push the result flag
    lua_pushboolean(L, returns);

    return 1;
}

static wxLuaArgType s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_constructor[] = { &wxluatype_TNUMBER, &wxluatype_TBOOLEAN, &wxluatype_TNUMBER, NULL };
static int LUACALL wxLua_TetoLuaProcCommandProcessor_constructor(lua_State *L);
static wxLuaBindCFunc s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_constructor[1] = {{ wxLua_TetoLuaProcCommandProcessor_constructor, WXLUAMETHOD_CONSTRUCTOR, 0, 3, s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_constructor }};
//     TetoLuaProcCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true, int maxCommands = -1 )
static int LUACALL wxLua_TetoLuaProcCommandProcessor_constructor(lua_State *L)
{
    // get number of arguments
    int argCount = lua_gettop(L);
    // int maxCommands = -1
    int maxCommands = (argCount >= 3 ? (int)wxlua_getnumbertype(L, 3) : -1);
    // bool initialize = true
    bool initialize = (argCount >= 2 ? wxlua_getbooleantype(L, 2) : true);
    // long flags = a2dDEFAULT_DOCUMENT_FLAGS
    long flags = (argCount >= 1 ? (long)wxlua_getnumbertype(L, 1) : a2dDEFAULT_DOCUMENT_FLAGS);
    // call constructor
    TetoLuaProcCommandProcessor* returns = new TetoLuaProcCommandProcessor(flags, initialize, maxCommands);
    // add to tracked memory list
    wxluaO_addgcobject(L, returns, wxluatype_TetoLuaProcCommandProcessor);
    // push the constructed class pointer
    wxluaT_pushuserdatatype(L, returns, wxluatype_TetoLuaProcCommandProcessor);

    return 1;
}

static wxLuaArgType s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_delete[] = { &wxluatype_TetoLuaProcCommandProcessor, NULL };
static wxLuaBindCFunc s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_delete[1] = {{ wxlua_userdata_delete, WXLUAMETHOD_METHOD|WXLUAMETHOD_DELETE, 1, 1, s_wxluatypeArray_wxLua_TetoLuaProcCommandProcessor_delete }};



void wxLua_TetoLuaProcCommandProcessor_delete_function(void** p)
{
    TetoLuaProcCommandProcessor* o = (TetoLuaProcCommandProcessor*)(*p);
    delete o;
}

// Map Lua Class Methods to C Binding Functions
wxLuaBindMethod TetoLuaProcCommandProcessor_methods[] = {
    { "ShowDlg", WXLUAMETHOD_METHOD, s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_ShowDlg, 1, NULL },
    { "TetoLuaProcCommandProcessor", WXLUAMETHOD_CONSTRUCTOR, s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_constructor, 1, NULL },
    { "delete", WXLUAMETHOD_METHOD|WXLUAMETHOD_DELETE, s_wxluafunc_wxLua_TetoLuaProcCommandProcessor_delete, 1, NULL },
    { 0, 0, 0, 0 },
};

int TetoLuaProcCommandProcessor_methodCount = sizeof(TetoLuaProcCommandProcessor_methods)/sizeof(wxLuaBindMethod) - 1;

