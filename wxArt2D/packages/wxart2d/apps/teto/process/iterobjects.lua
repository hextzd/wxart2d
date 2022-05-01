cmdh = wx.a2dLuaWP
home = wx.wxGetHomeDir()

doc = cmdh:CheckCanvasDocument()
root = doc:GetRootObject()

childs = root:CreateChildObjectList()
iter = childs:lbegin()
cmdh:Message("name is:"..tostring(root))     
    
if not childs:empty() then
    ff = childs:front()
    ff:SetName( "reaalyItIshappening")
    aap = childs:front():GetName()
    --cmdh:Message("name is:"..aap)     
end

obj = wx.a2dCircle(600,-400,550)
obj:SetLayer(2)
childs:push_back( obj )
obj2 = wx.a2dCircle(1200,-400,550)
obj2:SetLayer(3)
childs:push_back( obj2 )
obj2 = wx.a2dRect(100,200,550,200)
obj:SetLayer(4)
childs:push_back( obj2 )

fill = obj:GetFill()
fill:SetColour( wx.wxRED )
fill:SetStyle( wx.a2dFILL_BDIAGONAL_HATCH )
obj:SetFill( fill )
fill2 = obj2:GetFill()
fill2:SetColour( wx.wxColour( 123, 23, 178 ) )
fill2:SetStyle( wx.a2dFILL_CROSSDIAG_HATCH )
obj2:SetFill( fill2 )

iter2 = childs:lend()

doc:SetCanvasDocumentRecursive()

--cmdh:Message("iter:"..tostring(iter).."iter2:"..tostring(iter2) )     
iter3 = childs:lbegin()
iter4 = childs:lend()

if iter3:op_eq(iter4) then
    cmdh:Message("list is empty"..tostring(iter3)..tostring(iter4) )     
else    
    cmdh:Message("list has "..childs:size().." elements" )     
end

nameOfobj = ""
iter3 = childs:lbegin()
for i=1,childs:size() do
    obj = iter3:GetItem()
    nameOfobj = nameOfobj.."\n"..obj:Get():GetName().."\n    "..obj:Get():GetClassInfo():GetClassName()
    iter3:op_inc()
end

cmdh:Message("names are:\n"..nameOfobj)     

layers = cmdh:GetLayersDocument() 
layers:SetPending( true );

root:SetPending( true )
view = cmdh:CheckDrawer()
view:Update( wx.a2dCANVIEW_UPDATE_OLDNEW )


--childs:ChangeLayer( 3 )


