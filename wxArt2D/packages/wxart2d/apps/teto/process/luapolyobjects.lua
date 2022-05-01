cmdh = wx.a2dLuaWP
home = wx.wxGetHomeDir()

doc = cmdh:CheckCanvasDocument()
root = doc:GetRootObject()

childs = root:CreateChildObjectList()
iter = childs:lbegin()
    
obj = wx.a2dCircle(600,-400,550)
obj:SetLayer(2)
fill = obj:GetFill()
fill:SetColour( wx.wxRED )
fill:SetStyle( wx.a2dFILL_BDIAGONAL_HATCH )
obj:SetFill( fill )
childs:push_back( obj )

obj2 = wx.a2dCircle(1200,-400,550)
obj2:SetLayer(3)
fill2 = obj2:GetFill()
fill2:SetColour( wx.wxColour( 123, 23, 178 ) )
fill2:SetStyle( wx.a2dFILL_CROSSDIAG_HATCH )
obj2:SetFill( fill2 )
childs:push_back( obj2 )

poly = wx.a2dPolygonL()
poly:AddPoint( 110, 100 )
poly:AddPoint( 210, 200 )
poly:AddPoint( 110, 150 )
poly:AddPoint( 310, 500 )
poly:AddPoint( 410, 200 )
poly:SetLayer(3)
polyfill = poly:GetFill()
polyfill:SetColour( wx.wxColour( 123, 23, 178 ) )
polyfill:SetStyle( wx.a2dFILL_CROSSDIAG_HATCH )
poly:SetFill( polyfill )
childs:push_back( poly )

    
   
doc:SetCanvasDocumentRecursive()

root:SetPending( true )


