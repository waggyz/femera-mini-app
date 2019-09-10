#!/bin/bash
CPUMODEL=`./cpumodel.sh`
CPUCOUNT=`./cpucount.sh`
CSTR=gcc
EXEDIR="."
GMSH2FMR=gmsh2fmr-$CPUMODEL-$CSTR
#
P=2; DOF_PER_ELEM=4;
C=$CPUCOUNT; N=$C; RTOL=1e-24;
TARGET_TEST_S=10;# Try for S sec/run
REPEAT_TEST_N=6;# Repeat each test N times
ITERS_MIN=10;
#
PERFDIR="perf"
PROFILE=$PERFDIR/"uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".pro"
LOGFILE=$PERFDIR/"uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".log"
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
echo Largest Test Model: $TET10_MAX Tet10, $NODE_MAX Nodes, $MDOF_MAX MDOF
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
  # with 10 iterations of the third-to-largest model
  if [ ! -f $CSVFILE ]; then
    ITERS=10; H=${LIST_H[$(( $TRY_COUNT - 3 ))]};
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    echo Estimating performance at\
      $(( ${NOMI_UDOF[$(( $TRY_COUNT - 3 ))]} / 1000000 )) MDOF...
    echo "Meshing, partitioning, and converting "$MESHNAME", if necessary..."
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" >> $LOGFILE
    echo Running $ITERS iterations of $MESHNAME...
    $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
      -p $MESH >> $CSVFILE
  fi
