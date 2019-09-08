#!/bin/bash
CPUMODEL=`./cpumodel.sh`
CPUCOUNT=`./cpucount.sh`
CSTR=gcc
EXEDIR="."
GMSH2FMR=gmsh2fmr-$CPUMODEL-$CSTR
#
P=2; C=$CPUCOUNT; N=$C; RTOL=1e-24;
TARGET_TEST_S=6;# Try for S sec/run
REPEAT_TEST_N=5;# Repeat each test N times
ITERS_MIN=10;
#
PERFDIR="perf"
PROFILE=$PERFDIR/"uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".pro"
CSVFILE=$PERFDIR/"uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".csv"
#
if [ -d "/hpnobackup1/dwagner5/femera-test/cube" ]; then
  MESHDIR=/hpnobackup1/dwagner5/femera-test/cube
else
  MESHDIR=cube
fi
echo "Mesh Directory: "$MESHDIR"/"
#
HAS_GNUPLOT=`which gnuplot`
MEM=`free -b  | grep Mem | awk '{print $7}'`
echo `free -g  | grep Mem | awk '{print $7}'` GB Available Memory
#
if [ -f $PROFILE ]; then
  NODE_MAX=`grep -m1 -i nodes $PROFILE | awk '{print $1}'`
  UDOF_MAX=$(( $NODE_MAX * 3 ))
  TET10_MAX=`grep -m1 -i elements $PROFILE | awk '{print $1}'`
else
  UDOF_MAX=$(( $MEM / 120 ))
  NODE_MAX=$(( $UDOF_MAX / 3))
  TET10_MAX=$(( $UDOF_MAX / 4 ))
fi
MDOF_MAX=$(( $UDOF_MAX / 1000000 ))
echo Max. Model: $TET10_MAX Tet10, $NODE_MAX Nodes, $MDOF_MAX MDOF
#
MESH_N=21
LIST_H=(2 3 4 5 7 9   12 15 21 26 33   38 45 56 72 96   123 156 205 265 336)
NOMI_UDOF=(500 1000 2500 5000 10000 25000\
 50000 100000 250000 500000 1000000\
 1500000 2500000 5000000 10000000 25000000\
 50000000 100000000 250000000 500000000 1000000000)
TRY_COUNT=0;
for I in $(seq 0 20); do
  if [ ${NOMI_UDOF[I]} -lt $UDOF_MAX ]; then
    TRY_COUNT=$(( $TRY_COUNT + 1))
  fi
done
#
if [ ! -f $PROFILE ]; then
  # First, get a rough idea of DOF/sec to estimate time
  # with 10 iterations of the second-to-largest model
  if [ ! -f $CSVFILE ]; then
    ITERS=10; H=${LIST_H[$(( $TRY_COUNT - 2 ))]};
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    echo Estimating performance at\
      $(( ${NOMI_UDOF[$(( $TRY_COUNT - 2 ))]} / 1000000 )) MDOF...
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR"
    echo Running $ITERS iterations of $MESHNAME...
    $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
      -p $MESH >> $CSVFILE
  fi
fi
MUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3/1000000 }'`
MDOFS=`head -n1 $CSVFILE | awk -F, '{ print $13/1000000 }'`
DOFS=`head -n1 $CSVFILE | awk -F, '{ print $13 }'`
echo "Initial performance estimate: "$MDOFS" MDOF/s at "$MUDOF" MDOF"
#
ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $MDOFS $MUDOF | bc`
CSVLINES=`wc -l < $CSVFILE`
BASIC_TEST_N=$(( $TRY_COUNT * $REPEAT_TEST_N + 1 ))
if [ "$CSVLINES" -lt "$BASIC_TEST_N" ]; then
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    H=${LIST_H[I]}
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    echo "Meshing, partitioning, and converting "$MESHNAME", if necessary..."
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR"
    NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    NDOF=$(( $NNODE * 3 ))
    ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $DOFS $NDOF | bc`
    if [ $ITERS -lt $ITERS_MIN ]; then ITERS=10; fi
    echo "Running "$ITERS" iterations of "$MESHNAME" ("$NDOF" DOF), "\
      $REPEAT_TEST_N" times..."
    for I in $(seq 1 $REPEAT_TEST_N ); do
      $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
      -p $MESH >> $CSVFILE
    done
  done
