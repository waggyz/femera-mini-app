#!/usr/bin/python
# -*- coding: utf8 -*-
import csv
import matplotlib.pyplot as plt
import datetime as dt
import numpy as np

def some_function( *args, **opts ):
  #TODO filename, cpumodel arguments
  return


if __name__ == "__main__":
  """ Plot zyc.?pp Performance Test results """
  now = dt.datetime.now()
  cpumodel="i7-7820HQ"
  timestr = now.strftime("%Y-%m-%d")
  file_name = "data/perf/zyc.perf-"+timestr+".out"
  plot_name = "build/"+cpumodel+"/zyc-perf-"+timestr
  name1 = []
  name2 = []
  order = []
  md_n  = []# number of multidual operations
  byte_n= []
  flop_n= []
  secs  = []
  #
  list_order = [0,1,2,3,4,5,6,7,8]
  speed_mult_ref = 0
  speed_sum2_ref = 0
  speed_mult = [0.0] * len(list_order);
  speed_sum2 = [0.0] * len(list_order);
  crmat_mult = [0.0] * len(list_order);
  crmat_sum2 = [0.0] * len(list_order);
  trnsp_mult = [0.0] * len(list_order);
  trnsp_sum2 = [0.0] * len(list_order);
  #
  with open (file_name) as File:
    data = csv.reader(File, delimiter=",")
    for rowcol in data:
      try:
        name0 = rowcol[0]
      except:
        name0 = "--skip--"
      if name0 == " zyc":
        name1.append(rowcol[1])
        name2.append(rowcol[2])
        order.append(int(rowcol[3]))
        o = int(rowcol[3])
        s = float(rowcol[4]) / float(rowcol[7])
        if o < 0:
          if rowcol[2] == " fma":
            speed_mult_ref += s / len(list_order)
          elif rowcol[2] == "sum2":
            speed_sum2_ref += s / len(list_order)
        else:
          if rowcol[2] == " fma":
            speed_mult[o] += s
          elif rowcol[2] == "sum2":
            speed_sum2[o] += s
          elif rowcol[2] == "tfma":
            trnsp_mult[o] += s
          elif rowcol[2] == " ts2":
            trnsp_sum2[o] += s
          elif rowcol[2] == "tnai":
            crmat_mult[o] += s
          elif rowcol[2] == "tns2":
            crmat_sum2[o] += s
        md_n.append(int(rowcol[4]))
        byte_n.append(float(rowcol[5]))
        flop_n.append(float(rowcol[6]))
        secs.append(float(rowcol[7]))
  #mdops = [n/s for n,s in zip(md_n,secs)]
  mult_nonz_lim = [speed_mult_ref/(3**o) for o in list_order]
  sum2_nonz_lim = [speed_sum2_ref/(3**o) for o in list_order]
  mult_mdsz_lim = [speed_mult_ref/(2**o) for o in list_order]
  sum2_mdsz_lim = [speed_sum2_ref/(2**o) for o in list_order]
  mult_crmat_lim = [speed_mult_ref/((2**o)**2) for o in list_order]
  sum2_crmat_lim = [speed_sum2_ref/((2**o)**2) for o in list_order]
  #
  mult_x = [mfree/crmat -1.0 for mfree,crmat in zip (speed_mult,crmat_mult)]
  sum2_x = [mfree/crmat -1.0 for mfree,crmat in zip (speed_sum2,crmat_sum2)]
  print mult_x
  print sum2_x
  #
  lw  = 2.0    # linewidth
  ms  = 6      # markersize
  mw  = 1      # markeredgewidth
  rlw = 1.0/3.0# reference linewidth
  rms = 4      # reference markersize
  #
  if True:
    plt.semilogy(list_order, sum2_mdsz_lim,
      label="Sum of squares (multidual size limited)",
      marker="s", markersize=rms, markeredgecolor='g', markerfacecolor='none',
      color='g', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, sum2_nonz_lim,
    label="Sum of squares (CR nonzero count limited)",
    marker="s", markersize=rms, markeredgecolor='y', markerfacecolor='none',
    color='y', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, sum2_crmat_lim,
    label="Sum of squares (CR matrix size limited)",
    marker="s", markersize=rms, markeredgecolor='r', markerfacecolor='none',
    color='r', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, speed_sum2,
    label="Sum of squares (matrix-free algorithm)",
    marker="s", markersize=ms, markeredgecolor="b", markerfacecolor='none',
    markeredgewidth=mw, color="b",linestyle="dashed")
  if False:
    plt.semilogy(list_order, trnsp_sum2, label="Sum of squares (transposed)",
      marker="s", markersize=ms, markeredgecolor="m", markerfacecolor='none',
      markeredgewidth=mw, color="m",linestyle="dashed")
  plt.semilogy(list_order, crmat_sum2,
    label="Sum of squares (naive algorithm)",
    marker="s", markersize=ms, markeredgecolor="k", markerfacecolor='none',
    markeredgewidth=mw, color="k",linestyle="dashed")
  #
  if True:
    plt.semilogy(list_order, mult_mdsz_lim,
      label="Multiply (multidual size limited)",
      marker="x", markeredgecolor='g', markerfacecolor='none',
      color='g', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, mult_nonz_lim,
    label="Multiply (CR nonzero count limited)",
    marker="x", markersize=rms, markeredgecolor='y', markerfacecolor='none',
    color='y', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, mult_crmat_lim,
    label="Multiply (CR matrix size limited)",
    marker="x", markersize=rms, markeredgecolor='r', markerfacecolor='none',
    color='r', linestyle="solid", linewidth=rlw)
  plt.semilogy(list_order, speed_mult, label="Multiply (matrix-free algorithm)",
    markersize=ms, marker="x", color="b", markeredgewidth=mw,
    linestyle="dotted")
  if False:
    plt.semilogy(list_order, trnsp_mult, label="Multiply (transposed)",
      marker="x", markersize=ms, markeredgecolor="m",
      markeredgewidth=mw, color="m",linestyle="dotted",)
  plt.semilogy(list_order, crmat_mult, label="Multiply (naive algorithm)",
    marker="x", markersize=ms, color="k", markeredgecolor="k",
    markeredgewidth=mw, linestyle="dotted")
  #
  #plt.gca().set(xlim=(-0.1, 8.1))
  plt.gca().set(ylim=(1e3, 1e10))
  plt.xlabel("Multidual order")
  plt.ylabel("Speed (multidual operations per second)")
  ''' plt.gca().annotate ("faster is better ->",
            xy=(0, 0.5), xytext=(10, 0),#, rotatation=90)#,
            xycoords=('axes fraction', 'figure fraction'),
            textcoords='offset points',
            size=14, ha='center', va='bottom')'''
  plt.text(x=8.1, y=2.7*10**7, s=r"Higher is better.$\rightarrow$",
    rotation=90.0)
  #
  plt.grid()
  plt.title("Multidual multiplication performance ("+cpumodel+")")
  plt.legend(loc='lower left', fontsize=10, numpoints=1)
  #
  plt.savefig(plot_name+".eps",format="eps")
  plt.savefig(plot_name+".png",format="png")
#
