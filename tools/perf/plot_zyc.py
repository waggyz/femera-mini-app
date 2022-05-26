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
  ncpu=4
  timestr = now.strftime("%Y-%m-%d")
  #timestr = "2022-05-18"
  timestr = "2022-05-26"
  file_name = "data/perf/zyc.perf."+timestr+".out"
  naive_file = "data/perf/zyc.perf.2022-05-16b.out"
  plot_name = "build/"+cpumodel+"/zyc-perf-"+timestr
  name1 = []
  name2 = []
  order = []
  md_n  = []# number of multidual operations
  byte_n= []
  flop_n= []
  secs  = []
  #
  list_order = [0,1,2,3,4,5,6,7,8,9,10]
  naiv_order = [0,1,2,3,4,5,6,7,8]
  max_order = max(list_order)
  '''
  speed_mult_ref = 0
  speed_mult = [0.0] * len(list_order);
  crmat_mult = [0.0] * len(list_order);
  trnsp_mult = [0.0] * len(list_order);
  permu_trnsp_mult = [0.0] * len(list_order);
  #
  speed_sum2_ref = 0
  speed_sum2 = [0.0] * len(list_order);
  crmat_sum2 = [0.0] * len(list_order);
  trnsp_sum2 = [0.0] * len(list_order);
  permu_trnsp_sum2 = [0.0] * len(list_order);
  #
  speed_mult_aos = [0.0] * len(list_order);
  speed_mult_soa = [0.0] * len(list_order);
  '''
  time_crmat_mult = [0.0] * len(naiv_order);
  mdop_crmat_mult = [0.0] * len(naiv_order);
  time_mult_ref = 0
  mdop_mult_ref = 0
  time_mult_aos = [0.0] * len(list_order)
  mdop_mult_aos = [0.0] * len(list_order)
  time_mult_soa = [0.0] * len(list_order)
  mdop_mult_soa = [0.0] * len(list_order)
  #
  time_div_aos = [0.0] * len(list_order)
  mdop_div_aos = [0.0] * len(list_order)
  time_div_soa = [0.0] * len(list_order)
  mdop_div_soa = [0.0] * len(list_order)
  time_add = [0.0] * len(list_order)
  mdop_add = [0.0] * len(list_order)
  #
  with open (naive_file) as File:
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
        m = float(rowcol[4])
        t = float(rowcol[7])
        if rowcol[2] == "tnai":
          if o < 9:
            time_crmat_mult[o] += t
            mdop_crmat_mult[o] += m
            #crmat_mult[o] += s
        md_n.append(int(rowcol[4]))
        byte_n.append(float(rowcol[5]))
        flop_n.append(float(rowcol[6]))
        secs.append(float(rowcol[7]))
  #
  with open (file_name) as File:
    data = csv.reader(File, delimiter=",")
    for rowcol in data:
      try:
        name0 = rowcol[0]
      except:
        name0 = "--skip--"
      if name0 == " zyc":
        if rowcol[2] == "mult":
          name1.append(rowcol[1])
          name2.append(rowcol[2])
          order.append(int(rowcol[3]))
        o = int(rowcol[3])
        s = float(rowcol[4]) / float(rowcol[7])
        m = float(rowcol[4])
        t = float(rowcol[7])
        if o < 0:
          if rowcol[2] == " fma":
            time_mult_ref += t / len(list_order)
            mdop_mult_ref += m / len(list_order)
          '''
          if rowcol[2] == " fma":
            speed_mult_ref += s / len(list_order)
          elif rowcol[2] == "sum2":
            speed_sum2_ref += s / len(list_order)
          '''
        else:
          if rowcol[2] == " aos":
            time_mult_aos[o] += t
            mdop_mult_aos[o] += m
            #speed_mult_aos[o] += s
          elif rowcol[2] == " soa":
            time_mult_soa[o] += t
            mdop_mult_soa[o] += m
          #
          elif rowcol[1]+rowcol[2] == "mdasmadd":
            time_mult_aos[o] += t
            mdop_mult_aos[o] += m
            #speed_mult_aos[o] += s
          elif rowcol[1]+rowcol[2] == "mdsamadd":
            time_mult_soa[o] += t
            mdop_mult_soa[o] += m
          elif rowcol[1]+rowcol[2] == "mdas div":
            time_div_aos[o] += t
            mdop_div_aos[o] += m
            #speed_mult_aos[o] += s
          elif rowcol[1]+rowcol[2] == "mdsa div":
            time_div_soa[o] += t
            mdop_div_soa[o] += m
          elif rowcol[1]+rowcol[2] == "  mz add":
            time_add[o] += t
            mdop_add[o] += m
        if rowcol[2] == "mult":
          md_n.append(int(rowcol[4]))
          byte_n.append(float(rowcol[5]))
          flop_n.append(float(rowcol[6]))
          secs.append(float(rowcol[7]))
  #mdops = [n/s for n,s in zip(md_n,secs)]
  speed_mult_ref = 4* mdop_mult_ref / time_mult_ref
  crmat_mult = [ncpu* zm/zs for zm,zs in zip (mdop_crmat_mult,time_crmat_mult)]
  speed_mult_aos = [ncpu* zm/zs for zm,zs in zip (mdop_mult_aos,time_mult_aos)]
  speed_mult_soa = [ncpu* zm/zs for zm,zs in zip (mdop_mult_soa,time_mult_soa)]
  speed_div_aos = [ncpu* zm/zs for zm,zs in zip (mdop_div_aos,time_div_aos)]
  speed_div_soa = [ncpu* zm/zs for zm,zs in zip (mdop_div_soa,time_div_soa)]
  speed_add = [ncpu* zm/zs for zm,zs in zip (mdop_add,time_add)]
  mult_nonz_lim = [speed_mult_ref/(3**o) for o in list_order]
  #sum2_nonz_lim = [speed_sum2_ref/(3**o) for o in list_order]
  mult_mdsz_lim = [speed_mult_ref/(2**o) for o in list_order]
  #sum2_mdsz_lim = [speed_sum2_ref/(2**o) for o in list_order]
  mult_crmat_lim = [speed_mult_ref/((2**o)**2) for o in list_order]
  #sum2_crmat_lim = [speed_sum2_ref/((2**o)**2) for o in list_order]
  #
  if False:
    mult_soa_x = [mfree/crmat for mfree,crmat in zip (speed_mult_soa,crmat_mult)]
    mult_aos_x = [mfree/crmat for mfree,crmat in zip (speed_mult_aos,crmat_mult)]
    print soa_x
    print aos_x
  #
  lw  = 2.0    # linewidth
  ms  = 6      # markersize
  mw  = 1      # markeredgewidth
  rlw = 1.0/1.0# reference linewidth
  rms = 6      # reference markersize
  #
  if False:#-------------------------------------------------------------------
    plt.semilogy(list_order, sum2_mdsz_lim,
      label="Sum of squares O($\mathregular{2^m}$) multidual size limited",
      marker="s", markersize=rms, markeredgecolor='g', markerfacecolor='g',
      color='g', linestyle="dotted", linewidth=rlw)
    plt.semilogy(list_order, sum2_nonz_lim,
      label="Sum of squares O($\mathregular{3^m}$) CR nonzero count limited",
      marker="s", markersize=rms, markeredgecolor='y', markerfacecolor='y',
      color='y', linestyle="dotted", linewidth=rlw)
    plt.semilogy(list_order, sum2_crmat_lim,
      label="Sum of squares O($\mathregular{4^m}$) CR matrix size limited",
      marker="s", markersize=rms, markeredgecolor='r', markerfacecolor='r',
      color='r', linestyle="dotted", linewidth=rlw)
    if True:
      plt.semilogy(list_order, permu_trnsp_sum2,
        label="Sum of squares (transposed)", color="m",linestyle="dashed",
        marker="s", markersize=ms, markeredgecolor="m", markerfacecolor='none',
        markeredgewidth=mw)
    if True:
      plt.semilogy(list_order, speed_sum2,
        label="Sum of squares (matrix-free algorithm)",
        marker="s", markersize=ms, markeredgecolor="c", markerfacecolor='none',
        markeredgewidth=mw, color="c",linestyle="dashed")
    if False:
      plt.semilogy(list_order, trnsp_sum2, label="Sum of squares (transposed)",
        marker="s", markersize=ms, markeredgecolor="g", markerfacecolor='none',
        markeredgewidth=mw, color="g",linestyle="dashed")
    plt.semilogy(list_order, crmat_sum2,
      label="Sum of squares (naive algorithm)",
      marker="s", markersize=ms, markeredgecolor="k", markerfacecolor='none',
      markeredgewidth=mw, color="k",linestyle="dashed")
  if False:#-------------------------------------------------------------------
    if True:
      plt.semilogy(list_order, speed_mult,
        label="Multiply (matrix-free algorithm)", color="c", linestyle="solid",
        markersize=ms, marker="x", markeredgecolor='c', markeredgewidth=mw)
    if True:
      plt.semilogy(list_order, permu_trnsp_mult,
        label="Multiply (matrix-free transposed)", color="g", linestyle="solid",
        markersize=ms, marker="x", markeredgecolor='g', markeredgewidth=mw)
    if False:
      plt.semilogy(list_order, trnsp_mult, label="Multiply (transposed)",
        marker="x", markersize=rms, markeredgecolor="m",
        markeredgewidth=mw, color="m",linestyle="solid",)
  if True:#--------------------------------------------------------------------
    plt.semilogy(list_order, speed_add,
      label="hypercomplex add", color="c",
      markersize=ms, marker="+", markeredgecolor='c', markeredgewidth=mw,
      linestyle="solid")
    plt.semilogy(list_order, speed_mult_aos,
      label="Matrix-free multiply, interleaved parts (AoS)",
      color="b",
      markersize=ms, marker="x", markeredgecolor='b', markeredgewidth=mw,
      linestyle="solid")
  if True:
    plt.semilogy(list_order, speed_mult_soa,
      label="Matrix-free multiply, contiguous parts (SoA)",
      color="m",
      markersize=ms, marker="x", markeredgecolor='m', markeredgewidth=mw,
      linestyle="solid")
    plt.semilogy(naiv_order, crmat_mult,
      label="Naive multiply algorithm", linewidth=rlw/2,
      marker="x", markersize=rms-1.5, color="k", markeredgecolor="k",
      linestyle="solid") #markeredgewidth=mw,
    plt.semilogy(list_order, speed_div_aos,
      label="Matrix-free divide, interleaved parts (AoS)", 
      color="b", markersize=ms, marker="s", markerfacecolor="None",
      markeredgecolor='b', markeredgewidth=mw,
      linestyle="dashed")
  if True:
    plt.semilogy(list_order, speed_div_soa,
      label="Matrix-free divide, contiguous parts (SoA)",
      color="m", markersize=ms, marker="s",
      markeredgecolor='m', markeredgewidth=mw, markerfacecolor="None",
      linestyle="dashed")
  if True:#--------------------------------------------------------------------
    plt.semilogy(list_order, mult_mdsz_lim,
      label="Complexity O($\mathregular{2^m}$): limited by multidual size",
      marker="*", markersize=rms, markeredgecolor='g', markerfacecolor='g',
      color='g', linestyle="dotted", linewidth=rlw)
    plt.semilogy(list_order, mult_nonz_lim,
      label="Complexity O($\mathregular{3^m}$): limited by CR nonzero count",
      marker="*", markersize=rms, markeredgecolor='y', markerfacecolor='y',
      color='y', linestyle="dotted", linewidth=rlw)
    plt.semilogy(list_order, mult_crmat_lim,
      label="Complexity O($\mathregular{4^m}$): limited by CR matrix size",
      marker="*", markersize=rms, markeredgecolor='r', markerfacecolor='r',
      color='r', linestyle="dotted", linewidth=rlw)
  #
  #plt.gca().set(xlim=(-0.1, 8.1))
  plt.gca().set(ylim=(1e3, 1e10))
  plt.xlabel("Multidual order (m)")
  plt.ylabel("Performance (multidual operations per second)")
  plt.text(x=max_order+0.1, y=2.7*10**7, s=r"Higher is better.$\rightarrow$",
    rotation=90.0)
  #
  plt.grid(axis = 'y')
  plt.title(r"Multidual operations ($\mathregular{c_j = a_j{\circ}b_j}$) "
    + cpumodel+" baseline")
  plt.legend(loc='lower left', fontsize=9, numpoints=1)
  #
  plt.savefig(plot_name+".eps",format="eps")
  plt.savefig(plot_name+".png",format="png")
#
