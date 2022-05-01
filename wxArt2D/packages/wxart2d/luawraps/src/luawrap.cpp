/*! \file luawrap/src/luawrap.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: luawrap.cpp,v 1.64 2009/09/26 19:01:08 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/luawraps/luawrap.h"
#include "wx/canvas/objlist.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/drawer.h"
#include "wx/editor/editmod.h"

#include "wxbind/include/wxbinddefs.h"
#include "wxbind/include/wxcore_bind.h"
#include "wxlua/wxlstate.h"
#include "wxlua/wxlbind.h"

#include "wx/luawraps/luabind.h"

//----------------------------------------------------------------------------
// a2dCanvasObjectLua
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasObjectLua, a2dCanvasObject )

BEGIN_EVENT_TABLE( a2dCanvasObjectLua, a2dCanvasObject )
END_EVENT_TABLE()

a2dCanvasObjectLua::a2dCanvasObjectLua( double x, double y, const wxString& script, const wxString& function ): a2dCanvasObject()
{
    m_function = function;
    m_script = script;
    m_lworld.Translate( x, y );
    m_xoffset = 0;
    m_yoffset = 0;
    m_run = false;
}

a2dCanvasObjectLua::~a2dCanvasObjectLua()
{
}

a2dCanvasObjectLua::a2dCanvasObjectLua( const a2dCanvasObjectLua& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
}

a2dObject* a2dCanvasObjectLua::DoClone( CloneOptions options, a2dRefMap* refs ) const

{
    return new a2dCanvasObjectLua( *this, options, refs );
}

void a2dCanvasObjectLua::SetField( wxLuaState lst, const wxString& name, const wxString& value )
{
    lst.lua_PushString( name );
    lst.lua_PushString( value );
    lst.lua_SetTable( -3 );
}

void a2dCanvasObjectLua::SetField( wxLuaState lst, const wxString& name, double value )
{
    lst.lua_PushString( name );
    lst.lua_PushNumber( value );

    lst.lua_SetTable( -3 );
}

void a2dCanvasObjectLua::SetField( wxLuaState lst, const wxString& name, long value )
{
    lst.lua_PushString( name );
    lst.lua_PushInteger( value );
    lst.lua_SetTable( -3 );
}

void a2dCanvasObjectLua::SetField( wxLuaState lst, const wxString& name, bool value )
{
    lst.lua_PushString( name );
    lst.lua_PushBoolean( value );
    lst.lua_SetTable( -3 );
}

void a2dCanvasObjectLua::SetField( wxLuaState lst, const wxString& name, wxColour value )
{
    lst.lua_PushString( name );

    wxColour* returns = new wxColour( value );
    // add the new object to the tracked memory list
    wxluaO_addgcobject( lst.GetLuaState(), returns, wxluatype_TSTRING );
    // push the result datatype
    wxluaT_pushuserdatatype( lst.GetLuaState(), returns, wxluatype_wxColour );
    lst.lua_SetTable( -3 );
}

bool a2dCanvasObjectLua::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_run )
    {
        m_run = true;

        wxLuaState lst = a2dLuaWP->GetLuaState();
        lua_State* L = lst.GetLuaState();
        if ( 0 != lst.RunString( m_script ) )
        {
            wxLogWarning( _( "Error in Lua Script" ) );
        }
        else
        {
            lua_getglobal( L, wx2lua( m_function ) );
            lst.wxluaT_PushUserDataType( this, wxluatype_a2dCanvasObject, true );

            lst.lua_PushNumber( m_xoffset );
            lst.lua_PushNumber( m_yoffset );

            lst.lua_NewTable();

            a2dNamedPropertyList allprops;
            CollectProperties2( &allprops, NULL );
            a2dNamedPropertyList::const_iterator iter;
            for( iter = allprops.begin(); iter != allprops.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                if ( wxDynamicCast( prop, a2dInt32Property ) != 0 )
                    SetField( lst, prop->GetName(), ( long ) prop->GetInt32() );
                else if ( wxDynamicCast( prop, a2dInt16Property ) != 0 )
                    SetField( lst, prop->GetName(), ( long ) prop->GetInt16() );
                else if ( wxDynamicCast( prop, a2dUint32Property ) != 0 )
                    SetField( lst, prop->GetName(), ( long ) prop->GetUint32() );
                else if ( wxDynamicCast( prop, a2dUint16Property ) != 0 )
                    SetField( lst, prop->GetName(), ( long ) prop->GetUint16() );
                else if ( wxDynamicCast( prop, a2dDoubleProperty ) != 0 )
                    SetField( lst, prop->GetName(), prop->GetDouble() );
                else if ( wxDynamicCast( prop, a2dFloatProperty ) != 0 )
                    SetField( lst, prop->GetName(), prop->GetFloat() );
                else if ( wxDynamicCast( prop, a2dBoolProperty ) != 0 )
                    SetField( lst, prop->GetName(), prop->GetBool() );
                else if ( wxDynamicCast( prop, a2dStringProperty ) != 0 )
                    SetField( lst, prop->GetName(), prop->GetString() );
                else if ( wxDynamicCast( prop, a2dColourProperty ) != 0 )
                {
                    a2dColourProperty* p = wxStaticCast( prop, a2dColourProperty );
                    SetField( lst, prop->GetName(), p->GetColour() );
                }
            }

            //SetField( lst, wxT("maxY"), 800.0 );
            //SetField( lst, wxT("maxX"), 70 );

            //if ( lst.LuaPCall( 4, 0 ) != 0 )
            if ( lst.lua_PCall( 4, 0, 0 ) != 0 )
            {
                wxString error;
                error.Printf( wxT( "%s %s" ), m_function.c_str() , lua_tostring( L, -1 ) );
                wxLuaEvent event( wxEVT_LUA_ERROR, lst.GetId(), lst );
                event.SetString( error );
                lst.SendEvent( event );
                lua_pop( L, 1 );
            }
            SetRoot( m_root );
            //childbox.SetValid(false);
        }
        return true;
    }
    return false;
}

void a2dCanvasObjectLua::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
}

#if wxART2D_USE_CVGIO
void a2dCanvasObjectLua::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dCanvasObjectLua::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dLuaCentralCommandProcessor
//----------------------------------------------------------------------------

a2dLuaCentralCommandProcessor* a2dLuaWP;

a2dLuaCentralCommandProcessor* a2dGetLuaWP()
{
    return a2dLuaWP;
}

bool a2dLuaCentralCommandProcessor::IsShowna2dLuaExecDlg()
{
    return m_luaExecDlg != NULL && m_luaExecDlg->IsShown();
}

IMPLEMENT_CLASS( a2dLuaCentralCommandProcessor, a2dCentralCanvasCommandProcessor )

const a2dCommandId a2dLuaCentralCommandProcessor::COMID_ShowLuaExecDlg( wxT( "ShowLuaExecDlg" ) );

a2dLuaCentralCommandProcessor::a2dLuaCentralCommandProcessor( long flags , bool initialize, int maxCommands )
    : a2dCentralCanvasCommandProcessor( flags, initialize, maxCommands )
{
    //m_listOfIdforAddPrims.push_back( ( a2dPropertyId* ) &a2dCanvasObject::PROPID_Layer );

    // intitialize the global commandprocessor, must be done just before the interpreter is initialized,
    // since it uses this pointer.
    a2dLuaWP = this;

    m_interp = wxLuaState( wxTheApp, wxID_ANY );

    m_interp.SetEventHandler( wxTheApp );

    m_luaExecDlg = NULL;

    //RunBuffer(plotto_lua, plotto_lua_len-1, wxT("plotto.lua"));
}

a2dLuaCentralCommandProcessor::~a2dLuaCentralCommandProcessor()
{
    if ( m_luaExecDlg )
        m_luaExecDlg->Destroy();

    m_luaExecDlg = NULL;

    m_interp.CloseLuaState( true );
    m_interp.Destroy();

}

void a2dLuaCentralCommandProcessor::OnExit()
{
    if ( m_luaExecDlg )
        m_luaExecDlg->Destroy();

    m_luaExecDlg = NULL;
    a2dCentralCanvasCommandProcessor::OnExit();
}

bool a2dLuaCentralCommandProcessor::ExecuteF( bool withUndo, wxChar* Format, ... )
{
    va_list ap;

    wxString commands;
    va_start( ap, Format );

    commands.PrintfV( Format, ap );
    va_end( ap );

    return Execute( commands, withUndo );
}


bool a2dLuaCentralCommandProcessor::Execute( const wxString& commandsString, bool withUndo )
{
    bool oldlog = a2dDocviewGlobals->GetDoLog();
    if ( oldlog )
        a2dDocviewGlobals->SetDoLog( false );

    a2dDocviewGlobals->SetDoLog( oldlog );
    // Run a string that contains lua code
    if ( 0 != m_interp.RunString( commandsString ) )
    {
        if ( !a2dDocviewGlobals->GetErrors().empty() )
            wxLogWarning( _( "Error in Lua Command: %s \n" ), commandsString.c_str() );

        //a2dDocviewGlobals->RecordF( NULL, wxT( "%s" ), commandsString.c_str() );
    }

    a2dDocviewGlobals->SetDoLog( oldlog );
    return true;
}

bool a2dLuaCentralCommandProcessor::ExecuteFile( const wxString& fileName )
{
    bool oldlog = a2dDocviewGlobals->GetDoLog();
//   if ( oldlog )
//        a2dDocviewGlobals->SetDoLog( false );

    // Run a file that contains lua code
    if ( 0 != m_interp.RunFile( fileName ) )
    {
        // if empty still show some error, since it went wrong somehow.
        if ( a2dDocviewGlobals->GetErrors().empty() )
            wxLogWarning( _( "Error in Lua File: %s \n" ), fileName.c_str() );
    }

    a2dDocviewGlobals->SetDoLog( oldlog );
    return true;
}

a2dLuaEditorFrame* a2dLuaCentralCommandProcessor::GetActiveEditorFrame()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        if ( drawer->GetDisplayWindow() )
        {
            wxWindow* frame = drawer->GetDisplayWindow()->GetParent();
            a2dLuaEditorFrame* frameEditor = wxDynamicCast( frame, a2dLuaEditorFrame );
            if ( frameEditor )
                return frameEditor;
        }
    }
    return NULL;
}

bool a2dLuaCentralCommandProcessor::ShowLuaExecDlg()
{
    if ( !m_luaExecDlg )
        m_luaExecDlg = new a2dLuaExecDlg( this, NULL );

    if ( m_luaExecDlg->IsShown() )
    {
        m_luaExecDlg->Show( false );
    }
    else
    {
        m_luaExecDlg->Show( true );
    }
    return true;
}



bool a2dLuaCentralCommandProcessor::ShowDlg( const a2dCommandId* comID, bool modal, bool onTop )
{
    if ( comID == &a2dLuaCentralCommandProcessor::COMID_ShowLuaExecDlg )
    {
        if ( !m_luaExecDlg )
            m_luaExecDlg = new a2dLuaExecDlg( this, NULL );

        if ( m_luaExecDlg->IsShown() )
        {
            m_luaExecDlg->Show( false );
        }
        else
        {
            m_luaExecDlg->Show( true );
        }
        return true;
    }
    else
    {
        return a2dCentralCanvasCommandProcessor::ShowDlg( comID, modal, onTop );
    }
}

bool a2dLuaCentralCommandProcessor::FileNew()
{
    a2dDocumentPtr doc;
    a2dError error = a2dDocumentCommandProcessor::FileNew( doc );
    return error == a2dError_NoError;
}

a2dError a2dLuaCentralCommandProcessor::FileOpen( a2dDocumentPtr& doc , wxFileName& path )
{
    return a2dDocumentCommandProcessor::FileOpen( doc, path );
}

bool a2dLuaCentralCommandProcessor::FileOpen( const wxString& path )
{
    a2dDocumentPtr doc;
    a2dError error = a2dDocumentCommandProcessor::FileOpen( doc, path );
    return error == a2dError_NoError;
}

bool a2dLuaCentralCommandProcessor::FileSaveAs( const wxString& path, bool silent  )
{
    return a2dDocumentCommandProcessor::FileSaveAs( wxFileName( path ), a2dREFDOC_NON );
}

bool a2dLuaCentralCommandProcessor::FileImport( const wxString& path, const wxString& description )
{
    return a2dDocumentCommandProcessor::FileImport( wxFileName( path ), description, a2dREFDOC_NON );
}

bool a2dLuaCentralCommandProcessor::FileExport( const wxString& path, const wxString& description, bool silent )
{
    return a2dDocumentCommandProcessor::FileExport( wxFileName( path ), description, a2dREFDOC_NON );
}

bool a2dLuaCentralCommandProcessor::Message( const wxString& message )
{
    wxFrame* pf = ( wxFrame* ) wxTheApp->GetTopWindow();
    wxMessageBox( message, wxT( "Gedi::Message" ),  ( int )wxOK | wxCENTRE, pf, 0, 0 );
    return true;
}

bool a2dLuaCentralCommandProcessor::PushTool( const a2dCommandId& which, bool shiftadd, bool oneshot )
{
    return GetDrawingCmdProcessor()->PushTool( which, shiftadd, oneshot );
}

bool a2dLuaCentralCommandProcessor::SetDocumentLayers( const wxString& propertyname, const wxString& value )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dLayers* layers = doc->GetLayerSetup();

    GetDrawingCmdProcessor()->SetOrAddPropertyToObject( layers, propertyname, value, false );
    return true;
}


a2dLayers* a2dLuaCentralCommandProcessor::GetLayersDocument()
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;
    return  doc->GetLayerSetup();
}

a2dLayers* a2dLuaCentralCommandProcessor::GetLayersGlobal()
{
    return a2dCanvasGlobals->GetHabitat()->GetLayerSetup();
}

bool a2dLuaCentralCommandProcessor::SetLayersDocument( a2dLayers* layers )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    doc->SetLayerSetup( layers );
    return true;
}

bool a2dLuaCentralCommandProcessor::SetLayersGlobal( a2dLayers* layers )
{
    a2dCanvasGlobals->GetHabitat()->SetLayerSetup( layers );
    return true;
}

a2dLayers* a2dLuaCentralCommandProcessor::LoadLayers( const wxString& filename )
{
    a2dLayers* layers = new a2dLayers();
    if ( layers->LoadLayers( filename ) )
    {
        return layers;
    }
    delete layers;
    return NULL;
}

bool a2dLuaCentralCommandProcessor::SaveLayersDocument( const wxString& filename )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    if ( doc->GetLayerSetup()->SaveLayers( filename ) )
        return true;
    return false;
}

bool a2dLuaCentralCommandProcessor::SaveLayersGlobal( const wxString& filename )
{
    if ( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() )
        return a2dCanvasGlobals->GetHabitat()->GetLayerSetup()->SaveLayers( filename );
    return false;
}

bool a2dLuaCentralCommandProcessor::SetLayer( wxUint16 layer )
{
    a2dCanvasGlobals->GetHabitat()->SetLayer( layer, true );
    return true;
}

bool a2dLuaCentralCommandProcessor::SetTarget( wxUint16 target )
{
    a2dCanvasGlobals->GetHabitat()->SetTarget( target );
    return true;
}

bool a2dLuaCentralCommandProcessor::ClearGroup( const wxString& group )
{
    a2dSmrtPtr<a2dCommand_SetLayerGroup> command = new a2dCommand_SetLayerGroup( a2dCommand_SetLayerGroup::Args().
            what( group == _T( "A" ) ?  a2dCommand_SetLayerGroup::ClearGroupA : a2dCommand_SetLayerGroup::ClearGroupB ) );
    return SubmitToDrawing( command, m_withUndo );
}

void a2dLuaCentralCommandProcessor::AddGroupA( wxUint16 layer )
{
    a2dCanvasGlobals->GetHabitat()->AddGroupA( layer );
}

void a2dLuaCentralCommandProcessor::AddGroupB( wxUint16 layer )
{
    a2dCanvasGlobals->GetHabitat()->AddGroupB( layer );
}

bool a2dLuaCentralCommandProcessor::DeleteGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::DeleteGroupA ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::MoveGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::MoveGroupA ) );

    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::CopyGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 
    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::CopyGroupA ) );

    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertToArcsGroupA( bool detectCircle )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 
    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertToArcs ).detectCircle( detectCircle ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertPolygonToArcsGroupA( bool detectCircle )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertPolygonToArcs ).detectCircle( detectCircle ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertToPolygonPolylinesWithArcsGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertToPolygonPolylinesWithArcs ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertToPolygonPolylinesWithoutArcsGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertToPolygonPolylinesWithoutArcs ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertPolylineToArcsGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertPolylineToArcs ) );
    return Submit( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertToVPathsGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertToVPaths ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertLinesArcsGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertLinesArcs ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ConvertToPolylinesGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::ConvertToPolylines ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::RemoveRedundantGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::RemoveRedundant ) );
    return SubmitToDrawing( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::DetectSmallGroupA()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( a2dCommand_GroupAB::DetectSmall ) );
    return SubmitToDrawing( command, m_withUndo );
}

#if wxART2D_USE_KBOOL
bool a2dLuaCentralCommandProcessor::BoolOperation_GroupAB( a2dCommand_GroupAB::a2dDoWhat operation, bool clearTarget, bool selectedA, bool selectedB )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dSmrtPtr<a2dCommand_GroupAB> command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
            what( operation ).
            clearTarget( clearTarget ).
            selectedA( selectedA ).
            selectedB( selectedB )
                                                                   );

    return SubmitToDrawing( command, m_withUndo );
}
#endif //wxART2D_USE_KBOOL 

bool a2dLuaCentralCommandProcessor::UnGroup( bool selected, bool deep )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_UnGroupMask* com = new a2dCommand_UnGroupMask( top, deep, selected ? a2dCanvasOFlags::SELECTED : a2dCanvasOFlags::ALL );
    return SubmitToDrawing( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::SetVariable( const wxString& varnamep, const wxString& varvaluep )
{
    a2dSmrtPtr<a2dCommand_SetVariable> command = new a2dCommand_SetVariable( a2dCommand_SetVariable::Args().varname( varnamep ).varvalue( varvaluep ) );
    return Submit( command, m_withUndo );
}

wxString a2dLuaCentralCommandProcessor::GetVariable( const wxString& varname )
{
    const a2dNamedProperty*  propfound = a2dDocviewGlobals->GetVariablesHash().GetVariable( varname );
    if( !propfound )
    {
        a2dDocviewGlobals->ReportError( a2dError_GetVar, _( "wrong variable name, variable does not exist" ) );
        return _( "NOT_A_VARIABLE" ); // error, variable does not exist
    }

    return propfound->StringValueRepresentation();
}

void a2dLuaCentralCommandProcessor::AddConfigPath( const wxString& path )
{
    a2dCanvasGlobals->GetConfigPathList().Add( path );
}

void a2dLuaCentralCommandProcessor::AddLayersPath( const wxString& path )
{
    a2dCanvasGlobals->GetLayersPathList().Add( path );
}

void a2dLuaCentralCommandProcessor::AddFontPath( const wxString& path )
{
    a2dGlobals->GetFontPathList().Add( path );
}

void a2dLuaCentralCommandProcessor::AddImagePath( const wxString& path )
{
    a2dGlobals->GetImagePathList().Add( path );
}

void a2dLuaCentralCommandProcessor::AddIconPath( const wxString& path )
{
    a2dGlobals->GetIconPathList().Add( path );
}

bool a2dLuaCentralCommandProcessor::SetSnap( bool snap )
{
    a2dCanvasGlobals->GetHabitat()->GetRestrictionEngine()->SetSnap( snap );
    return true;
}

bool a2dLuaCentralCommandProcessor::SetSnapFeatures( wxUint32 features )
{
    a2dCanvasGlobals->GetHabitat()->GetRestrictionEngine()->SetSnapTargetFeatures( features );
    return true;
}

bool a2dLuaCentralCommandProcessor::ViewAsImageAdv( const wxFileName& file, wxBitmapType type, bool onView )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 
    if ( !drawer->GetDrawingPart()->GetDrawing() )
        return false;

    return SaveViewAsImage( doc, file.GetFullPath(), type, drawer->GetDrawingPart()->GetShowObject() );
}

void a2dLuaCentralCommandProcessor::SetCursor( double x, double y )
{
    a2dSmrtPtr<a2dCommand_SetCursor> command = new a2dCommand_SetCursor( x, y );
    Submit( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::Find( const wxString& objectname )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dSmrtPtr<a2dCommand_SetShowObject> command = new a2dCommand_SetShowObject( drawer->GetDrawingPart(), a2dCommand_SetShowObject::Args().
            name( objectname ) );
    return drawing->GetCanvasCommandProcessor()->Submit( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::Ask( const wxString&  variablename, const wxString& mes )
{
    wxString text = wxGetTextFromUser( mes );
    a2dDocviewGlobals->GetVariablesHash().SetVariableString( variablename, text );

    return true;
}

bool a2dLuaCentralCommandProcessor::AskFile( const wxString& storeInVariable,
        const wxString& message,
        const wxString& defaultDir,
        const wxString& extension,
        const wxString& fileFilter
                                           )
{
    wxString file = a2dDocumentCommandProcessor::AskFile( message, defaultDir, _T( "" ), extension, fileFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    a2dDocviewGlobals->GetVariablesHash().SetVariable( storeInVariable, new a2dFileNameProperty( a2dPropertyIdFileName::GetDummy(), file ) );
    return true;
}

wxFileName a2dLuaCentralCommandProcessor::AskFile2(
    const wxString& message,
    const wxString& defaultDir,
    const wxString& extension,
    const wxString& fileFilter
)
{
    wxString storeInVariable = _T( "ask_file_result" );
    wxString file = a2dDocumentCommandProcessor::AskFile( message, defaultDir, _T( "" ), extension, fileFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    return wxFileName( file );
}

bool a2dLuaCentralCommandProcessor::ShowVariable( const wxString& variableName,
        const wxString& message
                                                )
{
    const a2dNamedProperty*  prop = a2dDocviewGlobals->GetVariablesHash().GetVariable( variableName );
    if ( !prop )
        return false;

    wxString mes = message + prop->StringValueRepresentation();
    wxMessageBox( mes , _( "ShowVariable" ), wxOK , NULL );

    return true;
}

//----------------------------------------------------------------------------
// object add
//----------------------------------------------------------------------------

a2dCanvasObject* a2dLuaCentralCommandProcessor::Add_a2dCanvasObject( double x, double y  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return AddCurrent( new a2dCanvasObject( x, y ), false, &m_listOfIdforAddPrims  );
}

a2dCanvasObjectReference* a2dLuaCentralCommandProcessor::Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dCanvasObjectReference* ) AddCurrent( new a2dCanvasObjectReference( x, y, obj ), false, &m_listOfIdforAddPrims  );
}

/* TODO
a2dNameReference* a2dLuaCentralCommandProcessor::Add_a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString &text, double size, double angle  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return (a2dNameReference*) AddCurrent( new a2dNameReference( x, y, obj, text, size, angle ) );
}
*/

