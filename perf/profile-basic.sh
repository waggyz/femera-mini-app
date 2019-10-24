#!/bin/bash
if [ -n "$1" ]; then P=$1; else P=2; fi
if [ -n "$2" ]; then PHYS=$2; else PHYS=elas-ort; fi
if [ -n "$3" ]; then CSTR=$3; else CSTR=gcc; fi
if [ -n "$4" ]; then MEM=$(( $4 * 1000000000 ));
  else MEM=`free -b  | grep Mem | awk '{print $7}'`; fi
if [ -n "$5" ]; then CPUMODEL=$5; else CPUMODEL=`./cpumodel.sh`; fi
if [ -n "$6" ]; then CPUCOUNT=$6; else CPUCOUNT=`./cpucount.sh`; fi
#
EXEDIR="."
GMSH2FMR=gmsh2fmr-$CPUMODEL-$CSTR
#
if [ -d "/hpnobackup1/dwagner5/femera-test/cube" ]; then
  MESHDIR=/hpnobackup1/dwagner5/femera-test/cube
else
  MESHDIR=cube
fi
echo "Mesh Directory: "$MESHDIR"/"
#
if [[ `hostname` == k3* ]]; then #FIXME Silly little hack
  MEM=30000000000
  module add gnuplot_5.0.5
fi
if [[ `hostname` == k4* ]]; then #FIXME Silly little hack
  module add gnuplot_5.0.5
fi
echo `free -g  | grep Mem | awk '{print $7}'` GB Available Memory
echo $(( $MEM / 1000000000 )) GB Assumed Memory
#
HAS_GNUPLOT=`which gnuplot`
#
case $P in
1) DOF_PER_ELEM=0.5; BYTE_PER_DOF=340; PSTR=tet4;  ;;
2) DOF_PER_ELEM=4;   BYTE_PER_DOF=120; PSTR=tet10; ;;
3) DOF_PER_ELEM=14;  BYTE_PER_DOF=100; PSTR=tet20; ;;
esac
#
C=$CPUCOUNT; N=$C; RTOL=1e-24;
TARGET_TEST_S=10;# Try for S sec/run
REPEAT_TEST_N=6;# Repeat each test N times
ITERS_MIN=10;
#LARGE_PART_MAX_MUDOF=27;
LARGE_PART_MAX_MUDOF=55;
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
#
PERFDIR="perf"
PROFILE=$PERFDIR/"uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".pro"
LOGFILE=$PERFDIR/"uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".log"
CSVFILE=$PERFDIR/"uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
CSVSMALL=$PERFDIR/"small-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
CSVPROFILE=$PERFDIR/"profile-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
#
if [ -f $PROFILE ]; then
  NODE_MAX=`grep -m1 -i nodes $PROFILE | awk '{print $1}'`
  UDOF_MAX=$(( $NODE_MAX * 3 ))
  TET10_MAX=`grep -m1 -i elements $PROFILE | awk '{print $1}'`
else
  UDOF_MAX=$(( $MEM / $BYTE_PER_DOF ))
  NODE_MAX=$(( $UDOF_MAX / 3))
  TET10_MAX=$(( $UDOF_MAX / $DOF_PER_ELEM ))
fi
MDOF_MAX=$(( $UDOF_MAX / 1000000 ))
echo Largest Test Model: $TET10_MAX $PSTR, $NODE_MAX Nodes, $MDOF_MAX MDOF
#
case $P in
1)
  NOMI_N=20
  NOMI_UDOF=(500 1000 2000 5000 10000 25000 \
  50000 100000 250000 500000 1000000\
    1500000 2500000 5000000 10000000 25000000\
    50000000 100000000 250000000 500000000 1000000000)
  LIST_H=(4 6 8 10 14 19  24 30 41 52 66  91 114 144 195  246 310 421 531 669)
  LIST_HH="4 6 8 10 14 19  24 30 41 52 66  77 91 114 144 195  246 310 421 531 669"
  ;;
2)
  NOMI_N=20
  NOMI_UDOF=(500 1000 2500 5000 10000 25000 \
  50000 100000 250000 500000 1000000\
    1500000 2500000 5000000 10000000 25000000\
    50000000 100000000 250000000 500000000 1000000000)
  LIST_H=(2 3 4 5 7 9   12 15 21 26 33   38 45 56 72 96   123 156 205 265 336)
  LIST_HH="2 3 4 5 7 9   12 15 21 26 33   38 45 56 72 96   123 156 205 265 336"
  ;;
3)
  NOMI_N=19
  NOMI_UDOF=(500 2000 5000 10000 25000 \
  50000 100000 250000 500000 1000000\
    1500000 2500000 5000000 10000000 25000000\
    50000000 100000000 250000000 500000000 1000000000)
  LIST_H=(1 2 3 4 6   7 9 13 17 21  24 29 37 46 63   80 101 138 174 220)
  LIST_HH="1 2 3 4 6   7 9 13 17 21  24 29 37 46 63   80 101 138 174 220"
  ;;
