#! /bin/sh

replace_pragma()
{
    
    LINE=`grep "#ifdef __GNUG__" $FILE`
    if test "x$LINE" != x ; then
        LINE=`grep "#pragma interface" $FILE`
        if test "x$LINE" != x ; then
            echo Adding NO_GCC_PRAGMA to file $FILE
            sed "s/#ifdef __GNUG__/#if defined(__GNUG__) \&\& !defined(NO_GCC_PRAGMA)/1" $FILE > $FILE.tmp
            sed "s/#if defined(__GNUG__) \&\& !defined(__APPLE__)/#if defined(__GNUG__) \&\& !defined(NO_GCC_PRAGMA)/1" $FILE.tmp > $FILE
	    rm $FILE.tmp
        fi
    fi
}

search_dir()
{
    for FILE in *; do
        if test -d $FILE; then
#            echo Changing directory to $FILE
            cd $FILE
            search_dir
            cd ..
#            echo Changing directory to ..
        fi
    done
    for FILE in *.h; do
        if test -a $FILE; then
            replace_pragma
        fi
    done
}

search_dir