a2dOrigin* a2dLuaCentralCommandProcessor::Add_a2dOrigin( double w, double h  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dOrigin* ) AddCurrent( new a2dOrigin( w, h ), false, &m_listOfIdforAddPrims  );
}

a2dRectC* a2dLuaCentralCommandProcessor::Add_a2dRectC( double xc, double yc, double w, double h, double angle, double radius  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dRectC* ) AddCurrent( new a2dRectC( xc, yc, w, h, angle, radius ), false, &m_listOfIdforAddPrims  );
}

a2dArrow* a2dLuaCentralCommandProcessor::Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dArrow* ) AddCurrent( new a2dArrow( xt, yt, l1, l2, b, spline ), false, &m_listOfIdforAddPrims  );
}

a2dRect* a2dLuaCentralCommandProcessor::Add_a2dRect( double x, double y, double w, double h , double radius  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dRect* ) AddCurrent( new a2dRect( x, y, w, h, radius ), false, &m_listOfIdforAddPrims );
}

a2dCircle* a2dLuaCentralCommandProcessor::Add_a2dCircle( double x, double y, double radius  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dCircle* ) AddCurrent( new a2dCircle( x, y, radius ), false, &m_listOfIdforAddPrims  );
}

a2dEllipse* a2dLuaCentralCommandProcessor::Add_a2dEllipse( double xc, double yc, double width, double height  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dEllipse* ) AddCurrent( new a2dEllipse( xc, yc, width, height ), false, &m_listOfIdforAddPrims  );
}

