// ---------------------------------------------------------------------------
// C:/Users/db348/art2d/trunk/wxArt2D/packages/wxart2d/apps/teto/src/teto_bind.cpp was generated by genwxbind.lua 
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
// wxLuaGetEventList_teto() is called to register events
// ---------------------------------------------------------------------------

wxLuaBindEvent* wxLuaGetEventList_teto(size_t &count)
{
    static wxLuaBindEvent eventList[] =
    {

        { 0, 0, 0 },
    };
    count = sizeof(eventList)/sizeof(wxLuaBindEvent) - 1;
    return eventList;
}

// ---------------------------------------------------------------------------
// wxLuaGetDefineList_teto() is called to register #define and enum
// ---------------------------------------------------------------------------

wxLuaBindNumber* wxLuaGetDefineList_teto(size_t &count)
{
    static wxLuaBindNumber numberList[] =
    {

        { 0, 0 },
    };
    count = sizeof(numberList)/sizeof(wxLuaBindNumber) - 1;
    return numberList;
}

// ---------------------------------------------------------------------------

// wxLuaGetStringList_teto() is called to register #define_string
// ---------------------------------------------------------------------------

wxLuaBindString* wxLuaGetStringList_teto(size_t &count)
{
    static wxLuaBindString stringList[] =
    {
        { "a2dTETO_VERSION", a2dTETO_VERSION, NULL },

        { 0, 0 },
    };
    count = sizeof(stringList)/sizeof(wxLuaBindString) - 1;
    return stringList;
}

// ---------------------------------------------------------------------------
// wxLuaGetObjectList_teto() is called to register object and pointer bindings
// ---------------------------------------------------------------------------

wxLuaBindObject* wxLuaGetObjectList_teto(size_t &count)
{
    static wxLuaBindObject objectList[] =
    {

        { 0, 0, 0, 0 },
    };
    count = sizeof(objectList)/sizeof(wxLuaBindObject) - 1;
    return objectList;
}

// ---------------------------------------------------------------------------
// wxLuaGetFunctionList_teto() is called to register global functions
// ---------------------------------------------------------------------------

// %function TetoLuaProcCommandProcessor* a2dGetGedi()
static int LUACALL wxLua_function_a2dGetGedi(lua_State *L)
{
    // call a2dGetGedi
    TetoLuaProcCommandProcessor* returns = (TetoLuaProcCommandProcessor*)a2dGetGedi();
    // push the result datatype
    wxluaT_pushuserdatatype(L, returns, wxluatype_TetoLuaProcCommandProcessor);

    return 1;
}
static wxLuaBindCFunc s_wxluafunc_wxLua_function_a2dGetGedi[1] = {{ wxLua_function_a2dGetGedi, WXLUAMETHOD_CFUNCTION, 0, 0, g_wxluaargtypeArray_None }};

// ---------------------------------------------------------------------------
// wxLuaGetFunctionList_teto() is called to register global functions
// ---------------------------------------------------------------------------

wxLuaBindMethod* wxLuaGetFunctionList_teto(size_t &count)
{
    static wxLuaBindMethod functionList[] =
    {
        { "a2dGetGedi", WXLUAMETHOD_CFUNCTION, s_wxluafunc_wxLua_function_a2dGetGedi, 1, NULL },

        { 0, 0, 0, 0 }, 
    };
    count = sizeof(functionList)/sizeof(wxLuaBindMethod) - 1;
    return functionList;
}



// ---------------------------------------------------------------------------
// wxLuaGetClassList_teto() is called to register classes
// ---------------------------------------------------------------------------

static const char* wxluaclassname_TetoLuaProcCommandProcessor = "TetoLuaProcCommandProcessor";
static const char* wxluaclassname_a2dLuaCentralCommandProcessor = "a2dLuaCentralCommandProcessor";

static const char* wxluabaseclassnames_TetoLuaProcCommandProcessor[] = { wxluaclassname_a2dLuaCentralCommandProcessor, NULL };
static wxLuaBindClass* wxluabaseclassbinds_TetoLuaProcCommandProcessor[] = { NULL };
// ---------------------------------------------------------------------------
// Lua Tag Method Values and Tables for each Class
// ---------------------------------------------------------------------------

extern wxLuaBindMethod TetoLuaProcCommandProcessor_methods[];
extern int TetoLuaProcCommandProcessor_methodCount;
extern void wxLua_TetoLuaProcCommandProcessor_delete_function(void** p);




wxLuaBindClass* wxLuaGetClassList_teto(size_t &count)
{
    static wxLuaBindClass classList[] =
    {
        { wxluaclassname_TetoLuaProcCommandProcessor, TetoLuaProcCommandProcessor_methods, TetoLuaProcCommandProcessor_methodCount, CLASSINFO(TetoLuaProcCommandProcessor), &wxluatype_TetoLuaProcCommandProcessor, wxluabaseclassnames_TetoLuaProcCommandProcessor, wxluabaseclassbinds_TetoLuaProcCommandProcessor, NULL, NULL, NULL, 0, &wxLua_TetoLuaProcCommandProcessor_delete_function, }, 

        { 0, 0, 0, 0, 0, 0, 0 }, 
    };
    count = sizeof(classList)/sizeof(wxLuaBindClass) - 1;

    return classList;
}

// ---------------------------------------------------------------------------
// wxLuaBinding_teto() - the binding class
// ---------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxLuaBinding_teto, wxLuaBinding)

wxLuaBinding_teto::wxLuaBinding_teto() : wxLuaBinding()
{
    m_bindingName   = wxT("teto");
    m_nameSpace     = wxT("wx");
    m_classArray    = wxLuaGetClassList_teto(m_classCount);
    m_numberArray   = wxLuaGetDefineList_teto(m_numberCount);
    m_stringArray   = wxLuaGetStringList_teto(m_stringCount);
    m_eventArray    = wxLuaGetEventList_teto(m_eventCount);
    m_objectArray   = wxLuaGetObjectList_teto(m_objectCount);
    m_functionArray = wxLuaGetFunctionList_teto(m_functionCount);
    InitBinding();
}



// ---------------------------------------------------------------------------

wxLuaBinding* wxLuaBinding_teto_init()
{
    static wxLuaBinding_teto m_binding;

    if (wxLuaBinding::GetBindingArray().Index(&m_binding) == wxNOT_FOUND)
        wxLuaBinding::GetBindingArray().Add(&m_binding);

    return &m_binding;
}


