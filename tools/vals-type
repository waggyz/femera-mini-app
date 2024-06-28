#!/bin/bash
#
awk -F, '{print $1","}' "data/vals-type.csv" > "src/data-enum.inc"
awk -F, '{print $2","}' "data/vals-type.csv" > "src/vals-type.inc"
awk -F, '{print "\""$1"\","}' "data/vals-type.csv" > "src/vals-name.inc"
awk -F, '{print $3","}' "data/vals-type.csv" > "src/vals-info.inc"
#