esac
TRY_COUNT=0;
for I in $(seq 0 $NOMI_N); do
  if [ ${NOMI_UDOF[I]} -lt $UDOF_MAX ]; then
    TRY_COUNT=$(( $TRY_COUNT + 1))
  fi
  LP=$(( $LARGE_PART_MAX_MUDOF * 1000000 ))
  if [ ${NOMI_UDOF[I]} -lt $LP ]; then
    H=${LIST_H[I]}
    FIRST_UDOF=${NOMI_UDOF[I]}
  fi
done
#
if [ ! -f $PROFILE ]; then
  # First, get a rough idea of DOF/sec to estimate time
  if [ ! -f $CSVFILE ]; then
    export OMP_SCHEDULE=static
    export OMP_PLACES=cores
    export OMP_PROC_BIND=spread
    ITERS=$ITERS_MIN;# H=${LIST_H[$(( $TRY_COUNT - 3 ))]};
    C=$CPUCOUNT
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
    echo Estimating performance at\
      $(( $FIRST_UDOF / 1000000 )) MDOF...
    #  $(( ${NOMI_UDOF[$(( $TRY_COUNT - 3 ))]} / 1000000 )) MDOF...
    #echo "Meshing, partitioning, and converting "$MESHNAME", if necessary..."
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
    echo Running $ITERS iterations of $MESHNAME...
    $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
      -p $MESH >> $CSVFILE
  fi
fi
if [ -f $CSVFILE ]; then
  echo "Femera Performance Profile" > $PROFILE
  echo "Writing maximum problem size estimate: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "        Maximum Elastic Model Size Estimate" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "     %9i : Maximum "$PSTR" Elements\n" $TET10_MAX >> $PROFILE
  printf "     %9i : Maximum Nodes\n" $NODE_MAX >> $PROFILE
  printf "    %9.0f  : Maximum MDOF\n" $MDOF_MAX >> $PROFILE
  #
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
  echo "Writing initial performance estimate: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "     Initial Elastic Model Performance Estimate" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "        %6.1f : Initial test performance [MDOF/s]\n" $MDOFS >> $PROFILE
  printf "        %6.1f : Initial test system Size [MDOF]\n" $MUDOF >> $PROFILE
  printf "%12i   : Initial model nodes\n" $NNODE >> $PROFILE
  printf "%12i   : Initial "$PSTR" Elements\n" $NELEM >> $PROFILE
  printf "%12i   : Initial test partitions\n" $NPART >> $PROFILE
  printf "%12i   : Initial test threads\n" $NCPUS >> $PROFILE
  printf "%12i   : Initial test iterations\n" $ITERS >> $PROFILE
  #echo "Mesh            : " FIXME Put initial mesh filename here
