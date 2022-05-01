#!/bin/sh

SOURCEDIR="./"

# generate a file of all needed files in distribution
# skip all CVS directories and *.bak and .#* files
#find $SOURCEDIR -name CVS -prune -o -name *.bak -o -name .#* -o -type f  -print > $DESTINATIONDIR/filelist.txt
echo create filelist from $SOURCEDIR
cd $SOURCEDIR
find . -name CVS -prune -o -name *.bak -o -name .#* -o -type f -print  > $SOURCEDIR/filelist.txt

 
#some more filtering
cat $SOURCEDIR/filelist.txt | \
grep -E '*.txt$|*.cpp$|*.h$' > $SOURCEDIR/filelistfilter.txt
mv $SOURCEDIR/filelistfilter.txt $SOURCEDIR/filelist.txt 


#some more filtering
cat filelist.txt | \
grep -E '.*\.cpp$|.*\.h$|.*\.inl$' > filelistfilter.txt
mv filelistfilter.txt filelist.txt 


filelist=`cat $SOURCEDIR/filelist.txt`

for i in $filelist
do 
    echo $i 
    echo " "
    #dos2unix $i
    #fromdos $i
    astyle -k1 -W1 --indent-switches --convert-tabs --indent-preprocessor --lineend=linux --style=ansi --suffix=none -o -O -v -S -p -D $i    
done 