a2dEllipticArc* a2dLuaCentralCommandProcessor::Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dEllipticArc* ) AddCurrent( new a2dEllipticArc( xc, yc, width, height, start, end ), false, &m_listOfIdforAddPrims  );
}

a2dArc* a2dLuaCentralCommandProcessor::Add_a2dArc( double xc, double yc, double radius, double start, double end  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dArc* ) AddCurrent( new a2dArc( xc, yc, radius, start, end ), false, &m_listOfIdforAddPrims  );
}

a2dSLine* a2dLuaCentralCommandProcessor::Add_a2dSLine( double x1, double y1, double x2, double y2  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dSLine* ) AddCurrent( new a2dSLine( x1, y1, x2, y2 ), false, &m_listOfIdforAddPrims  );
}

a2dEndsLine* a2dLuaCentralCommandProcessor::Add_a2dEndsLine( double x1, double y1, double x2, double y2  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dEndsLine* ) AddCurrent( new a2dEndsLine( x1, y1, x2, y2 ), false, &m_listOfIdforAddPrims  );
}

a2dImage* a2dLuaCentralCommandProcessor::Add_a2dImage( const wxImage& image, double xc, double yc, double w, double h  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dImage* ) AddCurrent( new a2dImage( image, xc, yc, w, h ) );
}