fi
if [ -f $CSVFILE ]; then
  INIT_MUDOF=`head -n1 $CSVFILE | awk -F, '{ print int($3/1e6) }'`
  INIT_MDOFS=`head -n1 $CSVFILE | awk -F, '{ print int(($13+5e5)/1e6) }'`
  INIT_DOFS=`head -n1 $CSVFILE | awk -F, '{ print int($13+0.5) }'`
  echo "Initial performance estimate: "$INIT_MDOFS" MDOF/s at "$INIT_MUDOF" MDOF"
  #
  echo "Writing basic profile test parameters: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "     Basic Performance Profile Test Parameters" >> $PROFILE
  echo "  ------------------------------------------------" >> $PROFILE
  printf "%6i     : Partitions = Threads = Physical Cores\n" $CPUCOUNT >> $PROFILE
  printf "%6i     : Basic test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf "  %6.1f   : Basic test solve time [sec]\n" $TARGET_TEST_S>>$PROFILE
  printf "%6i     : Basic Minimum iterations\n" $ITERS_MIN >> $PROFILE
  printf "     %5.0e : Basic relative residual tolerance\n" $RTOL >> $PROFILE
  #
  if false; then
    echo Removing old partitioned meshes...
    for PP in $(seq 1 3 ); do
    for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
      H=${LIST_H[I]}
      echo $MESHDIR"/uhxt"$H"p"$PP"/*.msh2, *n????*.msh, *.fmr"
      #
      find $MESHDIR"/uhxt"$H"p"$PP -maxdepth 1 -type f -name *.msh2 -delete
      find $MESHDIR"/uhxt"$H"p"$PP -maxdepth 1 -type f -name "uhxt"$H"p"$PP"n"????*.msh -delete
      find $MESHDIR"/uhxt"$H"p"$PP -maxdepth 1 -type f -name *.fmr -delete
    done
    done
    #exit 0
  fi
  ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_MDOFS $INIT_MUDOF | bc`
  CSVLINES=`wc -l < $CSVFILE`
  BASIC_TEST_N=$(( $TRY_COUNT * $REPEAT_TEST_N + 1 ))
  if [ "$CSVLINES" -lt "$BASIC_TEST_N" ]; then
    echo Running basic profile tests...
    C=$CPUCOUNT
    export OMP_SCHEDULE=static
    export OMP_PLACES=cores
    export OMP_PROC_BIND=spread
    for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
      H=${LIST_H[I]}
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
      $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
      NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
      NDOF=$(( $NNODE * 3 ))
      NDOF90=$(( $NDOF * 9 / 10 ))
      ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
      if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
      if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
      if [ $I -eq 0 ]; then
        echo Warming up...
        for I in $(seq 1 $REPEAT_TEST_N ); do
          $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
          -p $MESH > /dev/null
        done
      fi
      echo "Running "$ITERS" iterations of "$MESHNAME" ("$NDOF" DOF),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
        -p $MESH >> $CSVFILE
      done
    done
  fi
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
    plot 'perf/uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv'\
    using 3:(\$4 != \$9 ? 1/0:\$13/1e6)\
    with points pointtype 0\
    title '"$CPUCOUNT" Partitions';\
    "\
    | tee -a $PROFILE ;#| grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
  echo "Writing basic profile results: "$PROFILE"..." >> $LOGFILE
  echo "        Basic Performance Profile Test Results" >> $PROFILE
  echo "  --------------------------------------------------" >> $PROFILE
  printf "        %6.1f : Basic performance maximum [MDOF/s]\n" $MAX_MDOFS >> $PROFILE
  printf "%12i   : Basic system size [DOF]\n" $MAX_SIZE >> $PROFILE
  printf "%12i   : Basic model nodes\n" $MAX_NODES >> $PROFILE
  printf "%12i   : Basic "$PSTR" Elements\n" $MAX_ELEMS >> $PROFILE
  #
  # Find the max. performing model
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    H=${LIST_H[I]}
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    CHECK_NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    if [ $CHECK_NNODE -eq $MAX_NODES ]; then
      MED_UDOF=$((CHECK_NNODE * 3 ));
      MED_MESHNAME=$MESHNAME
      MED_MESH=$MESH
      MED_H=$H
    fi
  done
  # Find a representative large model
  if [ $(( $MED_UDOF * 3 * 20  )) -lt $LARGE_PART_MAX_MUDOF ]; then
    LARGE_PART_MAX_MUDOF=$(( $MED_UDOF * 3 * 20 / 1000000 ))
  fi
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    H=${LIST_H[I]}
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    CHECK_NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    if [ $(( $CHECK_NNODE * 3 / 1000000 )) -le $LARGE_PART_MAX_MUDOF ]; then
      LRG_MUDOF=$((CHECK_NNODE * 3 / 1000000 ));
      LRG_MESHNAME=$MESHNAME
      LRG_MESH=$MESH
      LRG_H=$H
    fi
  done
  MED_NELEM=$MAX_ELEMS;#`awk -F, -v n=$MAX_NODES '($2==n){ print $1; exit }' $CSVFILE`
  MED_NNODE=$MAX_NODES;#`awk -F, -v n=$MAX_NODES '($2==n){ print $2; exit }' $CSVFILE`
  MED_NUDOF=$(( $MAX_NODES * 3 ));#`awk -F, -v n=$MAX_NODES '($2==n){ print $3; exit }' $CSVFILE`
  #MED_MUDOF=`awk -F, -v n=$MAX_NODES '($2==n){ print int($3/1e6); exit }' $CSVFILE`
  #MED_MDOFS=`awk -F, -v n=$MAX_NODES '($2==n){ print int(($13+5e6)/1e6); exit }' $CSVFILE`
  MED_MDOFS=$MAX_MDOFS
  #
  MED_ITERS=`printf '%f*%f*1000000/%f\n' $TARGET_TEST_S $MED_MDOFS $MED_NUDOF | bc`
  if [ $MED_ITERS -lt $ITERS_MIN ]; then MED_ITERS=10; fi
  echo "Writing medium model partitioning test parameters: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "  Medium Model Partitioning Test Parameters" >> $PROFILE
  echo "  -----------------------------------------" >> $PROFILE
  printf " %9i   : Medium test model size [DOF]\n" $MED_NUDOF >> $PROFILE
  printf " %9i   : Medium test model nodes\n" $MED_NNODE >> $PROFILE
  printf " %9i   : Medium test "$PSTR" Elements\n" $MED_NELEM >> $PROFILE
  printf " %9i   : Medium test iterations\n" $MED_ITERS >> $PROFILE
  printf " %9i   : Medium test repeats\n" $REPEAT_TEST_N >> $PROFILE
  #
  #if [ $MUDOF -gt $LARGE_PART_MAX_MUDOF ];
  #  then LRG_MUDOF=$LARGE_PART_MAX_MUDOF;
  #  else LRG_MUDOF=$MUDOF;
  #fi
  ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $MDOFS $LRG_MUDOF | bc`
  if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
  echo "Writing large model partitioning test parameters: "$PROFILE"..." >> $LOGFILE
  echo >> $PROFILE
  echo "    Large Model Partitioning Test Parameters" >> $PROFILE
  echo "  --------------------------------------------" >> $PROFILE
  printf " %9.1f : Large test model size [MDOF]\n" $LRG_MUDOF >> $PROFILE
  printf " %7i   : Large test model test repeats\n" $REPEAT_TEST_N >> $PROFILE
  printf " %7i   : Large test iterations\n" $ITERS >> $PROFILE
