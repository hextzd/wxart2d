#!/thales/thales_localwork/soft/trunk/wxArt2D/bin/lua
-- ..\trunk\wxArt2D\bin\lua ..\trunk\wxArt2D\configure.lua -g

-- add some relative paths to find lfs
package.cpath = package.cpath .. ";../../../bin/?.so" .. ";../../../trunk/wxArt2D/bin/?.so" .. ";../trunk/wxArt2D/bin/?.so"

require "lfs"

function is_msw()
    return platform2().windows 
end

function platform2()

local detected = { unix=false, freebsd=false, bsd=false, windows=false, mingw32=false, macos=false }
    if os.getenv("WINDIR") then
        detected.windows = true
    else
        system = io.popen("uname -s"):read("*l")
        if system == "FreeBSD" then
           detected.unix = true
           detected.freebsd = true
           detected.bsd = true
        elseif system == "OpenBSD" then
           detected.unix = true
           detected.openbsd = true
           detected.bsd = true
        elseif system == "Darwin" then
           detected.unix = true
           detected.macosx = true   
           detected.bsd = true
        elseif system == "Linux" then   
           detected.unix = true   
           detected.linux = true
        elseif system and system:match("^CYGWIN") then   
           detected.unix = true   
           detected.cygwin = true
        elseif system and system:match("^Windows") then   
            detected.windows = true
        elseif system and system:match("^MINGW") then   
            detected.mingw32 = true
        else
            detected.unix = true   -- Fall back to Unix in unknown systems.end
        end
    end    
    return detected
end