fi
MUDOF=`head -n1 $CSVFILE | awk -F, '{ print int($3/1e6) }'`
MDOFS=`head -n1 $CSVFILE | awk -F, '{ print int(($13+5e5)/1e6) }'`
DOFS=`head -n1 $CSVFILE | awk -F, '{ print int($13+0.5) }'`
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
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" >> $LOGFILE
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
  echo "Writing basic profile: "$PROFILE"..." >> $LOGFILE
  echo "Femera Performance Profile" > $PROFILE
  echo "femerq-"$CPUMODEL"-"$CSTR >> $PROFILE
  grep -m1 -i "model name" /proc/cpuinfo >> $PROFILE
  MEM_GB="`free -g  | grep Mem | awk '{print $2}'`"
  printf "      %6i\t: GB Memory\n" $MEM_GB >> $PROFILE
  #
  MDOFS=`head -n1 $CSVFILE | awk -F, '{ print $13/1e6 }'`
  NELEM=`head -n1 $CSVFILE | awk -F, '{ print $1 }'`
  NNODE=`head -n1 $CSVFILE | awk -F, '{ print $2 }'`
  NUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3 }'`
  MUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3/1e6 }'`
  NPART=`head -n1 $CSVFILE | awk -F, '{ print $4 }'`
  ITERS=`head -n1 $CSVFILE | awk -F, '{ print $5 }'`
  NCPUS=`head -n1 $CSVFILE | awk -F, '{ print $9 }'`
  echo >> $PROFILE
  echo "     Initial Elastic Model Performance Estimate" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "        %6.1f : Initial test performance [MDOF/s]\n" $MDOFS >> $PROFILE
  printf "        %6.1f : Initial test system Size [MDOF]\n" $MUDOF >> $PROFILE
  printf "%12i   : Initial model nodes\n" $NNODE >> $PROFILE
  printf "%12i   : Initial tet10 Elements\n" $NELEM >> $PROFILE
  printf "%12i   : Initial test partitions\n" $NPART >> $PROFILE
  printf "%12i   : Initial test threads\n" $NCPUS >> $PROFILE
  printf "%12i   : Initial test iterations\n" $ITERS >> $PROFILE
  #echo "Mesh            : " FIXME Put initial mesh filename here
  #
  echo >> $PROFILE
  echo "     Basic Performance Profile Test Parameters" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "%6i     : Partitions = Threads = Physical Cores\n" $CPUCOUNT >> $PROFILE
  printf "%6i     : Basic test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf "  %6.1f   : Basic test solve time [sec]\n" $TARGET_TEST_S>>$PROFILE
  printf "%6i     : Basic Minimum iterations\n" $ITERS_MIN >> $PROFILE
  printf "     %5.0e : Basic relative residual tolerance\n" $RTOL >> $PROFILE
  #
  SIZE_PERF_MAX=`awk -F, -v c=$CPUCOUNT -v max=0\
    '($9==c)&&($13>max)&&($4==$9){max=$13;perf=int(($13+5e5)/1e6);size=$3}\
    END{print int((size+50)/100)*100,int(perf+0.5)}'\
    $CSVFILE`
  MAX_MDOFS=${SIZE_PERF_MAX##* }
  MAX_SIZE=${SIZE_PERF_MAX%% *}
  echo "Maximum basic performance: "${SIZE_PERF_MAX##* }" MDOF/s"\
  at ${SIZE_PERF_MAX%% *}" DOF, parts = cores = "$CPUCOUNT"."
  #
  NODE_ELEM_MAX=`awk -F, -v c=$CPUCOUNT -v max=0\
    '($9==c)&&($13>max)&&($4==$9){max=$13;nelem=$1;nnode=$2}\
    END{print nelem,nnode}'\
    $CSVFILE`
  MAX_ELEMS=${NODE_ELEM_MAX%% *}
  MAX_NODES=${NODE_ELEM_MAX##* }
  if [ ! -z "$HAS_GNUPLOT" ]; then
    echo "Plotting basic profile data: "$CSVFILE"..." >> $LOGFILE
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set logscale x;\
    set xrange [1e3:1.05e9];\
    set yrange [0:];\
    set key inside top right;\
    set title 'Femera Basic Elastic Performance Tests [MDOF/s]';\
    set xlabel 'System Size [DOF]';\
    set label at "$MAX_SIZE", "$MAX_MDOFS" \"* Max\";\
    plot 'perf/uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".csv'\
    using 3:(\$4 != \$9 ? 1/0:\$13/1e6)\
    with points pointtype 0\
    title '"$CPUCOUNT" Partitions';"\
    | tee -a $PROFILE | grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
  echo "Writing basic profile results: "$PROFILE"..." >> $LOGFILE
  echo "        Basic Performance Profile Test Results" >> $PROFILE
  echo "  --------------------------------------------------" >> $PROFILE
  printf "        %6.1f : Basic performance maximum [MDOF/s]\n" $MAX_MDOFS >> $PROFILE
  printf "%12i   : Basic system Size [DOF]\n" $MAX_SIZE >> $PROFILE
  printf "%12i   : Basic model nodes\n" $MAX_NODES >> $PROFILE
  printf "%12i   : Basic tet10 Elements\n" $MAX_ELEMS >> $PROFILE
  #
  # Find the max. performing model
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    H=${LIST_H[I]}
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    CHECK_NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    if [ $CHECK_NNODE -eq $MAX_NODES ]; then
      MED_MESHNAME=$MESHNAME
      MED_MESH=$MESH
    fi
  done
  #
  MED_NELEM=`awk -F, -v n=$MAX_NODES '($2==n){ print $1; exit }' $CSVFILE`
  MED_NNODE=`awk -F, -v n=$MAX_NODES '($2==n){ print $2; exit }' $CSVFILE`
  MED_NUDOF=`awk -F, -v n=$MAX_NODES '($2==n){ print $3; exit }' $CSVFILE`
  #MED_MUDOF=`awk -F, -v n=$MAX_NODES '($2==n){ print int($3/1e6); exit }' $CSVFILE`
  MED_MDOFS=`awk -F, -v n=$MAX_NODES '($2==n){ print int(($13+5e6)/1e6); exit }' $CSVFILE`
  #
  MED_ITERS=`printf '%f*%f/%f*1000000\n' $TARGET_TEST_S $MED_MDOFS $MED_NUDOF | bc`
  if [ $MED_ITERS -lt $ITERS_MIN ]; then MED_ITERS=10; fi
  echo "Writing medium model partitioning test parameters: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "  Medium Model Partitioning Test Parameters" >> $PROFILE
  echo "  -----------------------------------------" >> $PROFILE
  printf " %9i   : Medium model size [DOF]\n" $MED_NUDOF >> $PROFILE
  printf " %9i   : Medium model nodes\n" $MED_NNODE >> $PROFILE
  printf " %9i   : Medium tet10 Elements\n" $MED_NELEM >> $PROFILE
  printf " %9i   : Medium model test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf " %9i   : Medium model iterations\n" $MED_ITERS >> $PROFILE
  #
  ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $MDOFS $MUDOF | bc`
  if [ $ITERS -lt $ITERS_MIN ]; then ITERS=10; fi
  echo "Writing large model partitioning test parameters: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "  Large Model Partitioning Test Parameters" >> $PROFILE
  echo "  ----------------------------------------" >> $PROFILE
  printf " %9.1f : Large model size [MDOF]\n" $MUDOF >> $PROFILE
  printf " %7i   : Large model test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf " %7i   : Large model iterations\n" $ITERS >> $PROFILE
fi
  #
  #
# Check if any CSV lines have N > C
CSV_HAS_PART_TEST=`awk -F, -v e=$NELEM -v c=$CPUCOUNT\
  '($1==e)&&($9==c)&&($4>$9){print $4; exit}' $CSVFILE`
if [ -z "$CSV_HAS_PART_TEST" ]; then
  H=${LIST_H[$(( $TRY_COUNT - 3 ))]};
  ELEM_PER_PART=1000
  FINISHED=""
  while [ ! $FINISHED ]; do
    N=$(( $NELEM / $ELEM_PER_PART / $C * $C ))
    if [ "$N" -le 50000 ]; then
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
      echo "Partitioning and converting "$MESHNAME", if necessary..."
      $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" >> $LOGFILE
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
  # echo "Large Model Partitioning Profile" >> $PROFILE
fi
CSV_HAS_PART_TEST=`awk -F, -v c=$CPUCOUNT '($9==c)&&($4>$9){print $4; exit}' $CSVFILE`
if [ -n "$CSV_HAS_PART_TEST" ]; then
  SIZE_PERF_MAX=`awk -F, -v c=$CPUCOUNT -v elem=$NELEM -v max=0\
    '($9==c)&&($1==elem)&&($4>$9)&&($13>max){max=$13;perf=$13/1e6;size=$1/$4}\
    END{print int((size+50)/100)*100,int(perf+0.5)}'\
    $CSVFILE`
  LARGE_MDOFS=${SIZE_PERF_MAX##* }
  LARGE_ELEM_PART=${SIZE_PERF_MAX%% *}
  echo "Large model performance peak: "$LARGE_MDOFS" MDOF/s"\
    "at "$LARGE_ELEM_PART" elem/part."
  LARGE_ELEM=$(( $LARGE_ELEM_PART * $CPUCOUNT ))
  LARGE_UDOF=$(( $LARGE_ELEM * $DOF_PER_ELEM ))
  echo "Large model size initial estimate:"\
    ">"$LARGE_ELEM" elem,"$LARGE_UDOF" DOF."
  if [ ! -z "$HAS_GNUPLOT" ]; then
    MUDOF=`head -n1 $CSVFILE | awk -F, '{ print int($3/1e6) }'`
    echo "Plotting large model partitioning profile data: "$CSVFILE"..." >> $LOGFILE
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set key inside bottom center;\
    set title 'Femera Large Elastic Model Partitioning Tests [MDOF/s]';\
    set xrange [0:20000];\
    set xlabel 'Partition Size [elem/part]';\
    set label at "$LARGE_ELEM_PART", "$LARGE_MDOFS" \"* Max\";\
    plot 'perf/uhxt-tet10-elas-ort-"$CPUMODEL"-"$CSTR".csv'\
    using (\$1/\$4):((\$4 > \$9)&&( \$1 == $NELEM ) ? \$13/1e6:1/0)\
    with points pointtype 0\
    title 'Performance at $MUDOF MDOF';"\
    | tee -a $PROFILE | grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
  echo "Writing large model partitioning test results: "$PROFILE"..." >> $LOGFILE
  echo "        Large Model Partitioning Test Results" >> $PROFILE
  echo "  -------------------------------------------------" >> $PROFILE
  printf " %9i : Large model size estimate [DOF]\n" $LARGE_UDOF >> $PROFILE
  printf " %9i : Large model partition size [elem/part]\n" $LARGE_ELEM_PART\
    >> $PROFILE
fi
#