fi
if false; then rm $CSVSMALL; fi
if [ ! -f $CSVSMALL ]; then # Run small model tests
  echo Running concurrent small model tests...
  export OMP_PLACES=cores; export OMP_PROC_BIND=spread,close; \
  export OMP_NESTED=true; export OMP_MAX_ACTIVE_LEVELS=2; \
  P=2;
  S=100; X=2; N=1;
  #LIST_C=(16 8 4 2 1)
  # Get all factors of the number of cores
  IX=0;
  for I in $(seq $CPUCOUNT -1 1); do
    #[ $(expr $CPUCOUNT / $I \* $I) == $CPUCOUNT ] && LIST_C=$LIST_C" "$I
    [ $(expr $CPUCOUNT / $I \* $I) == $CPUCOUNT ] && ARRAY_X[$IX]=$I
    [ $(expr $CPUCOUNT / $I \* $I) == $CPUCOUNT ] && IX=$(( $IX + 1 ))
  done
  #for C in $LIST_C; do echo C is $C; done
  #for H in $LIST_HH; do echo H is $H; done
  #for XIX in $(seq 0 3); do echo C is ${ARRAY_C[XIX]}; done
  for SIZE_MULT in 1 2 4 8; do
  THIS_MAX=$(( $MAX_SIZE * $SIZE_MULT / 2 ))
  HIX=0; XIX=0; X=2;
  if true; then
  NDOF=0
  while (( $NDOF < $THIS_MAX && $X > 1 )); do
    H=${LIST_H[HIX]}
    X=${ARRAY_X[XIX]}
    C=$(( $CPUCOUNT / $X ))
    N=$C;
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
    #echo "Meshing, partitioning, and converting "$MESHNAME", if necessary..."
    $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
    NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    NDOF=$(( $NNODE * 3 ))
    NDOF90=$(( $NDOF * 9 / 10 ))
    ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
    if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
    if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
    #echo $(( $NDOF * $X )) '<' $(( $THIS_MAX ))
    #S=$(( 500000 / $NDOF ))
    #M100S=$(( $MDOF_MAX * $TARGET_TEST_S * 100 ))
    #S=$(( $TARGET_TEST_S * $INIT_DOFS / $NDOF / $ITERS))
    S=`printf '%f*%f/%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF $ITERS | bc`
    #S=$(( $S / 2 ))
    if (( $S < 1 )); then
      S=1;
      ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
    fi
    if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
    if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
    while (( $NDOF * $X > $THIS_MAX && $X > 0 )); do
      XIX=$(( $XIX + 1 ));
      X=${ARRAY_X[XIX]}
      if [ -z "$X" ]; then X=0; fi
    done
    if [ $X -gt 1 ]; then
      C=$(( $CPUCOUNT / $X ))
      N=$C;
      if [ -f $CSVSMALL ]; then
        if [ $(( $NDOF * $X )) -lt $(( $THIS_MAX )) ]; then
        HAS_TEST=`awk -F, -v n=$NNODE -v c=$C\
          '($2==n)&&($9==c){print $4; exit}' $CSVSMALL`
        fi
      else
        HAS_TEST=""
      fi
      if [ -z "$HAS_TEST" ]; then
        M=$(( $S * $X ))
        if [ $M -gt 10000 ]; then
          MC=$(( 10000 / $CPUCOUNT ))
          M=$(( $MC * $CPUCOUNT ));
          S=$(( $M / $X ))
        fi
        MESHNAME="uhxt"$H"p"$P"n"$N
        MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
        #echo "Partitioning and converting "$MESHNAME", if necessary..."
        $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
        if [ -f $MESH"_1.fmr" ]; then
          echo "Running "$REPEAT_TEST_N" repeats of "$S"x"$X" concurrent "$NDOF" DOF models..."
          START=`date +%s.%N`
          #
          for I in $(seq 1 $REPEAT_TEST_N ); do
            $EXEDIR"/femera-mmq-"$CPUMODEL"-"$CSTR -v1 -m$M -n$X -c$N \
              -i$ITERS -r$RTOL -p $MESH >> $CSVSMALL
          done
          #
          STOP=`date +%s.%N`
          TIME_SEC=`printf "%f-%f\n" $STOP $START | bc`
          #
          MDOF_TOTAL=`awk -F, -v n=$NNODE -v c=$N -v dof=0\
          '($2==n)&&($9==c){dof=dof+$3*$5;}\
            END{print dof/1000000;}' $CSVSMALL`
          TOTAL_MDOFS=`printf "%f/%f\n" $MDOF_TOTAL $TIME_SEC | bc`
          echo "Overall: "$TOTAL_MDOFS" MDOF/s ("$MDOF_TOTAL\
          "MDOF in "$TIME_SEC" sec)"
          #
          SOLVE_MDOFS=`awk -F, -v nnode=$NNODE -v c=$C -v nrun=0 -v mdofs=0 -v x=$X\
          '($2==nnode)&&($9==c){nrun=nrun+1;mdofs=mdofs+$13;}\
            END{print mdofs/nrun/1000000*x;}' $CSVSMALL`
          echo " Solver: "$SOLVE_MDOFS" MDOF/s at "$X"x "$NDOF" DOF"\
            "models ("$(( $X * $NDOF ))" DOF concurrent)..." 
        fi
      fi
    fi
    HIX=$(( $HIX + 1 ))
  done
  fi
  done