function platform()
    local USE_POPEN= false

    if rawget(io,"popen") then
        --
        local ok,retval= pcall( io.popen, "echo" )  -- "protected call" (= don't nag to user)
        if ok and retval then
            retval:close()
            USE_POPEN= true
        end
    end

    local uname
    if os.getenv("WINDIR") then
        HOST = "win32"
    elseif USE_POPEN then
        local f= io.popen "uname"
        uname= f:read'*a'
        ASSUME(uname, "'uname' did not work! (should have)")
        
        f:close()
        if string.sub( uname,-1 )=='\n' then
            uname= string.sub( uname, 1,-2 )    -- remove terminating newline
        end

        if string.find(uname,"BSD") then    -- "NetBSD"/"FreeBSD"
            HOST= "bsd"
        else
            HOST= string.lower(uname)   -- "linux"/"darwin"/"qnx"
        end
    else
        HOST= os.getenv("OSTYPE") or    -- "darwin" (if no popen)/"msys"/..
                error "Unknown OS! (or 'io.popen()' not enabled)"
    end 
    return HOST
end

function capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

-- Return true if file exists and is readable.
function file_exists(path)
  local file = io.open(path, "rb")
  if file then file:close() end
  return file ~= nil
end

-- Return true if directory exists
function dir_exists( path )
    local atrr = lfs.attributes( path ) 
    if ( atrr and atrr.mode == "directory" ) then
        return true
    end
    return false
end

function create_dir( relpath )
    -- os.execute( "mkdir "..relpath )
    lfs.mkdir( relpath )
end

function currentdir()
--[[
  os.execute("cd > cd.tmp") 
  local f = io.open("cd.tmp", r) 
  local cwd = f:read("*a") 
  f:close() 
  os.remove("cd.tmp") 
  return cwd 
--]]
    return lfs.currentdir()
end

function make( whichlib, whichpackage )
    if ( is_msw() ) then
        if ( CMAKE_GENERATOR_INTERNAL == VCversion ) then 
            if ( file_exists( "C:/Program Files/Common Files/Microsoft Shared/MSEnv/VSLauncher.exe" ) ) then
                os.execute( "\"C:/Program\ Files/Common\ Files/Microsoft\ Shared/MSEnv/VSLauncher.exe\" "..whichpackage..".sln")
                if  not file_exists( whichlib ) then
                    print( "After you compiled "..whichpackage.." package with VC studio project run this script again." )
                    os.exit(0)
                end
            else
                print( "Now first compile "..whichpackage.." package VC studio project and run this script again." )
                os.exit(0)
            end
        elseif ( CMAKE_GENERATOR_INTERNAL == "NMake Makefiles" ) then 
            os.execute( "nmake -f Makefile" )
        elseif ( CMAKE_GENERATOR_INTERNAL == "MinGW Makefiles" ) then             
            os.execute( "mingw32-make -f Makefile" )
        end
    else
        os.execute( "make" )
    end    
end

function answer( defaultYes )
    defaultYes = defaultYes or true
    local reply = io.stdin:read'*l'
    if reply ~= "y" then
        if defaultYes and reply == "" then
            return true
        end
        return false
    end
    return true
end

-- estimate generator and flags based on path else ask
function WhichGenerator( path )
    if is_msw() then
        if string.find( path, "vc_mswud$" ) then 
            CMAKE_GENERATOR_INTERNAL = VCversion
            wxdebug="ON"
            wxunicode="ON"            
        elseif string.find( path, "vc_mswu$" ) then 
            CMAKE_GENERATOR_INTERNAL = VCversion
            wxdebug="OFF"
            wxunicode="ON"            
        elseif string.find( path, "vc_mswd$" ) then 
            CMAKE_GENERATOR_INTERNAL = VCversion
            wxdebug="ON"
            wxunicode="OFF"            
        elseif string.find( path, "vc_msw$" ) then 
            CMAKE_GENERATOR_INTERNAL = VCversion
            wxdebug="OFF"
            wxunicode="OFF"            
        elseif string.find( path, "nmake_mswud$" ) then 
            CMAKE_GENERATOR_INTERNAL="NMake Makefiles"
            wxdebug="ON"
            wxunicode="ON"            
        elseif string.find( path, "nmake_mswu$" ) then 
            CMAKE_GENERATOR_INTERNAL="NMake Makefiles"
            wxdebug="OFF"
            wxunicode="ON"            
        elseif string.find( path, "nmake_mswd$" ) then 
            CMAKE_GENERATOR_INTERNAL="NMake Makefiles"
            wxdebug="ON"
            wxunicode="OFF"            
        elseif string.find( path, "nmake_msw$" ) then 
            CMAKE_GENERATOR_INTERNAL="NMake Makefiles"
            wxdebug="OFF"
            wxunicode="OFF"            
        elseif string.find( path, "mingw_mswud$" ) then 
            CMAKE_GENERATOR_INTERNAL="MinGW Makefiles"
            wxdebug="ON"
            wxunicode="ON"            
        elseif string.find( path, "mingw_mswu$" ) then 
            CMAKE_GENERATOR_INTERNAL="MinGW Makefiles"
            wxdebug="ON"
            wxunicode="ON"            
        elseif string.find( path, "mingw_mswd$" ) then 
            CMAKE_GENERATOR_INTERNAL="MinGW Makefiles"
            wxdebug="ON"
            wxunicode="OFF"            
        elseif string.find( path, "mingw_msw$" ) then 
            CMAKE_GENERATOR_INTERNAL="MinGW Makefiles"
            wxdebug="ON"
            wxunicode="OFF"            
        else
        	print( "Give CMAKE_GENERATOR_INTERNAL:" )
            CMAKE_GENERATOR_INTERNAL=io.stdin:read()
        end
    else
        if string.find( path, "gcc_gtkd$" ) then 
	        CMAKE_GENERATOR_INTERNAL="Unix Makefiles"
            wxdebug="ON"
            wxunicode="OFF"            
        elseif string.find( path, "gcc_gtk$" ) then 
	        CMAKE_GENERATOR_INTERNAL="Unix Makefiles"
            wxdebug="OFF"
            wxunicode="OFF"            
        elseif string.find( path, "gcc_gtkud$" ) then 
	        CMAKE_GENERATOR_INTERNAL="Unix Makefiles"
            wxdebug="ON"
            wxunicode="ON"            
        elseif string.find( path, "gcc_gtku$" ) then 
	        CMAKE_GENERATOR_INTERNAL="Unix Makefiles"
            wxdebug="OFF"
            wxunicode="ON"            
        else
        	print( "Give CMAKE_GENERATOR_INTERNAL:" )
            CMAKE_GENERATOR_INTERNAL=io.stdin:read()
        end
    end
end

--############################################################################

VCversion="Visual Studio 9 2008"
installprefix="/usr/local"
verbose="OFF"
CWD=currentdir () 
CMAKE="cmake"
--CMAKE="cmake-gui"

wxdebug="ON"
wxstatic="ON"
wxunicode="ON"
WXWIN=""

if platform2().unix then
	local f = io.popen("wx-config --version") -- store the output in a "file"	
	wxversionfull = f:read("*a")
	wxversionfull = string.gsub( wxversionfull, "\n","" )
	wxversion = string.gsub( wxversionfull, '%.%d*$',"" )
	print( "wx-config --version found:"..wxversionfull.." use:"..wxversion )    -- print out the "file"'s content
else
	WXWIN=os.getenv("WXWIN")
	wxversion="2.9"
	wxversion="2.8"
    print( "wxWidgets version to search is set to: "..wxversion.."\n and WXWIN is set to: "..WXWIN.."\n is that right (y/n):" )
    if not answer() then
        os.exit(1)
    end
end

-- all options
local i = 1
while arg[i] do
    argi = arg[i]
    if argi == "-h" or argi == "-help" then 
        print("help")
        print( "configure wxArt2D" )
        print( "-p |--prefix pathToInstallDir :for a different install directory (default /usr/local)" )
        print( "-g Use GUI with cmake-gui else cmake is used):" )
        print( "-v|--verbose :generate verbose makefiles" )
        print( "-d|--wxdebug ON/OFF: enable/disable debug on wxWidgets" )
        print( "-s|--wxstatic ON/OFF: static or shared on wxWidgets" )
        print( "-u|--wxunicode ON/OFF: enable/disable wxWidgets" )
        print( "example: ../trunk/wxArt2D/configure --prefix /thales/thales_localwork/soft/usr/local -v -g" )
        print( "example: ../trunk/wxArt2D/configure --prefix /thales/thales_localwork/soft/usr/local -v -g -u OFF -s ON -d ON" )
        os.exit(0)
    elseif argi == "-prefix" or argi == "--prefix" then 
        i = i + 1        
        installprefix= arg[i]
    elseif argi == "-c" or argi == "--clean" then 
        clean="ON"        
    elseif argi == "-g" then 
        CMAKE="cmake-gui"
    elseif argi == "-v" or argi == "--verbose" then 
        verbose="ON"        
    elseif argi == "-wx" or argi == "--wxversion" then 
        i = i + 1        
        wxversion=arg[i]
    elseif argi == "-d" or argi == "--wxdebug" then 
        i = i +1  
        if arg[i] == "ON" or arg[i] == "OFF" then
            wxdebug=arg[i]
        else
            print "wrong --wxdebug option, should be ON or OFF"    
            os.exit(0)
        end    
    elseif argi == "-s" or argi == "--wxstatic" then 
        i = i + 1        
        if arg[i] == "ON" or arg[i] == "OFF" then
            wxstatic=arg[i]
        else
            print "wrong --wxstatic option, should be ON or OFF"    
            os.exit(0)
        end    
    elseif argi == "-u" or argi == "--wxunicode" then 
        i = i + 1        
        if arg[i] == "ON" or arg[i] == "OFF" then
            wxunicode=arg[i]
        else
            print "wrong --wxunicode option, should be ON or OFF"    
            os.exit(0)
        end   
    else
            print "UnKnow option $1 for usage type: configure --help"
            os.exit(0)
    end    
    i = i + 1    
end

if clean then
    lfs.rmdir( CWD.."/packages" )
    lfs.rmdir( CWD.."/thirdparty" )
    os.exit(1)
end

if ( os.execute( "cmake --version"  )  ~=  0  ) then 
    os.exit(1)
    print("Error: Make sure cmake is found in your path.")
else
    print("CMake is found")
end 

if platform2().unix then
    wxWidgets_PFVERSION = wxversion
else
    wxWidgets_PFVERSION = string.gsub( wxversion, "%.", "" )
end

WhichGenerator( CWD )
print( "The CMake generator to use is: "..CMAKE_GENERATOR_INTERNAL )
print( "Debug setting is: "..wxdebug )
print( "Unicode setting is: "..wxunicode )

if ( wxdebug == "ON" ) then 
    wxWidgets_DBG = "d"       -- contains "d" when debug is required  
    if ( is_msw() ) then 
        if ( CMAKE_GENERATOR_INTERNAL == VCversion ) then 
            wxWidgets_COMPDIR = "Debug"
        elseif ( CMAKE_GENERATOR_INTERNAL == "NMake Makefiles" ) then 
            wxWidgets_COMPDIR = ""
        elseif ( CMAKE_GENERATOR_INTERNAL == "MinGW Makefiles" ) then             
            wxWidgets_COMPDIR = ""
        end    
    else    
        wxWidgets_COMPDIR = ""
    end    
else
    wxWidgets_DBG = ""       -- contains "d" when debug is required  
    if ( is_msw() ) then 
        if ( CMAKE_GENERATOR_INTERNAL == VCversion ) then 
            wxWidgets_COMPDIR = "Release"
        elseif ( CMAKE_GENERATOR_INTERNAL == "NMake Makefiles" ) then 
            wxWidgets_COMPDIR = ""
        elseif ( CMAKE_GENERATOR_INTERNAL == "MinGW Makefiles" ) then             
            wxWidgets_COMPDIR = ""
        end    
    else    
        wxWidgets_COMPDIR = ""
    end    
end

if ( wxunicode == "ON" ) then 
    wxWidgets_UCD = "u"       -- contains "d" when debug is required  
else
    wxWidgets_UCD = ""       -- contains "d" when debug is required  
end

if ( is_msw() ) then 
    wxWidgets_PORT = "msw"     -- port of wxWidgets ( msw, gtk2, gtk etc. )
else
    wxWidgets_PORT = "gtk2"     -- port of wxWidgets ( msw, gtk2, gtk etc. )
end

wxWidgets_UNV  = ""        -- contains "unv" when universal wxWidgets is required

--[[
if "`which ${CMAKE}`" == "" then
    print( CMAKE.." not found, set path to it. " )
    exit(1)
fi

if "`which wx-config`" == "" then
    print( "wx-config not found \nIf you compiled and installed wxWidgets, set the path to it.\nElse compile and install wxWidgets first." )
    exit(1)
else
    print( "wx-config --selected-config will be used as default: `wx-config --selected-config` " )
fi
--]]

print( "build in: "..CWD )
print( "Is this the directory to build in (y/n):" )
if not answer() then
    os.exit(1)
end

-- relative to trunk e.g c:\data\art2d\trunk\wxArt2D\build\msw\vc_mswud
local wxArt2D_root = CWD.."/../../../../wxArt2D"
print( "Source from: "..wxArt2D_root )
if  not dir_exists( wxArt2D_root ) then
    wxArt2D_root=CWD.."/../../../trunk/wxArt2D" -- outside build at same level
    if  not dir_exists( wxArt2D_root ) then
        wxArt2D_root=CWD.."/../trunk/wxArt2D" -- outside build single builddir at same level
        if  not dir_exists( wxArt2D_root ) then
            wxArt2D_root = os.getenv("wxArt2D_root")
            if not dir_exists( wxArt2D_root ) then
                print( "Where is the source? ( e.g. ../trunk/wxArt2D):" )
                wxArt2D_root = io.stdin:read()
                if  not dir_exists( wxArt2D_root ) then
                    print( "did not find the source in: "..wxArt2D_root )
                    os.exit(1)
                end
            end    
        end    
    end
end

print( "Source from: "..wxArt2D_root )
print( "Creating directories to build in." )

local builddir = "./thirdparty"
if  not dir_exists( builddir ) then
    create_dir( "thirdparty" )
end
if  not dir_exists( builddir.."/kbool" ) then
    create_dir( "thirdparty/kbool" )
end
if  not dir_exists( builddir.."/agg-2.4" ) then
    create_dir( "thirdparty/agg-2.4" )
end
builddir = "./packages"
if  not dir_exists( builddir ) then
    create_dir( "packages" )
end
if  not dir_exists( builddir.."/wxdocview" ) then
    create_dir( "packages/wxdocview" )
end
if  not dir_exists( builddir.."/wxart2d" ) then
    create_dir( "packages/wxart2d" )
end

--CMAKE_PARS="-DCMAKE_VERBOSE_MAKEFILE="..verbose.." -DCMAKE_INSTALL_PREFIX="..installprefix
local cachefilestr = ""
--if CMAKE == "cmake-gui" then
    CMAKE_PARS=""
    cachefilestr="//Install path prefix, prepended onto install directories.\n"
    cachefilestr=cachefilestr.."CMAKE_INSTALL_PREFIX:PATH="..installprefix.."\n"
    cachefilestr=cachefilestr.."CMAKE_VERBOSE_MAKEFILE:BOOL="..verbose.."\n"
    cachefilestr=cachefilestr.."CMAKE_GENERATOR:INTERNAL="..CMAKE_GENERATOR_INTERNAL.."\n"
    cachefilestr=cachefilestr.."wxWidgets_IN_PFVERSION:STRING="..wxWidgets_PFVERSION.."\n"
    cachefilestr=cachefilestr.."wxWidgets_IN_PORT:STRING="..wxWidgets_PORT.."\n"
    cachefilestr=cachefilestr.."wxWidgets_IN_USE_DEBUG:BOOL="..wxdebug.."\n"
    cachefilestr=cachefilestr.."wxWidgets_IN_USE_STATIC:BOOL="..wxstatic.."\n"
    cachefilestr=cachefilestr.."wxWidgets_IN_USE_UNICODE:BOOL="..wxunicode.."\n"
--end

print( CMAKE_PARS )

wxkboollib = CWD.."/thirdparty/kbool/lib/"..wxWidgets_COMPDIR.."/kbool.lib"

if  not file_exists( wxkboollib ) then
    lfs.chdir( CWD.."/thirdparty/kbool" )
    print( "build kbool from "..wxArt2D_root.."/thirdparty/kbool in:"..currentdir () )
    cachefile = io.open( "CMakeCache.txt", "a" )
    if ( not cachefile ) then
        print( "cache file not found" )
    end 
    cachefile:write( cachefilestr )
    cachefile:close()
    print( CMAKE.." "..wxArt2D_root.."/thirdparty/kbool "..CMAKE_PARS )
    os.execute( CMAKE.." "..wxArt2D_root.."/thirdparty/kbool "..CMAKE_PARS )
    make( wxkboollib, "kbool" )
else
    print( "kbool compiled and library ready" )
end

wxagglib = CWD.."/thirdparty/agg-2.4/lib/"..wxWidgets_COMPDIR.."/agg.lib"

if  not file_exists( wxagglib ) then
    lfs.chdir( CWD.."/thirdparty/agg-2.4" )
    print( "build agg-2.4 from "..wxArt2D_root.."/thirdparty/agg-2.4 in:"..currentdir () )
    cachefile = io.open( "CMakeCache.txt", "a" )
    if ( not cachefile ) then
        print( "cache file not found" )
    end 
    cachefile:write( cachefilestr )
    cachefile:close()
    print( CMAKE.." "..wxArt2D_root.."/thirdparty/agg-2.4 "..CMAKE_PARS )
    os.execute( CMAKE.." "..wxArt2D_root.."/thirdparty/agg-2.4 "..CMAKE_PARS )
    make( wxagglib, "antigrain" )
else
    print( "agg-2.4 compiled and library ready" )
end

if platform2().unix then
	--libwxdocview_gtk2d_docview-2.8.a
	wxdocviewlib = CWD.."/packages/wxdocview/lib/libwxdocview_"..wxWidgets_PORT..wxWidgets_DBG.."_docview-"..wxWidgets_PFVERSION..wxWidgets_UCD..".a"
else
	wxdocviewlib = CWD.."/packages/wxdocview/lib/"..wxWidgets_COMPDIR.."/wxdocview_"..wxWidgets_PORT..wxWidgets_PFVERSION..wxWidgets_UCD..wxWidgets_DBG.."_docview.lib"
end

--print(  wxdocviewlib )

if  not file_exists( wxdocviewlib ) then
    lfs.chdir( CWD.."/packages/wxdocview" )
    print( "build wxdocview from "..wxArt2D_root.."/packages/wxdocview in:"..currentdir () )
    cachefile = io.open( "CMakeCache.txt", "a" )
    if ( not cachefile ) then
        print( "cache file not found" )
    end 
    cachefile:write( cachefilestr )
    cachefile:close()
    print( CMAKE.." "..wxArt2D_root.."/packages/wxdocview "..CMAKE_PARS )
    os.execute( CMAKE.." "..wxArt2D_root.."/packages/wxdocview "..CMAKE_PARS )
    make( wxdocviewlib, "wxdocview" )
else
    print( "wxdocview package compiled and library ready" )
end

if platform2().unix then
	wxart2dlib = CWD.."/packages/wxart2d/lib/libwxart2d_"..wxWidgets_PORT..wxWidgets_DBG.."_canvas-"..wxWidgets_PFVERSION..wxWidgets_UCD..".a"
else
	wxart2dlib = CWD.."/packages/wxart2d/lib/"..wxWidgets_COMPDIR.."/wxart2d_"..wxWidgets_PORT..wxWidgets_PFVERSION..wxWidgets_UCD..wxWidgets_DBG.."_canvas.lib"
end

if  not file_exists( wxart2dlib ) then
    lfs.chdir( CWD.."/packages/wxart2d" )
    print( "build wxart2d from "..wxArt2D_root.."/packages/wxart2d in:"..currentdir () )
    cachefile = io.open( "CMakeCache.txt", "a" )
    if ( not cachefile ) then
        print( "cache file not found" )
    end 
    cachefile:write( cachefilestr )
    cachefile:close()
    print( CMAKE.." "..wxArt2D_root.."/packages/wxart2d "..CMAKE_PARS )
    os.execute( CMAKE.." "..wxArt2D_root.."/packages/wxart2d "..CMAKE_PARS )
    make( wxart2dlib, "wxart2d" )
else
    print( "wxart2d package compiled and library ready" )
end

if ( is_msw() ) then 
	print( "\nAll done\nThe VC project contains samples: "..CWD.."/packages/wxart2d/wxart2d.sln" )
	print( "For your own application, the place to start is: "..CWD.."/packages/wxart2d/myapp" )
else
	print( "\nAll done\nsamples are in: "..CWD.."/packages/wxart2d/samples" )
	print( "For your own application, the place to start is: "..CWD.."/packages/wxart2d/myapp" )	
end

--[[
read -p "Install all packages now? (y/n):"
if [ "$REPLY" != "y" ] && [ "$REPLY" != "" ] ; then
    exit
fi
SUDO=""
read -p "Do you want to do it using sudo? (y/n):"
if [ "$REPLY" = "y" ] && [ "$REPLY" != "" ] ; then
    SUDO=sudo
    echo "Install packages as sudo make install"
fi
echo "Install packages"

echo "install kbool from $CWD/thirdparty/kbool"
cd $CWD/thirdparty/kbool 
$SUDO make install
echo "install Agg from $CWD/thirdparty/agg-2.4"
cd $CWD/thirdparty/agg-2.4
$SUDO make install
echo "install wxdocview from $CWD/packages/wxdocview"
cd $CWD/packages/wxdocview
$SUDO make install
echo "install wxart2d from $CWD/packages/wxart2d"
cd $CWD/packages/wxart2d
$SUDO make install

echo "All done"
--]]


