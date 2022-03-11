#!/usr/bin/python
# -*- coding: utf8 -*-

import csv
import matplotlib.pyplot as plt
import matplotlib.dates as pdt
import datetime as dt
import numpy as np

def some_function( *args, **opts ):
  #TODO filename, cpumodel arguments
  return

def time2sec(time_str):
  """Get Seconds from time."""
  h, m, s = time_str.split(':')
  return int(h) * 3600 + int(m) * 60 + int(s)

def parse_commit_number(version):
  """Parse commit number from version string (e.g. 0.3.220+g317d8f9.)"""
  ends_with_commit = version.split('+')[0]
  return int(ends_with_commit.split('.')[-1])

if __name__ == "__main__":
  ''' Plot code stats '''
  cpumodel='i7-7820HQ'
  t = []
  #dur = [] # seconds since commit #177
  loc = [] # lines of code
  foc = [] # number of source code files
  lot = [] # lines of test code
  fot = [] # number of test code files
  build_time = []
  ntd = [] # number of TO_DO
  nfm = [] # number of FIX_ME
  commit = []
  files_ylim = 150.0
  lines_ylim = files_ylim * 100.0
  with open ('data/src/femera-'+cpumodel+'-build-stats.csv') as File:
    data = csv.reader(File, delimiter=',')
    #i=177
    for rowcol in data:
      if rowcol[11] == cpumodel:
        dto = dt.datetime.strptime(rowcol[0],'%Y-%m-%dT%H:%M')
        t.append(dto.date())
        #if i == 177:
        #  start = dto
        #dur.append((dto - start).total_seconds())
        commit.append(parse_commit_number(rowcol[1]))
        loc.append(float(rowcol[2]) * files_ylim/lines_ylim)
        foc.append(int(rowcol[3]))
        lot.append(float(rowcol[4]) * files_ylim/lines_ylim)
        fot.append(int(rowcol[5]))
        nfm.append(int(rowcol[6]))
        ntd.append(int(rowcol[7]))
        build_time.append(time2sec(rowcol[8]))
        #i+=1
  #x = commit
  #plt.xlabel('Repository commit number')
  #
  x = t
  #plt.gca().xaxis.set_major_formatter(pdt.DateFormatter('%m/%d/%y'))
  plt.gca().xaxis.set_major_formatter(pdt.DateFormatter('%Y-%m-%d'))
  plt.gca().xaxis.set_major_locator(pdt.MonthLocator())
  #
  plt.gca().set_yticks (np.arange(0, files_ylim*1.001, 25))
  plt.plot(x,loc,label='Hundreds of lines source code', linestyle='dashed')
  plt.plot(x,lot,label='Hundreds of lines test code', linestyle='dashed')
  plt.plot(x,foc,label='Number of source code files')
  plt.plot(x,fot,label='Number of test code files')
  plt.plot(x,ntd,label='Number of TO'+'DO in source code')
  plt.plot(x,nfm,label='Number of FIX'+'ME in source code')
  plt.plot(x,build_time,label='Build time for '+cpumodel+' (sec)')
  #plt.plot(x,commit,label='Repository commit number', linestyle='dotted')
  plt.grid()
  plt.title('Femera 0.3 mini-app source code statistics')
  dt0=dt.datetime.strptime('2022-02-01','%Y-%m-%d')
  plt.gca().set(xlim=(dt0.date(), max(t)))
  plt.gca().set(ylim=(0, files_ylim))
  plt.legend(loc='upper left', fontsize=10)
  # plt.gcf().autofmt_xdate()
  #plt.show()
  plt.savefig('build/'+cpumodel+'/code-stats.png',format='png')
  plt.savefig('build/'+cpumodel+'/code-stats.eps',format='eps')
