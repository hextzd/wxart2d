cmdh = wx.a2dLuaWP
home = wx.wxGetHomeDir()

doc = cmdh:CheckCanvasDocument()
drawing = doc:GetDrawing()
root = drawing:GetRootObject() 

childs = root:CreateChildObjectList()
cmdh:Message("root name is:"..tostring(root))     
    
nameOfobj = ""

function CanvasObjectChildrenReport( indent, canobj )
    if ( canobj:GetChildObjectsCount() ) then
        local childs = canobj:GetChildObjectList()
        local iter3 = childs:lbegin()
        for i=1,childs:size() do
            local obj = iter3:op_mul()
            nameOfobj = nameOfobj..indent.." "..indent..obj:Get():GetName().." "..indent..obj:Get():GetClassInfo():GetClassName().." "
            iter3:op_inc()
            if obj:Get():GetChildObjectsCount() then
                local indentdeeper = indent.."   "
                CanvasObjectChildrenReport( indentdeeper, obj:Get() )                
                if ( obj:Get():IsKindOf( wx.wxClassInfo.FindClass("a2dCanvasObjectReference"))) then                
                    local refobj = obj:Get():DynamicCast("a2dCanvasObjectReference")
                    if refobj then
                        local indentdeeper = indent.."   "
                        --cmdh:Message("name of ref:\n"..refobj:GetName() )     
                        nameOfobj = nameOfobj.."\n"..indentdeeper..refobj:GetName().."\n"
                        local referredobj = refobj:GetCanvasObject()
                        CanvasObjectChildrenReport( indentdeeper, referredobj )                
                    end               
                end    
            end    
        end
        nameOfobj = nameOfobj.."\n"
    end    
end

CanvasObjectChildrenReport( "", root )

cmdh:Message("names are:\n"..nameOfobj)     




