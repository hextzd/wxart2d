#!/bin/sh


# \file distrib/buildconf.sh
# \author Erik van der Wal
#
#     Copyright: 2000-2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: buildconf.sh,v 1.4 2004/06/25 21:54:23 kire_putsje Exp $



# strip wxArt2D version from makedist.sh
# grep the line containing the version from makedist.sh
#     WARTVER=1.0\r
# then get rid of all non-printable characters (\r \n)
#     WARTVER=1.0
# Split this in a part befor and after the = sign
# Use the second part as the version number.

grep "WARTVER=" makedist.sh | tr -c -d [:print:] \
| awk "{
    split(\$1,txt,\"=\");
    print \"AC_INIT(wxArt2D, \" txt[2] \", wxart2d-users_dev@lists.sourceforge.net)\"
}" > autoconfversion

# Strip options from a CmakeLists.txt and prepare them for autoconf
#
# options in the CMakeLists.txt appear as e.g.:
# OPTION( WXART2D_USE_EDITOR "Enable Editor module" ON )
# first we grep the OPTION lines and filter out WXART2D_DEPENDENT_OPTION lines
# to be able to get the string, this line is split in three parts:
#     1: OPTION( WXART2D_USE_EDITOR
#     2: Enable Editor module
#     3: ON )
# the line is also split by awk in words, separated by spaces.
#
# Depending on the one-but-last word, which represents the default setting
# we set either a enable/disable string, later to be used for the help.
# then the other settings are printed.
# The output stream of the first awk will now look like:
#
#     disable WXART2D_USE_EDITOR
#     Enable Editor module
#     Enable Editor module
#     WXART2D_USE_EDITOR
#
# In this stream, the first and second/third are used for the help message,
# the fourth will be used for as a cmake option
#
# In the help, we only want to display the basic option name, so we
# have to strip things like WXART2D_USE_ with:
#     s/WXART2D_\(USE\|COMP\)_//        = search/replace
#     s/_DRAWER//
# While we are at it, also move to lowercase.
#     y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/
# The help string will be transformed in an enable/disable version:
#     n
#     s/Disable/Enable/
#     n
#     s/Enable/Disable/
# Leave the last line as is: n.
#
# So, now the output stream will look like:
#
#     disable editor
#     Enable Editor module
#     Disable Editor module
#     WXART2D_USE_EDITOR
#
# With these lines, the following autoconf macro can be generated
#
#     AC_ARG_ENABLE(editor, AC_HELP_STRING([--disable-editor],[Disable Editor module]),
#       [
#         if test \"$enableval\" = yes; then
#           CMAKE_OPTIONS=\"${CMAKE_OPTIONS} -DWXART2D_USE_EDITOR:BOOLEAN=TRUE\"
#         else
#           CMAKE_OPTIONS=\"${CMAKE_OPTIONS} -DWXART2D_USE_EDITOR:BOOLEAN=FALSE\"
#         fi
#       ]
#     )
#
# All options will be parsed this way and dumped to the file: autoconfoptions

grep OPTION ../CMakeLists.txt | grep -v DEPENDENT \
| awk "{
    split(\$0,txt,\"\\\"\");
    if (\$(NF-1)!=\"ON\")
        printf \"enable \"
    else
        printf \"disable \"
    print \$2
    print txt[2]
    print txt[2]
    print \$2
}" \
| sed -e "s/WXART2D_\(USE\|COMP\)_//
s/_DRAWER//
y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/
n
s/Disable/Enable/
n
s/Enable/Disable/
n" | awk "{
    print \"AC_ARG_ENABLE(\" \$2 \",\"
    printf \"  AC_HELP_STRING([--\" \$1 \"-\" \$2 \"],[\"
    if (\$1!=\"disable\")
    {
        getline
        print \$0 \"]), \"
	getline
    }
    else
    {
        getline
	getline
        print \$0 \"]), \"
    }    
    getline
    print \"  [\"
    print \"    if test \\\"\$enableval\\\" = yes; then\"
    print \"      CMAKE_OPTIONS=\\\"\${CMAKE_OPTIONS} -D\" \$0 \":BOOLEAN=TRUE\\\"\"
    print \"    else\"
    print \"      CMAKE_OPTIONS=\\\"\${CMAKE_OPTIONS} -D\" \$0 \":BOOLEAN=FALSE\\\"\"
    print \"    fi\"
    print \"  ]\"
    print \")\"
}" > autoconfoptions

# Now we have to place the version and options in the configure.ac file.
# We start by having allready a template file: configure.ac.in
# In this file there is a specific line, where all option should be placed
# This line is searched here and replaced by the contents of the autoconfoptions file.
# This same procedure is also followed for the version
sed -e "/#wxart2d version info will be placed here/r autoconfversion
/#All wxart2d options will be placed here/r autoconfoptions" configure.ac.in > configure.ac

# Now we do not need the autoconfversion/options file anymore.
rm autoconfversion
rm autoconfoptions

# Run autoconf, to generate the configure script from configure.ac
autoconf

# Clean up again
rm -r autom4te.cache
rm configure.ac

# Move configure file to base directory.
mv configure ..

