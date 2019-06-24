#!/bin/bash
grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}'