fi
if [ -f $CSVFILE ]; then
  echo "Writing basic profile: "$PROFILE"..."
  echo "Femera Performance Profile" > $PROFILE
  echo "femerq-"$CPUMODEL"-"$CSTR >> $PROFILE
  grep -m1 -i "model name" /proc/cpuinfo >> $PROFILE
  MEM_GB="`free -g  | grep Mem | awk '{print $2}'`"
  printf "      %6i\t: GB Memory\n" $MEM_GB >> $PROFILE
  #
  MDOFS=`head -n1 $CSVFILE | awk -F, '{ print $13/1000000 }'`
  NELEM=`head -n1 $CSVFILE | awk -F, '{ print $1 }'`
  NNODE=`head -n1 $CSVFILE | awk -F, '{ print $2 }'`
  MUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3/1000000 }'`
  NPART=`head -n1 $CSVFILE | awk -F, '{ print $4 }'`
  ITERS=`head -n1 $CSVFILE | awk -F, '{ print $5 }'`
  NCPUS=`head -n1 $CSVFILE | awk -F, '{ print $9 }'`
  echo >> $PROFILE
  echo "  Initial Elastic Performance Estimate" >> $PROFILE
  echo "  ------------------------------------" >> $PROFILE
  printf "        %6.1f : Performance [MDOF/s]\n" $MDOFS >> $PROFILE
  printf "        %6.1f : System Size [MDOF]\n" $MUDOF >> $PROFILE
  printf "%12i   : Nodes\n" $NNODE >> $PROFILE
  printf "%12i   : Tet10 Elements\n" $NELEM >> $PROFILE
  printf "%12i   : Partitions\n" $NPART >> $PROFILE
  printf "%12i   : Threads\n" $NCPUS >> $PROFILE
  printf "%12i   : Iterations\n" $ITERS >> $PROFILE
  #echo "Mesh            : " FIXME Put initial mesh filename here
  #
  echo >> $PROFILE
  echo "     Performance Profile Basic Test Parameters" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "%6i     : Partitions = Threads = Physical Cores\n" $CPUCOUNT >> $PROFILE
  printf "%6i     : Test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf "  %6.1f   : Each test solve time [sec]\n" $TARGET_TEST_S>>$PROFILE
  printf "%6i     : Minimum iterations\n" $ITERS_MIN >> $PROFILE
  printf "     %5.0e : Relative residual tolerance\n" $RTOL >> $PROFILE
  if [ ! -z "$HAS_GNUPLOT" ]; then
    echo "Plotting basic profile data: "$CSVFILE"..."
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set logscale x;\
    set xrange [1e3:1.05e9];\
    set yrange [0:];\
    set key inside top right;\
    set title 'Femera Elastic Performance Basic Tests [MDOF/s]';\
    set xlabel 'System Size [DOF]';\
    plot 'perf/uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".csv'\
    using 3:(\$4 != \$9 ? 1/0:\$13/1e6)\
    with points pointtype 0\
    title '"$CPUCOUNT" Partitions';"\
    | tee -a $PROFILE | grep --no-group-separator -C25 --color=always '\.'
  fi
fi
#FIXME Change to check if any CSV lines have N != C
#if ! grep -q -i "partitioning" $PROFILE; then
CSV_HAS_PART_TEST=`awk -F, '$4!=$9{print $4; exit}' $CSVFILE`
if [ -z "$CSV_HAS_PART_TEST" ]; then
  H=${LIST_H[$(( $TRY_COUNT - 2 ))]};
  # Assume the first line contains the correct problem size
  NELEM=`head -n1 $CSVFILE | awk -F, '{ print $1 }'`
  NNODE=`head -n1 $CSVFILE | awk -F, '{ print $2 }'`
  NUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3 }'`
  MUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3/1000000 }'`
  MDOFS=`head -n1 $CSVFILE | awk -F, '{ print $13/1000000 }'`
  #
  ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $MDOFS $MUDOF | bc`
  if [ $ITERS -lt $ITERS_MIN ]; then ITERS=10; fi
  ELEM_PER_PART=1000
  FINISHED=""
  while [ ! $FINISHED ]; do
    N=$(( $NELEM / $ELEM_PER_PART / $C * $C ))
    if [ "$N" -le 50000 ]; then
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
      echo "Partitioning and converting "$MESHNAME", if necessary..."
      exit
      $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR"
      echo "Running "$ITERS" iterations of "$MESHNAME" ("$MUDOF" MDOF), "\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
          -p $MESH >> $CSVFILE
      done
    fi
    ELEM_PER_PART=$(( $ELEM_PER_PART + 1000 ))
    if [[ $ELEM_PER_PART -ge 20000 ]]; then FINISHED=TRUE; fi
  done
  echo "Partitioning Profile" >> $PROFILE
fi
if [ -n "$CSV_HAS_PART_TEST" ]; then
  if [ ! -z "$HAS_GNUPLOT" ]; then
    echo "Plotting partitioning profile data: "$CSVFILE"..."
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set key inside top right;\
    set title 'Femera Elastic Performance Partitioning Tests [MDOF/s]';\
    set xlabel 'Partition Size [elem/part]';\
    plot 'perf/uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".csv'\
    using (\$1/\$4):(\$4 == \$9 ? 1/0:\$13/1e6)\
    with points pointtype 0\
    title '"$CPUCOUNT" Partitions';"\
    | tee -a $PROFILE | grep --no-group-separator -C25 --color=always '\.'
  fi
fi
#