fi
if [ -f $CSVSMALL ]; then
  if [ -f $CSVPROFILE ]; then rm $CSVPROFILE; fi
  NODE_ARRAY=($( cut -d',' -f2 $CSVSMALL | uniq ))
  for NNODE in "${NODE_ARRAY[@]}"; do
      NDOF=$(( $NNODE * 3 ))
      SOLVE_MDOFS=`awk -F, -v nnode=$NNODE -v nrun=0 -v mdofs=0 -v c=$CPUCOUNT\
      '($2==nnode){nrun=nrun+1;mdofs+=$13;cc=$9;}\
        END{print mdofs/(nrun==0?1:nrun)/1000000*c/(cc==0?1:cc);}' $CSVSMALL`
      if [ "$SOLVE_MDOFS" != "0" ]; then
        if false; then
          echo "Average: "$SOLVE_MDOFS" MDOF/s with "$NDOF" DOF models..."
        fi
        for C in $(seq 1 $(( $CPUCOUNT / 2 )) ); do
          HAS_TEST=`awk -F, -v n=$NNODE -v c=$C\
            '($2==n)&&($9==c){print $4; exit}' $CSVSMALL`
          if [ ! -z "$HAS_TEST" ]; then
          awk -F, -v nnode=$NNODE -v nrun=0 -v c=$C -v mdofs=0 -v ctot=$CPUCOUNT\
            'BEGIN{OFS=",";t10=0;t11=0;t12=0;}\
            ($2==nnode)&&($9==c){nrun+=1;t10+=$10;t11+=$11;t12+=$12;mdofs+=$13;\
              e=$1;n=$2;f=$3;p=$4;i1=$5;i2=$6;r1=$7;r2=$8;cc=$9;}\
            END{print e,n,f,p,i1,i2,r1,r2,cc,t10/nrun,t11/nrun,t12/nrun,\
            mdofs/(nrun==0?1:nrun)*ctot/(cc==0?1:cc)}'\
            $CSVSMALL >> $CSVPROFILE
          fi
        done
      fi
  done;
  if [ ! -z "$HAS_GNUPLOT" ]; then
    echo "Plotting small model profile data: "$CSVSMALL"..." >> $LOGFILE
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set logscale x;\
    set xrange [1e3:1.05e9];\
    set yrange [0:];\
    set key inside top right;\
    set title 'Femera Small Model Elastic Performance Tests [MDOF/s]';\
    set xlabel 'System Size [DOF]';\
    plot \
    '"$CSVPROFILE"'\
    using 3:(\$13/1e6)\
    with points pointtype 19\
    title 'Small Model Average',\
    '"$CSVSMALL"'\
    using 3:(\$9<$CPUCOUNT""?"$CPUCOUNT"/\$9*\$13/1e6:1/0)\
    with points pointtype 0\
    title 'Small Model Tests';\
    "\
    | tee -a $PROFILE ;#| grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
fi
# Check if any medium model CSV lines have N > C
CSV_HAS_MEDIUM_PART_TEST=`awk -F, -v e=$MED_NELEM -v c=$CPUCOUNT\
  '($1==e)&&($9==c)&&($4>$9){print $4; exit}' $CSVFILE`
