cmdh = wx.a2dLuaWP
home = wx.wxGetHomeDir()

doc = cmdh:CheckCanvasDocument()
drawing = doc:GetDrawing()
root = drawing:GetRootObject() 

childs = root:CreateChildObjectList()
iter = childs:lbegin()
    
function Mlin( canobj, x, y, args )
    local childs = canobj:CreateChildObjectList()
    canobjrect1 = wx.a2dRect( 0, -args.width/2, args.lenght, args.width )
    fill = wx.a2dFill( wx.wxColour(123,34,255) )
    canobjrect1:SetFill( fill )
    childs:push_back( canobjrect1 )
    pin1 = wx.a2dPin( canobj, "pin1", wx.a2dPinClass.Standard, 0, 0, 180 )
    childs:push_back( pin1 )
    pin2 = wx.a2dPin( canobj, "pin2", wx.a2dPinClass.Standard, args.lenght, 0, 0 )
    childs:push_back( pin2 )
end    

canobjMlin = wx.a2dCanvasObjectLua( 650, 350,  "", "Mlin"  )

-- create property Id's for input parameters to wxLua object. The properties will be transformed into a hash with key being the names of the property ID's
propid = canobjMlin:HasPropertyId( "width" );
if propid == nil then
    propid_width = wx.a2dPropertyIdDouble( "width", 0, wx.a2dPropertyId.flag_userDefined )
    canobjMlin:AddPropertyId( propid_width );
end
propid = canobjMlin:HasPropertyId( "lenght" );
if propid == nil then
    propid_lenght = wx.a2dPropertyIdDouble( "lenght", 0, wx.a2dPropertyId.flag_userDefined )
    canobjMlin:AddPropertyId( propid_lenght );
end


propid_width:SetPropertyToObject( canobjMlin, 125 )
propid_lenght:SetPropertyToObject( canobjMlin, 400 )
fill = wx.a2dFill( wx.wxColour(123,34,255) )
canobjMlin:SetFill( fill )
childs:push_back( canobjMlin )

canobjMlin2 = wx.a2dCanvasObjectLua( 1050, 350,  "", "Mlin"  )
propid_width:SetPropertyToObject( canobjMlin2, 250 )
propid_lenght:SetPropertyToObject( canobjMlin2, 800 )
fill = wx.a2dFill( wx.wxColour(223,234,55) )
canobjMlin:SetFill( fill )
childs:push_back( canobjMlin2 )
  
    
drawing:SetRootRecursive()  
cmdh:Refresh()

root:SetPending( true )


