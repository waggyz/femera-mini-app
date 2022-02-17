#!/usr/bin/python
# -*- coding: utf8 -*-

import csv
import matplotlib.pyplot as plt

def some_function( *args, **opts ):
  
  return

def time2sec(time_str):
    """Get Seconds from time."""
    h, m, s = time_str.split(':')
    return int(h) * 3600 + int(m) * 60 + int(s)



if __name__ == "__main__":
  ''' Plot code stats '''
  x = []
  loc = [] # lines of code
  foc = [] # number of source code files
  lot = [] # lines of test code
  fot = [] # number of test code files
  build_time = []
  td_n = [] # number of TO_DO
  fm_n = [] # number of FIX_ME
  with open ('data/src/femera-i7-7820HQ-build-stats.csv') as File:
    data = csv.reader(File, delimiter=',')
    i=0
    for row in data:
      i+=1
      ix.append(i)
      x.append(row[0])
      loc.append(int(row[2]))
      foc.append(int(row[3]))
      lot.append(int(row[4]))
      fot.append(int(row[5]))
      td_n.append(int(row[6]))
      fm_n.append(int(row[7]))
      build_time.append(row[8])
  lines_ylim = 20000
  files_ylim = 200
  w = 0.1
  plt.bar(ix,foc,w)
  plt.bar(ix,fot,w)
  plt.bar(ix,loc * files_ylim/lines_ylim,w)
  plt.bar(ix,lot * files_ylim/lines_ylim,w)
  plt.plot(ix,td_n)
  plt.plot(ix,fm_n)
  plt.plot(ix,time2sec(build_time))
  plt.legend()
  plt.show()
