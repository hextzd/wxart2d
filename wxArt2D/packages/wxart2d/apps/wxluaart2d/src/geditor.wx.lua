
frame = nil
cmdh = wx.a2dGetLuaWP()

home = wx.wxGetHomeDir()

layers = cmdh:GetLayersGlobal() 
--layers = wx.a2dLayers()
layers:InitWith( 500, false, true )

cmdh:SaveLayersGlobal( home .. "/config/luagenerated.cvg" )  
cmdh:SetLayersGlobal( layers ) 
cmdh:SetLayersDocument( layers ) 

package.cpath = package.cpath..";./?.dll;./?.so;../lib/?.so;../lib/vc_dll/?.dll;../lib/bcc_dll/?.dll;../lib/mingw_dll/?.dll;"
require("wx")

-- create document via central command processor, setting templates etc.
function fillDocument()
    local document
    document = wx.a2dCanvasDocument()
    document:CreateCommandProcessor()
    --cmdh:CreateDocuments( "dummy",  wx.a2dREFDOC_NEW )
    ---document =  cmdh:GetDocuments():back():DynamicCast("a2dCanvasDocument")
    root = document:GetRootObject()
    childs = root:CreateChildObjectList()
    obj = wx.a2dCircle(600,-400,550)
    obj:SetLayer(0)
    childs:push_back( obj )
    document:SetCanvasDocumentRecursive()
    root:SetPending( true )
    return document
end


function ThemeEditor( editorFrame )
    -- create a simple file menu
    local fileMenu = wx.wxMenu()
    fileMenu:Append(wx.wxID_EXIT, "E&xit", "Quit the program")

    res, maskproc = wx.wxGetEnv( "MASKPROC_ROOT"  )
    command = wx.a2dCommand_FileOpen{ filename = maskproc.."/drawings/bigtest.cal" }
    ef:AddCommandToMenu( fileMenu, "&Open", "Open file", command )
    command = wx.a2dCommand_FileOpen{}
    ef:AddCommandToMenu( fileMenu, "&Open", "Open file", command )
    command = wx.a2dCommand_GroupAB{ what = wx.a2dCommand_GroupAB.BoolOperation_GroupAB, operation = wx.BOOL_OR }
    ef:AddCommandToMenu( fileMenu, "&Merge", "Merge polygons", command )

    -- create a menu bar and append the file and help menus
    local menuBar = wx.wxMenuBar()
    menuBar:Append(fileMenu, "&File")

    -- attach the menu bar into the frame
    ef:SetMenuBar(menuBar)

    -- create a simple status bar
    ef:CreateStatusBar(1)
    ef:SetStatusText("Welcome to wxLua with WxArt2D.")
end

-- Create a function to encapulate the code, not necessary, but it makes it
--  easier to debug in some cases.
function editorframe()
                      
    --document = wx.a2dCanvasDocument()                      
    document:SetLayerSetup( layers )
    view = cmdh:AddDocumentCreateView( document ):DynamicCast("a2dCanvasView") 
    --view = wx.a2dCanvasView()                      
    --view:SetDocument( document )
    editorFrame = wx.a2dEditorFrame( true, view, wx.NULL, wx.wxDefaultPosition, wx.wxSize(600,700),wx.wxDEFAULT_FRAME_STYLE )
    editorFrame:SetDestroyOnCloseView( false );
    editorFrame:SetTitle( "wxLua created drawing tool" )
    --wx:SetTopWindow(editorFrame)

	canvas = editorFrame:GetCanvas()
    canvas:SetZoomOutBorder( 30 );
    connector = document:GetDocumentTemplate():GetViewConnector():DynamicCast("a2dWindowViewConnector")
    connector:SetDisplayWindow( canvas )

    editorFrame:Connect(0, wx.a2dEVT_THEME_EVENT,
                    function ( themeEvent )
                        ef = themeEvent:GetEditorFrame()
                        --ef:Theme()
                        ThemeEditor( ef )
                    end )

    editorFrame:Enable(true);                      
    editorFrame:Init();                      
    editorFrame:Show(true)


end

document = fillDocument()
editorframe()



wx.wxGetApp():MainLoop()
