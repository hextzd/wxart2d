#!/bin/sh

SAXON=/d/tools/saxon/saxon.exe

usage()
{
    echo generate tar gz distribution and documentation for wxArt2D.
    echo start this script 
    echo For example: 
    echo Usage: makedist.sh wxArt2Dsourcedir DESTINATIONDIRdir
    echo $1 default to wxArt2D and $2 to distribution
    exit 1
}

doreplace()
{
    thefile=$1
    theexpr=$2

    if [ -f $thefile ]; then
      sed -e "$theexpr" < $thefile > $thefile.tmp
      mv $thefile.tmp $thefile
    else
      echo "*** $thefile not found."
    fi
}

gendoc()
{
    cd $DESTINATIONDIR/wxArt2D-${WARTVER}/doc
    confile=$DESTINATIONDIR/wxArt2D-${WARTVER}/doc/doxygen.doxy
    echo create doxygen documentation
    PATH=/d/tools/Graphviz/bin:d/tools/doxygen/bin:$PATH
    cp $SOURCEDIR/doc/doxygen.in $confile
    doreplace $confile "s%@wxart2d_SOURCE_DIR@%../../wxArt2D-${WARTVER}%"
    doreplace $confile "s%@DOT_PATH@%d:/tools/Graphviz/bin%"
    doxygen doxygen.doxy > doxygenwarnings.txt 2>&1
    #doxygen doxygen.doxy
    
    $SAXON -o ./html/canvas.html ./docbook/canvasbt.xml d:/tools/docbook/docbook-xsl-1.61.3/html/klaas2.xsl 
    cd html
    $SAXON ../docbook/canvasbt.xml d:/tools/docbook/docbook-xsl-1.61.3/html/linkclass.xsl    
    cd $DESTINATIONDIR
}


WARTVER=1.0

SOURCEDIR=$1
DESTINATIONDIR=$2

if [ "$SOURCEDIR" = "" ] ; then
    SOURCEDIR=`pwd`/wxArt2D
fi
if [ "$DESTINATIONDIR" = "" ] ; then
    DESTINATIONDIR=`pwd`/distribution
fi
 
echo  generate from:   $SOURCEDIR
echo  build in:  $DESTINATIONDIR

if ! test -d "$SOURCEDIR" ; then
    usage
fi    
if ! test -d "$DESTINATIONDIR" ; then
    usage
fi

# generate a file of all needed files in distribution
# skip all CVS directories and *.bak and .#* files
#find $SOURCEDIR -name CVS -prune -o -name *.bak -o -name .#* -o -type f  -print > $DESTINATIONDIR/filelist.txt
echo create filelist from $SOURCEDIR
cd $SOURCEDIR
find . -name CVS -prune -o -name *.bak -o -name .#* -o -type f -print  > $DESTINATIONDIR/filelist.txt

#some more filtering
cat $DESTINATIONDIR/filelist.txt | grep -v propeditoops |\
grep -v apps/mich2 | grep -v apps/tetocad | \
grep -v apps/thumbo | grep -v SVGSAMPLES | \
grep -v apps/template > $DESTINATIONDIR/filelistfilter.txt

# create a tar archive containing all files in fileslist.txt 
echo tar filelist to $DESTINATIONDIR/wxArt2D-${WARTVER}.tar
rm -f $DESTINATIONDIR/wxArt2D-${WARTVER}.tar 
tar cf $DESTINATIONDIR/wxArt2D-${WARTVER}.tar -T $DESTINATIONDIR/filelistfilter.txt

#extract to generate docs for it.
cd $DESTINATIONDIR
echo untar to version directory
mkdir wxArt2D-${WARTVER}
cd wxArt2D-${WARTVER}
tar xf ../wxArt2D-${WARTVER}.tar
cd ..
rm wxArt2D-${WARTVER}.tar

#generate doxygen and docbook docs
gendoc

echo tar and gzip this version directory
rm -rf wxArt2D-${WARTVER}.tar 
tar cf wxArt2D-${WARTVER}.tar wxArt2D-${WARTVER}/*
gzip wxArt2D-${WARTVER}.tar 



