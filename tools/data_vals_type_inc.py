#!/usr/bin/python3
# -*- coding: utf8 -*-

import csv

if __name__ == "__main__":
  ''' make fmr::Vals_type include files '''
  vals_enum = []
  vals_name = []
  vals_type = []
  vals_info = []
  with open ('data/src/data-type.csv') as File:
    data = csv.reader(File, delimiter=',')
    for rowcol in data:
      vals_enum.append(rowcol[0])
      vals_name.append("\""+rowcol[0]+"\"")
      vals_type.append(rowcol[1])
      vals_info.append(rowcol[2])
  with open ('src/fmr/data-type-enum.inc','w') as File:
    for item in vals_enum:
      print('{0},'.format(item.strip()), file=File)
  with open ('src/fmr/data-type-name.inc','w') as File:
    for item in vals_name:
      print('{0},'.format(item.strip().lower()), file=File)
  with open ('src/fmr/data-vals-type.inc','w') as File:
    for item in vals_type:
      print('{0},'.format(item.strip()), file=File)
  with open ('src/fmr/data-type-info.inc','w') as File:
    for item in vals_info:
      print('{0},'.format(item.strip()), file=File)
#