if [ -z "$CSV_HAS_MEDIUM_PART_TEST" ]; then
  echo Running medium model partitioning tests...
  C=$CPUCOUNT
  export OMP_PROC_BIND=spread
  export OMP_SCHEDULE=static
  export OMP_PLACES=cores
  for N in $(seq $(( $CPUCOUNT * 2 )) $CPUCOUNT $(( $CPUCOUNT * 16 )) ); do
    MESHNAME="uhxt"$MED_H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$MED_H"p"$P/$MESHNAME
    #echo "Partitioning and converting "$MESHNAME", if necessary..."
    $PERFDIR/mesh-uhxt.sh $MED_H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
    if (( $N == $CPUCOUNT )); then
      echo "Warming up..."
      $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$MED_ITERS -r$RTOL\
        -p $MESH >> /dev/null
    fi
    echo "Running "$MED_ITERS" iterations of "$MESHNAME" ("$MED_NUDOF" DOF),"\
      $REPEAT_TEST_N" times..."
    for I in $(seq 1 $REPEAT_TEST_N ); do
      $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$MED_ITERS -r$RTOL\
        -p $MESH >> $CSVFILE
    done
  done
fi
# Check if any medium model CSV lines have N > C
CSV_HAS_MEDIUM_PART_TEST=`awk -F, -v e=$MED_NELEM -v c=$CPUCOUNT\
  '($1==e)&&($9==c){print $4; exit}' $CSVFILE`