a2dImage* a2dLuaCentralCommandProcessor::Add_a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    wxString foundfile = imagefile;
    a2dPathList path;
    path.Add( wxT( "." ) );
    if ( !path.ExpandPath( foundfile ) )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), imagefile.c_str(), foundfile.c_str() );
        return false;
    }
    if ( foundfile.IsEmpty() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), imagefile.c_str() );
        return false;
    }


    return ( a2dImage* ) AddCurrent( new a2dImage( foundfile, type, xc, yc, w, h ), false, &m_listOfIdforAddPrims  );
}

a2dText* a2dLuaCentralCommandProcessor::Add_a2dText( const wxString& text, double x, double y, double angle, const a2dFont& font )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dText* ) AddCurrent( new a2dText( text, x, y, font, angle ), false, &m_listOfIdforAddPrims  );
}

a2dPolygonL* a2dLuaCentralCommandProcessor::Add_a2dPolygonL( a2dVertexList* points, bool spline  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dPolygonL* ) AddCurrent( new a2dPolygonL( points, spline ), false, &m_listOfIdforAddPrims  );
}

a2dPolylineL* a2dLuaCentralCommandProcessor::Add_a2dPolylineL( a2dVertexList* points, bool spline  )
{
    if ( !CheckCanvasDocument() )
        return NULL;

    return ( a2dPolylineL* ) AddCurrent( new a2dPolylineL( points, spline ), false, &m_listOfIdforAddPrims  );
}

