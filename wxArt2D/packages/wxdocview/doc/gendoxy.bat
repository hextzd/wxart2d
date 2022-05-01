set graphviz=C:\data\tools\Graphviz
set doxygen=C:\data\tools\doxygen

path=%graphviz%\bin;%doxygen%\bin;%path%

doxygen doxygen.doxy > aap.txt 2>&1

pause
