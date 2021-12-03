#!/usr/bin/python
# -*- coding: utf8 -*-
from __future__ import print_function
import sys

def all_are_same (l):
  return len(set(l)) <= 1

def read_list_file (filename):
  this_file = open(filename, "r")
  this_list = this_file.readlines()
  this_file.close()
  this_list.sort()
  return this_list

def count_left_list_in_right_list (left_list, right_list):
  left_word=""
  left_n=0 # number of unique first words in left_list
  common_n=0 # number of unique first words in left_list also in right_list
  for left_line in left_list:
    if (left_word!=left_line.split()[0]):# Avoid counting duplicates.
      left_word = left_line.split()[0]
      left_n+=1
      left_is_in_right=False
      i=0
      for right_line in right_list:
        i+=1
        right_word = right_line.split()[0]
        if(left_word==right_word):
          left_is_in_right=True
          common_n+=1
          break# Stop looking when matched.
      if(left_is_in_right==False): print(left_word, file=sys.stderr)
  return common_n, left_n

def compare_lists( left_filename, right_filename ):
  left_list = read_list_file(left_filename)
  right_list = read_list_file(right_filename)
  print("in "+left_filename+" but missing from "+right_filename+":",
    file=sys.stderr)
  left_in_right_n,left_n =count_left_list_in_right_list (left_list, right_list)
  print("in "+right_filename+" but missing from "+left_filename+":",
    file=sys.stderr)
  right_in_left_n,right_n =count_left_list_in_right_list (right_list, left_list)
  print(str(left_n)+","+str(right_n))
  print(str(left_in_right_n)+","+str(right_in_left_n))
  # Return true if all values are the same.
  return all_are_same({left_n,right_n,left_in_right_n,right_in_left_n})

if __name__ == "__main__":
  ''' Compare lists in two text files '''
  if (len(sys.argv)-1 == 2):
    if(compare_lists(sys.argv[1], sys.argv[2])): sys.exit(0)
    else: sys.exit(1)
  else:
    print("compare-lists.py needs two arguments.")
    sys.exit(1)