bool a2dLuaCentralCommandProcessor::Add_Point( double x, double y )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 
    if ( !drawer->GetDrawingPart()->GetDrawing() )
        return false;

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    drawing->GetCommandProcessor()->Submit( new a2dCommand_AddPoint( drawing->GetCanvasCommandProcessor()->GetCurrentCanvasObject(), x, y, -1 ), m_withUndo );

    return true;
}

bool a2dLuaCentralCommandProcessor::Move_Point( int index , double x, double y )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 
    if ( !drawer->GetDrawingPart()->GetDrawing() )
        return false;

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dSLine* sline = wxDynamicCast( drawing->GetCanvasCommandProcessor()->GetCurrentCanvasObject(), a2dSLine );
    drawing->GetCommandProcessor()->Submit( new a2dCommand_MovePoint( sline, x, y, true ), m_withUndo );

    return true;
}


 

//--------------------- drawer related commands ------------------

bool a2dLuaCentralCommandProcessor::DrawWireFrame( bool onOff )
{
    wxUint16 style = RenderWIREFRAME_ZERO_WIDTH | RenderWIREFRAME_SELECT;
    if ( !onOff )
        style = RenderLAYERED | RenderWIREFRAME_SELECT;

    return SetPropertyToObject( _T( "a2dDrawingPart" ), new a2dUint16Property( a2dDrawingPart::PROPID_drawstyle, style ) );
}

