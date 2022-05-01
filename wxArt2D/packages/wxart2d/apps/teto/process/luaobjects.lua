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

    -- add script object based on lua script to add child objects
    -- make a string of the script which will add the objects from within C++ calling AddChilds on that object as input for adding child objects
    TheObjectAddScript =
    [[
            function AddChilds( canobj, x, y )
                local childs = canobj:CreateChildObjectList()
                canobjcirc2 = wx.a2dCircle( 100, 200, 500 )
                fill = canobjcirc2:GetFill()
                fill:SetColour( wx.wxColour(13,234,255) )
                canobjcirc2:SetFill( fill )
                childs:push_back( canobjcirc2 )
                canobjrect1 = wx.a2dRect( 300, 200, 500,100 )
                childs:push_back( canobjrect1 )
            end
    ]]

-- add the object using the above string as input script
canobjAddScript = wx.a2dCanvasObjectLua( 150, 250,  TheObjectAddScript, "AddChilds"  )
fill = canobjAddScript:GetFill()
fill:SetColour( wx.wxRED )
fill:SetStyle( wx.a2dFILL_BDIAGONAL_HATCH )
obj:SetFill( fill )
childs:push_back( canobjAddScript )

    TheObjectAddScript_2 =
    [[
        function XoverX( canobj, x, y, args )
            fill = wx.a2dFill( args.Colour )

            local childs = canobj:CreateChildObjectList()
            canobjcirc2 = wx.a2dCircle( 0, 0, 20 )
            childs:push_back( canobjcirc2 )
            for i = -args.maxX, args.maxX do
                local y2
                if  i ~= 0  then
                    y2 = -args.maxY*math.sin(i/16)/(i/16)
                else
                    y2 = -args.maxY; --sin(x)/x goes to one at zero
                end
                -- all relative
                line = wx.a2dRect( i, 0, 0.1, y2 )
                line:SetFill( fill )
                childs:push_back( line )
            end
        end
    ]]

-- add the object using the above string as input script
canobjAddScript2 = wx.a2dCanvasObjectLua( 350, -450,  TheObjectAddScript_2, "XoverX"  )
canobjAddScript3 = wx.a2dCanvasObjectLua( 550, 250,  TheObjectAddScript_2, "XoverX"  )

-- create property Id's for input parameters to wxLua object. The properties will be tarnsformed into a hash with key being the names of the property ID's
if propidInit == nil then
    propid_maxX = wx.a2dPropertyIdDouble( "maxX", 0 )
    propid_maxY = wx.a2dPropertyIdDouble( "maxY", 0 )
    propid3 = wx.a2dPropertyIdUint32( "aUint32", 10 )
    propid4 = wx.a2dPropertyIdInt32( "aInt32", 10 )
    propid5 = wx.a2dPropertyIdString( "aString", "propstring" )
    propid_Colour = wx.a2dPropertyIdColour( "Colour", wx.wxColour( 0, 255, 255 ) )
    propidInit = true
end

propid_maxX:SetPropertyToObject( canobjAddScript2, 425 )
propid_maxY:SetPropertyToObject( canobjAddScript2, 432 )
propid_Colour:SetPropertyToObject( canobjAddScript2, wx.wxColour( 223, 155, 5 ) )
propid3:SetPropertyToObject( canobjAddScript2, 10 )
propid4:SetPropertyToObject( canobjAddScript2, 10 )
propid5:SetPropertyToObject( canobjAddScript2, "propstring" )

propid_maxX:SetPropertyToObject( canobjAddScript3, 625 )
propid_maxY:SetPropertyToObject( canobjAddScript3, -232 )
propid_Colour:SetPropertyToObject( canobjAddScript3, wx.wxColour( 0, 155, 255 ) )

--prop = wx.a2dDoubleProperty( propid, 125 );
--canobjAddScript2:AddProperty( prop )
--prop2 = wx.a2dDoubleProperty( propid2, 423 );
--canobjAddScript2:AddProperty( prop2 )

fill = canobjAddScript2:GetFill()
fill:SetColour( wx.wxGREEN )
obj:SetFill( fill )
childs:push_back( canobjAddScript2 )
fill = canobjAddScript3:GetFill()
fill:SetColour( wx.wxBLUE )
obj:SetFill( fill )
childs:push_back( canobjAddScript3 )
    
function PreFuncAddChilds( canobj, x, y )
    local childs = canobj:CreateChildObjectList()
    canobjcirc2 = wx.a2dCircle( 100, 200, 500 )
    fill = canobjcirc2:GetFill()
    fill:SetColour( wx.wxBLUE )
    canobjcirc2:SetFill( fill )
    childs:push_back( canobjcirc2 )
    canobjrect1 = wx.a2dRect( 300, 200, 500, 350 )
    fill = wx.a2dFill( wx.wxColour(123,34,255) )
    --fill:SetColour( wx.wxRED )
    canobjrect1:SetFill( fill )
    childs:push_back( canobjrect1 )
end    

canobjAddScript3 = wx.a2dCanvasObjectLua( 650, 350,  "", "PreFuncAddChilds"  )
fill = canobjAddScript3:GetFill()
fill:SetColour( wx.wxGREEN )
obj:SetFill( fill )
childs:push_back( canobjAddScript3 )
    
    
doc:SetCanvasDocumentRecursive()

root:SetPending( true )


