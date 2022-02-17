#!/usr/bin/python
# -*- coding: utf8 -*-

import csv
import matplotlib.pyplot as plt
# import matplotlib.dates as pdt
# import datetime as dt

def some_function( *args, **opts ):
  
  return

def time2sec(time_str):
    """Get Seconds from time."""
    h, m, s = time_str.split(':')
    return int(h) * 3600 + int(m) * 60 + int(s)



if __name__ == "__main__":
  ''' Plot code stats '''
  ix = []
  x = []
  loc = [] # lines of code
  foc = [] # number of source code files
  lot = [] # lines of test code
  fot = [] # number of test code files
  build_time = []
  td_n = [] # number of TO_DO
  fm_n = [] # number of FIX_ME
  lines_ylim = 20000
  files_ylim = 200
  with open ('data/src/femera-i7-7820HQ-build-stats.csv') as File:
    data = csv.reader(File, delimiter=',')
    i=177
    for row in data:
      i+=1
      ix.append(i)
      # x.append(dt.datetime.strptime(row[0],'%Y-%d-%mT').date())
      loc.append(int(row[2]) * files_ylim/lines_ylim)
      foc.append(int(row[3]))
      lot.append(int(row[4]) * files_ylim/lines_ylim)
      fot.append(int(row[5]))
      td_n.append(int(row[6]))
      fm_n.append(int(row[7]))
      build_time.append(time2sec(row[8]))
  # w = 0.5
  # plt.gca().xaxis.set_major_formatter(pdt.DateFormatter('%Y-%M-%D'))
  # plt.gca().xaxis.set_major_locator(pdt.DayLocator())
  plt.plot(ix,foc,label='Source code files')
  plt.plot(ix,fot,label='Hundreds of lines source code')
  plt.plot(ix,loc,label='Test code files')
  plt.plot(ix,lot,label='Hundreds of lines test code')
  plt.plot(ix,td_n,label='Number of TO'+'DO in source code')
  plt.plot(ix,fm_n,label='Number of FIX'+'ME in source code')
  plt.plot(ix,build_time,label='Build time (sec)')
  plt.title('Femera 0.3 source code statistics')
  plt.xlabel('Commit number')
  plt.legend()
  # plt.gcf().autofmt_xdate()
  plt.show()
