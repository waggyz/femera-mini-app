#!/bin/bash
#
# inp2txtmsh.sh ABAQUS_INP_FILE TXTMSH_DIRECTORY
INP=$1
DIR=$2"/"
#
if [ ! -d $DIR ]; then
  mkdir $DIR
fi
# SurfaceSets not yet supported. Just make an empty file.
touch $DIR"SurfaceSets"
#
# Make the Nodes file
sed -e '1,/\*[nN][oO][dD][eE]/d;/\*/,$d' $INP | awk -F,\
  'BEGIN { OFS = " " }{print $1+0,$2,$3,$4}' > $DIR"Nodes"
#
# Make an ElementSet containing all elements
NELEM=`sed -e '1,/\*[eE][lL][eE][mM][eE][nN][tT]/d;/\*/,$d' $INP | wc -l`
echo "elem_all 1" > $DIR"ElementSets"
echo "1 "$NELEM >> $DIR"ElementSets"

# Make the Elements file (tet10)
echo "TET 10 "$NELEM > $DIR"Elements"
sed -e '1,/\*[eE][lL][eE][mM][eE][nN][tT]/d;/\*/,$d' $INP |\
  awk -F, 'BEGIN { OFS = " " }\
  {print $1+0,$2+0,$3+0,$4+0,$5+0,$6+0,$7+0,$8+0,$9+0,$11+0,$10+0}'\
  >> $DIR"/Elements"
#
# Standard fixed planes and displaced at x==1
#
sed -e '1,/\*[bB][oO][uU][nN][dD][aA][rR][yY]/d;/\*/,$d' $INP > $DIR"bound.tmp"
#
grep ",1,1$" $DIR"bound.tmp" > $DIR"boun1.tmp"
grep ",2,2$" $DIR"bound.tmp" > $DIR"boun2.tmp"
grep ",3,3$" $DIR"bound.tmp" > $DIR"boun3.tmp"
grep ",1,1,0.001$" $DIR"bound.tmp" > $DIR"boun0.tmp"

NNODE=`wc -l < $DIR"boun1.tmp"`
echo node_fix_x $NNODE > $DIR"NodeSets"
awk -F, '{print $1}' $DIR"boun1.tmp" >> $DIR"NodeSets"

NNODE=`wc -l < $DIR"boun2.tmp"`
echo node_fix_y $NNODE >> $DIR"NodeSets"
awk -F, '{print $1}' $DIR"boun2.tmp" >> $DIR"NodeSets"

NNODE=`wc -l < $DIR"boun3.tmp"`
echo node_fix_z $NNODE >> $DIR"NodeSets"
awk -F, '{print $1}' $DIR"boun3.tmp" >> $DIR"NodeSets"

NNODE=`wc -l < $DIR"boun0.tmp"`
echo node_dis_x $NNODE >> $DIR"NodeSets"
awk -F, '{print $1}' $DIR"boun0.tmp" >> $DIR"NodeSets"

rm $DIR"bound.tmp" $DIR"boun1.tmp" $DIR"boun2.tmp" $DIR"boun3.tmp" $DIR"boun0.tmp"