bool a2dLuaCentralCommandProcessor::DrawGridLines( bool onOff )
{
    return SetPropertyToObject( _T( "a2dDrawingPart" ), new a2dBoolProperty( a2dDrawingPart::PROPID_gridlines, onOff ) );
}

bool a2dLuaCentralCommandProcessor::DrawGridAtFront( bool onOff )
{
    return SetPropertyToObject( _T( "a2dDrawingPart" ), new a2dBoolProperty( a2dDrawingPart::PROPID_gridatfront, onOff ) );
}

bool a2dLuaCentralCommandProcessor::DrawGrid( bool onOff )
{
    return SetPropertyToObject( _T( "a2dDrawingPart" ), new a2dBoolProperty( a2dDrawingPart::PROPID_grid, onOff ) );
}

bool a2dLuaCentralCommandProcessor::ZoomOut()
{
    return GetDrawingCmdProcessor()->ZoomOut();
}

bool a2dLuaCentralCommandProcessor::Refresh()
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    doc->GetDrawing()->GetRootObject()->Update( a2dCanvasObject::updatemask_force );
    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
        drawer->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
    return true;
}

bool a2dLuaCentralCommandProcessor::InsertGroupRef( double x, double y )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
        a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

        a2dWalker_CollectCanvasObjects collector;
        collector.Start( doc->GetDrawing()->GetRootObject() );

        a2dCanvasObjectsDialog groups( doc->GetAssociatedWindow(), &collector.m_found, false, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dCanvasObjectReference* groupref = new a2dCanvasObjectReference( x, y, groups.GetCanvasObject() );
            top->Prepend( groupref );
        }
    }
    return true;
}

bool a2dLuaCentralCommandProcessor::AddGroupObject( const wxString& groupName, double x, double y, wxUint16 layer )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dSmrtPtr<a2dCommand_NewGroup> command = new a2dCommand_NewGroup( top, a2dCommand_NewGroup::Args().
            name( groupName ).
            x( x ).
            y( y ).
            layer( layer ) );
    return Submit( command, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::NewPin( double x, double y, const wxString& groupname )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
        a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

        wxString name = groupname;
        if ( name.IsEmpty() )
            name = wxGetTextFromUser( _( "give name of new pin:" ) );
        long ang = wxGetNumberFromUser( _( "Give pin angle:" ), _( "angle:" ), _( "pin angle" ), 0, -360, 360 );

        a2dPin* pin = new a2dPin( top, name, a2dPinClass::Standard, 0, 0, ang );
        top->Prepend( pin );
    }
    return true;
}

bool a2dLuaCentralCommandProcessor::PushInto( const wxString& name )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

        a2dCanvasObject* g = NULL;
        if ( name.IsEmpty() )
            g = wxStaticCast( top->Find( wxT( "" ),  wxT( "" ), a2dCanvasOFlags::SELECTED ), a2dCanvasObject );
        else
            g = wxStaticCast( top->Find( name ), a2dCanvasObject );

        if ( g )
        {
            //let drawer follow document if needed.
            //if ( doc->GetRootObject() != g->GetRoot() )
            //    drawer->SetDocument( g->GetCanvasDocument() );
            drawer->GetDrawingPart()->SetShowObject( g );
        }
        else
            ( void )wxMessageBox( _( "Does not contain a group, unable to push into" ), _( "push into" ), wxICON_INFORMATION | wxOK );
    }
    return true;
}

bool a2dLuaCentralCommandProcessor::SelectAll()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    GetDrawingCmdProcessor()->Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::SelectAll ) ), m_withUndo );

    return true;
}

bool a2dLuaCentralCommandProcessor::DeSelectAll()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    GetDrawingCmdProcessor()->Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ), m_withUndo );
    return true;
}

bool a2dLuaCentralCommandProcessor::SelectedChangeLayer( long layer )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    SetTarget( layer );
    GetDrawingCmdProcessor()->Submit( new a2dCommand_SetLayerMask( top ), m_withUndo );
    return true;
}

bool a2dLuaCentralCommandProcessor::SetFillStrokeSelected()
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
        a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

        top->GetChildObjectList()->SetDrawerStyle( a2dCanvasGlobals->GetHabitat()->GetFill() , a2dCanvasGlobals->GetHabitat()->GetStroke() , a2dCanvasOFlags::SELECTED );
    }
    return true;
}

bool a2dLuaCentralCommandProcessor::ToTop()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    return GetDrawingCmdProcessor()->Submit( new a2dCommand_ToTopMask( top ), m_withUndo );
}

