
frame = nil
cmdh = wx.a2dLuaWP

package.cpath = package.cpath..";./?.dll;./?.so;../lib/?.so;../lib/vc_dll/?.dll;../lib/bcc_dll/?.dll;../lib/mingw_dll/?.dll;"
require("wx")

-- Create a function to encapulate the code, not necessary, but it makes it
--  easier to debug in some cases.
function editorframe()
                      
    doc = wx.a2dCanvasDocument()                      
    view = wx.a2dCanvasView()                      
    view:SetDocument( doc )
    editorFrame = wx.a2dEditorFrame( false, view, wx.NULL, wx.wxDefaultPosition, wx.wxSize(600,700),wx.wxDEFAULT_FRAME_STYLE )
    editorFrame:Enable(true);                      
    editorFrame:Init();                      
    editorFrame:Show(true)
end

editorframe()

wx.wxGetApp():MainLoop()
