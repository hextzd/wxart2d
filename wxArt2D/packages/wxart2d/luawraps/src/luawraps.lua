-------------------------------------------------------------------------------
-- This is the lua wrapper to interface between the C++ wxPlotWindow classes
-- and the wxLua based interpreter used the the Plotto program.
--
-- It does some type checking for each function and gives reasonable error msgs.
-- 
-- The table plotto has member functions that take an integer index into the 
-- curves that are displayed in the plot window. It should be always safe to
-- use these functions and any errors should be caught in lua and an error
-- message given instead of an gui assert dialog.
-------------------------------------------------------------------------------

teto = {}

-------------------------------------------------------------------------------
-- Definitions
-------------------------------------------------------------------------------

local letSp  = string.byte(" ")
local letTab = 9
local letCR  = 13
local letLF  = 10

-------------------------------------------------------------------------------
-- Utilities
-------------------------------------------------------------------------------
function plotassert(no_assert_on_error, cond, msg)
    if not no_assert_on_error then 
        assert(cond, msg) 
    end
end

-------------------------------------------------------------------------------
-- string utilities
-------------------------------------------------------------------------------

function string.printf(...)
    print(string.format(unpack(arg)))
end

function io.printf(...)
    io.write(string.format(unpack(arg)))
end

function string.trim(a_string, flag)
    local len     = string.len(a_string)
    local i_start = 1
    local i_end   = len
    local is_empty = true

    if len == 0 then return a_string end

    -- remove leading blank chars
    if (flag == nil or flag == "lead") then
        for i = 1, len do
            local c = string.byte(a_string, i)
            if c ~= letCR and c ~= letLF and c ~= letSp and c ~= letTab then
                i_start = i
                is_empty = false
                break
            end
        end
    end

    if is_empty then return "" end

    -- remove trailing blank chars
    if (flag == nil or flag == "trail") then
        for i = len, 1, -1 do
            local c = string.byte(a_string, i)
            if c ~= letCR and c ~= letLF and c ~= letSp and c ~= letTab then
                i_end = i
                break
            end
        end
    end
    return string.sub(a_string, i_start, i_end)
end

function string.findnth(str, char, nth, from_end, n_start, plain)
    if (nth == nil) then nth = 1 end

    -- find from start
    if (from_end == nil) or (from_end == false) then
        local n = string.find(str, char, 1, true)
        local n_last = n
        local count = 1
        while n and ((nth > 0) or (count < nth)) do 
            n_last = n
            count = count + 1
            n = string.find(str, char, n_last+1, true)
        end
        if count ~= nth then 
            return nil 
        else 
            return n_last 
        end
    else -- find from end
        local len = string.len(str)
        local count = 0
        local n_last = nil
        for n = len, 1, -1 do
            local c = string.byte(str, n)
            if (c == char) then
                count = count + 1
                n_last = n
                if (nth > 0) and (count >= nth) then break end
            end
        end
        return n_last
    end
end

function string.afterfirst(str, char)
    local n = string.find(str, char, 1, true)
    if n == nil then return "" end
    return string.sub(str, n)
end

function string.afterlast(str, char)
    local n = string.find(str, char, 1, true)
    local n_last = n
    while n do 
        n_last = n
        n = string.find(str, char, 1, true)
    end
    
    if n_last == nil then return str end
    return string.sub(str, n_last)
end

function string.beforefirst(str, char)

end
-------------------------------------------------------------------------------
-- File and directory command prompt utilities
-------------------------------------------------------------------------------

function os.exists(filename)
    local file = io.open(filename, "r")
    if (file == nil) then return nil end
    io.close(file)
    return true
end

function os.cd(path)
    if wx.wxDirExists(path) then
        wx.wxSetWorkingDirectory(path)
    else 
        print("No such directory exists '"..path.."'")
    end
    os.pwd()
end

function os.fixpath(path)
    if path then 
        local sep = wx.wxFileSeparator()
        if (path == ".") or (path == "."..sep) then
            path = wx.wxGetCwd()..sep
        elseif (string.sub(path, 1, 2) == "..") or (string.sub(path, 1, 3) == ".."..sep) then
            local cwd = wx.wxGetCwd()
            print("Found")
            local i = string.find(cwd, sep, -1, true)
            print("Found1", i)
            local up_cwd = cwd
            if i then up_cwd = string.sub(cwd, 1, i-1) end
            print("Found2", up_cwd)
            if wx.wxDirExists(up_cwd) then
                path = up_cwd..sep..string.sub(path, 3)
            end
        end
    end
    return path
end

-- Get a lua table with table members dirs and files that contain the names
--   of the files
function GetDirectoryTable(path)
    local fileTable = { path = path, dirs = {}, files = {} }
    
    if wx.wxDirExists(path) then 
        local filename = wx.wxFindFirstFile(path, wx.wxDIR)
        while filename and string.len(filename) > 0 do
            table.insert(fileTable.dirs, filename)
            filename = wx.wxFindNextFile()
        end
    
        filename = wx.wxFindFirstFile(path, wx.wxFILE)
        while filename and string.len(filename) > 0 do
            table.insert(fileTable.files, wx.wxFileName(filename):GetFullName())
            filename = wx.wxFindNextFile()
        end
    end
    
    return fileTable
end

function os.dir(optionalPathFilter)
    local path = optionalPathFilter or (wx.wxGetCwd()..wx.wxFileSeparator())
    path = os.fixpath(path)

    local fileTable = GetDirectoryTable(path);

    print("Directory listing of '"..fileTable.path.."'")
    print("Size (bytes) Name")

    for n = 1, table.getn(fileTable.dirs) do
        print("      <DIR>  "..fileTable.dirs[n])
    end
    for n = 1, table.getn(fileTable.files) do
        print(string.format("%11d  %s", wx.wxFileSize(fileTable.files[n]), fileTable.files[n]))
    end
end

function os.pwd()
    print(wx.wxGetCwd()..wx.wxFileSeparator())
end

function os.cat(fileName)
    for linetext in io.lines(fileName) do
        print(linetext)
    end
end

-- skipFlag is "#" for skip comments "t" for trim or "b" for blank
function os.readToTable(filename, skipFlag)
    local linenumber = 0
    local filetable = {}

    local skip_comments = string.find(skipFlag or "", "#")
    local skip_blank = string.find(skipFlag or "", "b")
    local trim = string.find(skipFlag or "", "t")

    for linetext in io.lines(filename) do

        if (trim) then linetext = StringTrim(linetext) end

        if ((skip_blank and string.len(linetext) > 0) or (not skip_blank)) then
            if ((skip_comments and string.sub(linetext,1,1) ~= "#") or (not skip_comments)) then
                filetable[linenumber] = linetext
                linenumber = linenumber + 1
            end
        end
    end

    -- if (linenumber == 0) return nil end -- return nil for bad file

    return filetable
end

function Cmd_SetTitle( title_string, show_title )
--    local current_title, current_show
--    current_title, current_show = GetTitle()
end

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- teto lua table to store functions
-------------------------------------------------------------------------------

--teto = {}
--teto.about = "Hello, welcome to Plotto.lua. Written by John Labenski"
--teto.GetCanvas = GetCanvas
