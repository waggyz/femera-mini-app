#!/bin/bash
# NOTE ***** converts microns to meters *****
INP=cube/unit1p2n2.inp
INP=$1
#MSH=cube/unit1p2n2.msh

#NELEM=6
#NNODE=27

echo "\$MeshFormat"
echo "2.2 0 8"

echo "\$EndMeshFormat"
echo "\$Nodes"
#echo $NNODE
sed -e '1,/\*[nN][oO][dD][eE]/d;/\*/,$d' $INP | wc -l
sed -e '1,/\*[nN][oO][dD][eE]/d;/\*/,$d' $INP | awk -F,\
  'BEGIN { OFS = " " }{print $1+0,$2*1e-6,$3*1e-6,$4*1e-6}'

echo "\$EndNodes"
echo "\$Elements"
#echo $NELEM
sed -e '1,/\*[eE][lL][eE][mM][eE][nN][tT]/d;/\*/,$d' $INP | wc -l
sed -e '1,/\*[eE][lL][eE][mM][eE][nN][tT]/d;/\*/,$d' $INP |\
  awk -F, 'BEGIN { OFS = " " }\
  {print $1+0,11,2,1,1 ,$2+0,$3+0,$4+0,$5+0,$6+0,$7+0,$8+0,$9+0,$11+0,$10+0}'

echo "\$EndElements"