bool a2dLuaCentralCommandProcessor::ToBack()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    return GetDrawingCmdProcessor()->Submit( new a2dCommand_ToBackMask( top ), m_withUndo );
}

bool a2dLuaCentralCommandProcessor::DeleteSelected()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    return GetDrawingCmdProcessor()->Submit( new a2dCommand_DeleteMask( top ), m_withUndo );
}

bool a2dLuaCentralCommandProcessor::MoveSelected( double x, double y, long layer )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_MoveMask* com = new a2dCommand_MoveMask( top, x, y, layer );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::CopySelected( double x, double y, long layer )
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_CopyMask* com = new a2dCommand_CopyMask( top, x, y, layer );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::TransformSelected( const wxString& str )
{
    a2dAffineMatrix lworld;

    wxString error;
    if ( !str.IsEmpty() && !::ParseCvgTransForm( lworld, str, error ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, _( "CVG : invalid transform %s " ), str.c_str() );
    }

    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_TransformMask* com = new a2dCommand_TransformMask( top, lworld );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::Group()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupMask* com = new a2dCommand_GroupMask( top );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::UnGroup()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_UnGroupMask* com = new a2dCommand_UnGroupMask( top );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

bool a2dLuaCentralCommandProcessor::SetUrl()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return false; 

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_UrlOnMask* com = new a2dCommand_UrlOnMask( top );
    return GetDrawingCmdProcessor()->Submit( com, m_withUndo );
}

//----------------------------------------------------------------------------
// a2dLuaConsole
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( a2dLuaConsole, wxLuaIDE );

BEGIN_EVENT_TABLE( a2dLuaConsole, wxLuaIDE )
    EVT_LUA_PRINT     ( wxID_ANY, a2dLuaConsole::OnLua )
    EVT_LUA_ERROR       ( wxID_ANY, a2dLuaConsole::OnLua )
    EVT_LUA_DEBUG_HOOK  ( wxID_ANY, a2dLuaConsole::OnLua )
END_EVENT_TABLE()

a2dLuaConsole::a2dLuaConsole(  a2dLuaCentralCommandProcessor* cmdh, wxWindow* parent, int id,
                               const wxPoint& pos, const wxSize& size,
                               long style, long options, const wxString& name )
    : wxLuaIDE( parent, id, pos, size, style | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE, options, name )
{
    m_cmdh = cmdh;

    wxFrame* pf = ( wxFrame* ) wxTheApp->GetTopWindow();

    wxMenuBar* menuBar = 0;//pf->GetMenuBar();
    wxToolBar* toolBar = 0;//pf->GetToolBar();

    GetEditorNotebook()->GetOptions().SetMenuBar( menuBar );
    GetLuaOutputWin()->GetOptions().SetMenuBar( menuBar );
    GetLuaShellWin()->GetOptions().SetMenuBar( menuBar );

    GetEditorNotebook()->GetOptions().SetToolBar( toolBar );
    GetLuaOutputWin()->GetOptions().SetToolBar( toolBar );
    GetLuaShellWin()->GetOptions().SetToolBar( toolBar );
}

a2dLuaConsole::~a2dLuaConsole()
{
}

void a2dLuaConsole::OnLua( wxLuaEvent& event )
{
    if ( event.GetwxLuaState() ==  m_cmdh->GetLuaState() )
        OutputLuaEvent( event, m_luaOutput );
    else
        wxLuaIDE::OnLua( event );
}

BEGIN_EVENT_TABLE( a2dLuaExecDlg, wxDialog )
    EVT_CLOSE( a2dLuaExecDlg::OnCloseWindow )
    EVT_CHAR_HOOK( a2dLuaExecDlg::OnCharHook )

    EVT_RECORD( a2dLuaExecDlg::OnRecord )
    EVT_DO( a2dLuaExecDlg::OnDoEvent )
    EVT_UNDO( a2dLuaExecDlg::OnUndoEvent )
END_EVENT_TABLE()

a2dLuaExecDlg::a2dLuaExecDlg( a2dLuaCentralCommandProcessor* commandProcessor, wxFrame* parent, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, -1, title, wxDefaultPosition, wxDefaultSize, style, name )
{
    m_parent = parent;

    wxBoxSizer* itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer );

    m_commandProcessor = commandProcessor;

    m_luaConsole = new a2dLuaConsole( commandProcessor, this, wxID_ANY, wxDefaultPosition, wxSize( 400, 400 ) );
    itemBoxSizer->Add( m_luaConsole, 1, wxGROW | wxALL, 0 );

    SetSizeHints( wxSize( 400, 400 ) );
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
    // The size of the frame isn't set when the splitter is created, resize it
    wxSplitterWindow* splitWin = m_luaConsole->GetSplitterWin();
    splitWin->SetSashPosition( splitWin->GetSize().y / 2 );
}

a2dLuaExecDlg::~a2dLuaExecDlg()
{
}

void a2dLuaExecDlg::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    Show( false );
}

void a2dLuaExecDlg::OnRecord( a2dCommandEvent& event )
{
    wxString cmdName = event.GetRecord();
}

void a2dLuaExecDlg::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{

}

void a2dLuaExecDlg::OnDoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
}

void a2dLuaExecDlg::OnCharHook( wxKeyEvent& event )
{
    /*
        if ( event.GetKeyCode() == WXK_RETURN )
        {
          Execute();
        }
        else*/ if ( event.GetKeyCode() == WXK_ESCAPE )
    {
        wxCommandEvent eventc;
        Show( false );

    }
    else
    {
        // key code is within legal range. we call event.Skip() so the
        // event can be processed either in the base wxWindows class
        // or the native control.

        event.Skip();
    }
}

