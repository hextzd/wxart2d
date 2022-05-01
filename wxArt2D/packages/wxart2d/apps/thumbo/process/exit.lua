wx.a2dLuaWP:Message("Open two files in sequence and display them \n sleep 10 second in between")     
wx.a2dLuaWP:FileOpen("../drawings/test0721.cal") wx.wxYield() wx.wxSleep(2) wx.a2dLuaWP:FileOpen("../drawings/polygons.cvg")
wx.a2dLuaWP:FileExport("../drawings/ape.xml", "")
wx.a2dLuaWP:Exit()
