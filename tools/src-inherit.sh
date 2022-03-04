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
# e.g.   class Jobs : public Sims<Jobs> { #NOTE does not work
#sed -n 's/^[ ]*class \([A-Z][a-z0-9_]*\)\([ :a-z0-9_]*\)\([A-Z][A-Za-z0-9_<>]*\).*/\"\1\" -> \"\3\"/p' \
sed -n 's/^[ ]*class \([A-Z][a-z0-9_]*\)\([ :a-z0-9_]*\)\([A-Z][a-z0-9_]*\).*/\"\1\" -> \"\3\"/p' \
  src/*/*.hpp src/*/*/*.hpp >>$DOT
#
#echo '{ rank = sink; "perf"; }' >>$DOT
#
#echo '{ rank = sink; "math"; }' >>$DOT
#echo '{ rank = sink; "type"; }' >>$DOT
#echo '{ rank = sink; "data-type"; }' >>$DOT
#echo '{ rank = sink; "vals"; }' >>$DOT
#
#echo '{ rank = sink; "Task"; }' >>$DOT
# echo '{ rank = sink; "Flog"; }' >>$DOT
#
echo \} >>$DOT
#
dot $DOT -Gsize="6.0,3.0" -Teps -o build/src-inherit.eps