//----------------------------------------------------------------------------
// a2dLuaEditorFrame
//----------------------------------------------------------------------------

extern const long SCRIPT_luawrap = wxNewId();
extern const long EXECDLG_lua = wxNewId();

IMPLEMENT_DYNAMIC_CLASS( a2dLuaEditorFrame, a2dEditorFrame )

BEGIN_EVENT_TABLE( a2dLuaEditorFrame, a2dEditorFrame )

    EVT_THEME_EVENT( a2dLuaEditorFrame::OnTheme )
    EVT_INIT_EVENT( a2dLuaEditorFrame::OnInit )

END_EVENT_TABLE()

//! canvas window to display the view upon to be defined by user
#define DOC ((a2dCanvasView*)m_view.Get())->GetCanvasDocument()

a2dLuaEditorFrame::a2dLuaEditorFrame()
    : a2dEditorFrame()
{
    m_initialized = false;
}

a2dLuaEditorFrame::a2dLuaEditorFrame( bool isParent, 
                                      wxFrame* parent, const wxPoint& pos, const wxSize& size, long style )
    : a2dEditorFrame( isParent, parent, pos, size, style )
{
    m_initialized = false;

    // create a canvas in Create, the first arg. is true.
    Create( isParent, parent, pos, size, style );

    a2dCanvas* canvas = new a2dCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    m_drawingPart = canvas->GetDrawingPart();
}

a2dLuaEditorFrame::~a2dLuaEditorFrame()
{
}

void a2dLuaEditorFrame::OnInit( a2dEditorFrameEvent& initEvent )
{
    initEvent.Skip();
}


void a2dLuaEditorFrame::OnTheme( a2dEditorFrameEvent& themeEvent )
{

    //CreateThemeDefault();
    wxMenu* dlg_menu = new wxMenu;
    m_menuBar->Append( dlg_menu, _( "&Lua Dialogs" ) );

    AddFunctionToMenu( EXECDLG_lua, dlg_menu, _( "&Lua Commandline Dialog" ), _( "interactive lua command dialog" ), ( wxObjectEventFunctionM ) &a2dLuaEditorFrame::OnShowExecDlg2, true );
    AddFunctionToMenu( SCRIPT_luawrap, dlg_menu, _T( "Run Lua Script" ), _T( "choose a wxLua script to run" ), ( wxObjectEventFunctionM ) &a2dLuaEditorFrame::OnRunScript , true );

    themeEvent.Skip();
}

void a2dLuaEditorFrame::OnShowExecDlg2( wxCommandEvent& event )
{
    a2dCommand_ShowDlg* command = new a2dCommand_ShowDlg( a2dLuaCentralCommandProcessor::COMID_ShowLuaExecDlg );
    a2dLuaCentralCommandProcessor* luacmd = wxDynamicCast( a2dGetCmdh(), a2dLuaCentralCommandProcessor );
    luacmd->Submit( command );
}

void a2dLuaEditorFrame::OnRunScript( wxCommandEvent& event )
{
#if wxART2D_USE_LUA
    if ( !wxDynamicCast( a2dGetCmdh(), a2dLuaCentralCommandProcessor ) )
    {
        ( void )wxMessageBox( _( "For this function a a2dLuaCentralCommandProcessor is needed" ), _( "Lua Scripts" ), wxICON_INFORMATION | wxOK );
        return;
    }

    a2dLuaCentralCommandProcessor* luacmd = wxDynamicCast( a2dGetCmdh(), a2dLuaCentralCommandProcessor );

    wxString fullPath = wxFileSelector( _( "Select a script file" ),
                                        _( "" ),
                                        _( "" ),
                                        _( "lua" ),
                                        _( "*.lua" ) ,
                                        0,
                                        this
                                      );

    if ( !fullPath.IsEmpty() && ::wxFileExists( fullPath ) )
    {
        if ( !luacmd->ExecuteFile( fullPath ) )
        {
            wxLogWarning( _( "Error in Lua Script" ) );
        }
    }
#endif
}

void a2dLuaEditorFrame::OnUpdateUI( wxUpdateUIEvent& event )
{
    a2dLuaCentralCommandProcessor* luacmd = wxDynamicCast( a2dGetCmdh(), a2dLuaCentralCommandProcessor );
    if ( event.GetId() == EXECDLG_lua )
    {
        m_menuBar->Check( event.GetId(), luacmd->IsShowna2dLuaExecDlg() );
    }

}

bool a2dLuaEditorFrame::CallLuaScriptThemeFunction( const wxString& fileName, const wxString& function )
{
    a2dLuaCentralCommandProcessor* luacmd = a2dGetLuaWP();

    wxLuaState lst = luacmd->GetLuaState();
    lua_State* L = lst.GetLuaState();
    if ( 0 != lst.RunFile( fileName ) )
    {
        wxLogWarning( _( "Error in Lua Script" ) );
        return false;
    }
    else
    {
        lua_getglobal( L, wx2lua( function ) );
        lst.wxluaT_PushUserDataType( this, wxluatype_a2dLuaEditorFrame, true );
        if ( lst.lua_PCall( 1, 0, 0 ) != 0 )
        {
            wxString error;
            error.Printf( wxT( "%s %s" ), function.c_str() , lua_tostring( L, -1 ) );
            wxLuaEvent event( wxEVT_LUA_ERROR, lst.GetId(), lst );
            event.SetString( error );
            lst.SendEvent( event );
            lua_pop( L, 1 );
        }
    }
    return true;
}
