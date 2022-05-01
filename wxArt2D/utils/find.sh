#!/bin/sh

SOURCEDIR="./"

REPLACEFILE=$1

#sed -f $FILE
doreplacefile()
{
    thefile=$1
    theexpr=$2

    if [ -f $thefile ]; then
          sed -f $REPLACEFILE   < $thefile > $thefile.tmp
          mv $thefile.tmp $thefile
    else
          echo "*** $thefile not found."
    fi
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

dofind()
{
    thefile=$1
    theexpr=$2

    if [ -f $thefile ]; then
          grep -e "$theexpr" $thefile
    else
          echo "*** $thefile not found."
    fi
}

# generate a file of all needed files in distribution
# skip all CVS directories and *.bak and .#* files
#find $SOURCEDIR -name CVS -prune -o -name *.bak -o -name .#* -o -type f  -print > $DESTINATIONDIR/filelist.txt
echo create filelist from $SOURCEDIR
cd $SOURCEDIR
find . -name CVS -prune -o -name *.bak -o -name .#* -o -type f -print  > $SOURCEDIR/filelist.txt

#some more filtering
cat $SOURCEDIR/filelist.txt | \
grep -v apps/tetocad | \
grep -v apps/thumbo | grep -v SVGSAMPLES | \
grep -v apps/template > $SOURCEDIR/filelistfilter.txt
mv $SOURCEDIR/filelistfilter.txt $SOURCEDIR/filelist.txt 

#some more filtering
cat $SOURCEDIR/filelist.txt | \
grep -E '*.cpp$|*.h$' > $SOURCEDIR/filelistfilter.txt
mv $SOURCEDIR/filelistfilter.txt $SOURCEDIR/filelist.txt 

#doreplace $filetoreplacein "s%string1%string2%"

filelist=`cat $SOURCEDIR/filelist.txt`

for i in $filelist
do 
    echo $i 
    echo " "
    dofind $i "class "
done 