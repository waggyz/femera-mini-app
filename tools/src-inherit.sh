#!/bin/bash
#
DOT=build/src-inherit.dot
echo digraph \"inheritance tree\" \{ >$DOT
echo    overlap=scale\; >>$DOT
echo     size=\"8,10\"\; >>$DOT
echo     ratio=\"fill\"\; >>$DOT
echo     fontsize=\"16\"\; >>$DOT
echo     fontname=\"Helvetica\"\; >>$DOT
echo         clusterrank=\"local\"\; >>$DOT
#
sed -n 's/^[ ]*class \([A-Z][a-z0-9]*\)\([ :a-z0-9]*\)\([A-Z][a-z0-9]*\).*/\"\1\" -> \"\3\"/p' \
 src/*.hpp src/*/*.hpp >>$DOT
#
echo '{ rank = sink; "perf"; }' >>$DOT

echo '{ rank = sink; "math"; }' >>$DOT
echo '{ rank = sink; "type"; }' >>$DOT
echo '{ rank = sink; "data-type"; }' >>$DOT
echo '{ rank = sink; "vals"; }' >>$DOT

echo '{ rank = sink; "Task"; }' >>$DOT
echo '{ rank = sink; "Flog"; }' >>$DOT
#
echo \} >>$DOT
#
dot $DOT -Gsize="10.0,8.0" -Teps -o build/src-inherit.eps