#!/bin/bash
INP=cube/unit1p2n2.inp
INP=$1
#MSH=cube/unit1p2n2.msh

sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'min=="" || $2 < min {min=$2} END{print min}'
sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'max=="" || $2 > max {max=$2} END{print max}'

sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'min=="" || $3 < min {min=$3} END{print min}'
sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'max=="" || $3 > max {max=$3} END{print max}'

sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'min=="" || $4 < min {min=$4} END{print min}'
sed '1,/\$Nodes/d;/\$EndNodes/,$d' $INP |\
  awk 'max=="" || $4 > max {max=$4} END{print max}'