if [ ! -z "$CSV_HAS_MEDIUM_PART_TEST" ]; then
  SIZE_PERF_MAX=`awk -F, -v c=$CPUCOUNT -v elem=$MED_NELEM -v max=0\
    '($9==c)&&($1==elem)&&($13>max){max=$13;perf=$13/1e6;size=$4}\
    END{print size,int(perf+0.5)}'\
    $CSVFILE`
  MED_MDOFS=${SIZE_PERF_MAX##* }
  MED_PART=${SIZE_PERF_MAX%% *}
  echo "Medium model performance peak: "$MED_MDOFS" MDOF/s"\
    "with "$MED_PART" partitions."
  if [ ! -z "$HAS_GNUPLOT" ]; then
    #MED_MUDOF=`head -n1 $CSVFILE | awk -F, '{ print int($3/1e6) }'`
    echo "Plotting medium model partitioning profile data: "$CSVFILE"..." >> $LOGFILE
    gnuplot -e  "\
    set terminal dumb noenhanced size 79,25;\
    set datafile separator ',';\
    set tics scale 0,0;\
    set key inside bottom center;\
    set title 'Femera Medium Elastic Model Partitioning Tests [MDOF/s]';\
    set xrange [0:$(($CPUCOUNT * 16 ))];\
    set xlabel 'Number of Partitions';\
    set label at "$MED_PART", "$MED_MDOFS" \"* Max\";\
    plot 'perf/uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv'\
    using (\$4):(( \$1 == $MED_NELEM ) ? \$13/1e6:1/0)\
    with points pointtype 0\
    title 'Performance at $MED_NUDOF DOF';"\
    | tee -a $PROFILE;# | grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
  echo "Writing medium model partitioning test results: "$PROFILE"..." >> $LOGFILE
  echo "        Medium Model Partitioning Test Results" >> $PROFILE
  echo "  -------------------------------------------------" >> $PROFILE
  printf " %9i : Medium model partitions [part]\n" $MED_PART >> $PROFILE
  printf " %9i : Medium model test size [DOF]\n" $MED_NUDOF >> $PROFILE
  printf " %9i : Medium model performance [MDOF/s]\n" $MED_MDOFS >> $PROFILE
    >> $PROFILE
fi
#if (( $MED_PART > $CPUCOUNT ));then
# Check if any large model CSV lines have N > C
LARGE_NELEM=`head -n1 $CSVFILE | awk -F, '{ print $1 }'`
CSV_HAS_LARGE_PART_TEST=`awk -F, -v e=$LARGE_NELEM -v c=$CPUCOUNT\
  '($1>=e)&&($9==c)&&($4>$9){print $4; exit}' $CSVFILE`
if [ -z "$CSV_HAS_LARGE_PART_TEST" ]; then
  echo "Running large ("$LRG_MUDOF" MDOF) model partitioning tests..."
  export OMP_SCHEDULE=static
  export OMP_PLACES=cores
  export OMP_PROC_BIND=spread
  C=$CPUCOUNT
  #H=${LIST_H[$(( $TRY_COUNT - 3 ))]};
  ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $MDOFS $LRG_MUDOF | bc`
  if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
  ELEM_PER_PART=1000
  FINISHED=""
  while [ ! $FINISHED ]; do
    Ntarget=$(( $LARGE_NELEM / $ELEM_PER_PART ))
    if [ $Ntarget -ge 1000 ]; then
      NXYZ=($(python perf/part_slice_xyz.py -n $Ntarget -c $C))
      N=${NXYZ[0]}
    else
      N=$Ntarget
    fi
    #if [ "$N" -le 50000 ]; then
      MESHNAME="uhxt"$LRG_H"p"$P"n"$Ntarget
      MESH=$MESHDIR"/uhxt"$LRG_H"p"$P/$MESHNAME
      #echo "Partitioning and converting "$MESHNAME", if necessary..."
      $PERFDIR/mesh-uhxt.sh $LRG_H $P $Ntarget "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
      echo "Running "$ITERS" iterations of "$MESHNAME\
        "("$ELEM_PER_PART" elem/part),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
          -p $MESH >> $CSVFILE
      done
    #fi
    ELEM_PER_PART=$(( $ELEM_PER_PART + 1000 ))
    #if [[ $ELEM_PER_PART -gt 20000 ]]; then FINISHED=TRUE; fi
    if [[ $N -lt $(( $MED_PART / 2 )) ]]; then FINISHED=TRUE; fi
    if [[ $N -lt $(( $C )) ]]; then FINISHED=TRUE; fi
  done
  # echo "Large Model Partitioning Profile" >> $PROFILE
fi
CSV_HAS_LARGE_PART_TEST=`awk -F, -v c=$CPUCOUNT -v elem=$LARGE_NELEM \
  '($9==c)&&($1==elem)&&($4>$9){print $4; exit}' $CSVFILE`
if [ ! -z "$CSV_HAS_LARGE_PART_TEST" ]; then
  SIZE_PERF_MAX=`awk -F, -v c=$CPUCOUNT -v elem=$LARGE_NELEM -v max=0\
    '($9==c)&&($1==elem)&&($13>max+0){max=$13;perf=$13/1e6;size=$1/$4}\
    END{print int((size+500)/1000)*1000,int(perf+0.5)}'\
    $CSVFILE`
  LARGE_MDOFS=${SIZE_PERF_MAX##* }
  LARGE_ELEM_PART=${SIZE_PERF_MAX%% *}
  #LARGE_ELEM_PART=$(( ${SIZE_PERF_MAX%% *} /1000 *1000 ))
  echo "Large model performance peak: "$LARGE_MDOFS" MDOF/s"\
    "with "$LARGE_ELEM_PART" elem/part."
  #LARGE_ELEM=$(( $LARGE_ELEM_PART * $CPUCOUNT ))
  #LARGE_UDOF=$(( $LARGE_ELEM * $DOF_PER_ELEM ))
  #echo "Large model size initial estimate:"\
  #  ">"$LARGE_ELEM" elem,"$LARGE_UDOF" DOF."
  LARGE_ELEM_MIN=$(( $MED_PART * $LARGE_ELEM_PART ))
  LARGE_UDOF_MIN=$(( $LARGE_ELEM_MIN * $DOF_PER_ELEM ))
  #LARGE_MDOF_MIN=$(( $LARGE_UDOF_MIN / 1000000 ))
  echo "Large model size estimate:"\
    ">"$LARGE_ELEM_MIN" elem, >"$LARGE_UDOF_MIN" DOF."
  if [ ! -z "$HAS_GNUPLOT" ]; then
    #LRG_MUDOF=`head -n1 $CSVFILE | awk -F, '{ print int($3/1e6) }'`
    #if [ $LRG_MUDOF -gt $LARGE_PART_MAX_MUDOF ];
    #  then LRG_MUDOF=$LARGE_PART_MAX_MUDOF;
    #fi
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
    plot 'perf/uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv'\
    using (\$1/\$4):((\$4 > \$9)&&( \$1 == $LARGE_NELEM ) ? \$13/1e6:1/0)\
    with points pointtype 0\
    title 'Performance at $LRG_MUDOF MDOF';"\
    | tee -a $PROFILE;# | grep --no-group-separator -C25 --color=always '\*'
  else
    echo >> $PROFILE
  fi
  echo "Writing large model partitioning test results: "$PROFILE"..." >> $LOGFILE
  echo "        Large Model Partitioning Test Results" >> $PROFILE
  echo "  -------------------------------------------------" >> $PROFILE
  printf " %9i : Large model partition size [elem/part]\n" $LARGE_ELEM_PART\
    >> $PROFILE
  printf " %9i : Large model minimum size [DOF]\n" $LARGE_UDOF_MIN >> $PROFILE
  printf " %9i : Large test model size [DOF]\n" $LARGE_UDOF >> $PROFILE
  printf " %9i : Large test model performance [MDOF/s]\n" $LARGE_MDOFS >> $PROFILE
fi
if [ -f $CSVFILE ]; then
  CSV_HAS_FINAL_TEST=`awk -F, -v e=$LARGE_NELEM -v c=$CPUCOUNT\
    '($1>(e*1.5))&&($9>c){print $4; exit}' $CSVFILE`
else
  CSV_HAS_FINAL_TEST=""
fi
if [ -z "$CSV_HAS_FINAL_TEST" ]; then
  echo Running final profile tests...
  export OMP_SCHEDULE=static
  export OMP_PLACES=cores
  export OMP_PROC_BIND=spread
  C=$CPUCOUNT
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    H=${LIST_H[I]}
    N=1;
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
    NELEM=`grep -m1 -A1 -i elem $MESH".msh" | tail -n1`
    NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
    MN=$(( $MED_NELEM * 4 / 10 ))
    if (( $NELEM > $MN )); then
      NC=$(( $NELEM / $LARGE_ELEM_PART / $C ))
      N=$(( $NC * $C ))
      if (( $N >= 1000 )); then
        N=$(( $N / 200 * 200 ))
      fi
      if (( $N >= 10000 )); then
        N=$(( $N / 1000 * 1000 ))
      fi
      if (( $N < $MED_PART )); then N=$MED_PART; fi
      HAS_TEST=`awk -F, -v n=$NNODE -v p=$N\
        '($2==n)&&($4==p){print $4; exit}' $CSVFILE`
      if [ -z "$HAS_TEST" ]; then
        MESHNAME="uhxt"$H"p"$P"n"$N
        MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
        #echo "Meshing, partitioning, and converting "$MESHNAME", if necessary..."
        $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR" $PHYS
        #NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
        NDOF=$(( $NNODE * 3 ))
        NDOF90=$(( $NDOF * 9 / 10 ))
        ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
        if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
        if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
        echo "Running "$ITERS" iterations of "$MESHNAME" ("$NDOF" DOF),"\
          $REPEAT_TEST_N" times..."
        for I in $(seq 1 $REPEAT_TEST_N ); do
          $EXEDIR"/femerq-"$CPUMODEL"-"$CSTR -v1 -c$C -i$ITERS -r$RTOL\
            -p $MESH >> $CSVFILE
        done
      fi
    fi
  done
fi
#FIXME
CSV_HAS_FINAL_TEST="true"
if [ ! -z "$CSV_HAS_FINAL_TEST" ]; then
  #FIXME Change to loop through unique CSV col 1 values
  for I in $(seq 0 $(( $TRY_COUNT - 1)) ); do
    C=$CPUCOUNT
    H=${LIST_H[I]}
    N=1;
    MESHNAME="uhxt"$H"p"$P"n"$N
    MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
    NELEM=`grep -m1 -A1 -i elem $MESH".msh" | tail -n1`
    NC=$(( $NELEM / $LARGE_ELEM_PART / $C ))
    N=$(( $NC * $C ))
    if (( $N < $MED_PART )); then N=$MED_PART; fi
    awk -F, -v nelem=$NELEM -v parts=$N\
      'BEGIN{OFS=",";dofs=0;}\
      ($1==nelem)&&($4==parts)&&($13>dofs+0){\
        e=$1;n=$2;f=$3;p=$4;i1=$5;i2=$6;r1=$7;r2=$8;cc=$9;}\
        t10=$10;t11=$11;t12=$12;dofs=$13;\
      END{print e,n,f,p,i1,i2,r1,r2,cc,t10,t11,t12,dofs}'\
      $CSVFILE >> $CSVPROFILE
  done
fi
if [ ! -z "$HAS_GNUPLOT" ]; then
  echo "Plotting profile data: "$CSVSMALL"..." >> $LOGFILE
  gnuplot -e  "\
  set terminal dumb noenhanced size 79,25;\
  set datafile separator ',';\
  set tics scale 0,0;\
  set logscale x;\
  set xrange [1e3:1.05e9];\
  set yrange [0:];\
  set key inside top right;\
  set title 'Femera Elastic Performance Tests [MDOF/s]';\
  set xlabel 'System Size [DOF]';\
  plot\
  '"$CSVPROFILE"' using 3:(\$9<$CPUCOUNT)?(\$13/1e6):1/0\
    with points pointtype 19 title 'Concurrent Small Models',\
  '"$CSVPROFILE"' using 3:(\$4==$MED_PART)?(\$13/1e6):1/0\
    with points pointtype 13 title 'Medium Model [$MED_PART partitions]',\
  '"$CSVPROFILE"' using 3:(\$4>$MED_PART)?(\$13/1e6):1/0\
    with points pointtype 12 title 'Large Model [$LARGE_ELEM_PART elem/part]',\
  '"$CSVPROFILE"' using 3:(\$4==\$9)&&(\$9==$CPUCOUNT)?(\$13/1e6):1/0\
    with points pointtype 0 title 'C=N="$CPUCOUNT"';\
  "\
  | tee -a $PROFILE ;#| grep --no-group-separator -C25 --color=always '\*'
  #FIXME Removed small model profile because it's wrong.
  #FIXME I think some tests are repeated, and not accounted for correctly.
else
  echo >> $PROFILE
fi
#
