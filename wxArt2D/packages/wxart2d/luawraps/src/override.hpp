// ----------------------------------------------------------------------------
// Overridden functions for the wxLua binding for wxArt2D
//
// Please keep these functions in the same order as the .i file and in the
// same order as the listing of the functions in that file.
// ----------------------------------------------------------------------------

   
// ----------------------------------------------------------------------------
// Overrides for wxArt2D luawrap info files (*.i)
// ----------------------------------------------------------------------------

%override wxLua_a2dCommand_GroupAB_constructor
static int LUACALL wxLua_a2dCommand_GroupAB_constructor(lua_State *L)
{
    if ( !lua_istable(L, 1))
    {    
        wxlua_argerror(L, 1, wxT("arguments must be table"));
        return 0;
    } 

    a2dCommand_GroupAB::Args args;
    /* table is in the stack at index '1' */
    lua_pushnil(L);  /* first key */
    while (lua_next(L, 1) != 0) 
    {
        // value = -1, key = -2, table = -3
        wxString key = wxlua_getwxStringtype( L, -2 );
        if ( key == wxT("what") )
            args.what( (a2dCommand_GroupAB::a2dDoWhat) wxlua_getenumtype( L, -1 ) );
        else if ( key == wxT("target") )
            args.target( wxlua_getintegertype( L, -1 ) );
        else if ( key == wxT("clearTarget") )
            args.target( wxlua_getbooleantype( L, -1 ) );
        else if ( key == wxT("selectedA") )
            args.selectedA( wxlua_getbooleantype( L, -1) );
        else if ( key == wxT("selectedB") )
            args.selectedB( wxlua_getbooleantype( L, -1) );
        else if ( key == wxT("detectCircle") )
            args.detectCircle( wxlua_getbooleantype( L, -1) );
        else if ( key == wxT("fileNameOut") )
            args.fileNameOut( wxlua_getwxStringtype( L, -1) );

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }              
    a2dCanvasView* drawer = a2dLuaWP->CheckDrawingView();
    if ( !drawer )
        return false; 
    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    a2dCommand_GroupAB *returns = new a2dCommand_GroupAB( top, args );

    // push the constructed class pointer
    wxluaT_pushuserdatatype(L, returns, wxluatype_a2dCommand_GroupAB );
    // return the number of parameters
    return 1;
}
%end 
