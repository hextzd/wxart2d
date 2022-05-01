#!/bin/sh
sort -r $1 > $1.tmp
mv $1.tmp $1
awk '{ 
        print "s+\\(^\\|[^a-zA-Z]*\\)"$1"\\([^a-zA-Z]\\|\$\\)+\\1"$2"\\2+g" 
}'   $1