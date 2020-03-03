#!/bin/bash
if [ -n "$1" ]; then PLIST=$1; else PLIST="2 3 1" ; fi
if [ -n "$2" ]; then PHYS=$2; else PHYS=elas-iso; fi
if [ -n "$3" ]; then CSTR=$3; else CSTR=gcc; fi
if [ -n "$4" ]; then MEM=$(( $4 * 1000000000 ));
  else MEM=`free -b  | grep Mem | awk '{print $7}'`; fi
if [ -n "$5" ]; then CPUMODEL=$5; else CPUMODEL=`./cpumodel.sh`; fi
if [ -n "$6" ]; then CPUCOUNT=$6; else CPUCOUNT=`./cpucount.sh`; fi
#
EXEDIR="/u/dwagner5/femera-mini-develop"
PERFDIR=$EXEDIR/"perf"
#
GMSH2FMR=$EXEDIR/"gmsh2fmr-"$CPUMODEL"-"$CSTR
EXEFMR=$EXEDIR/"femerq-"$CPUMODEL"-"$CSTR
#
C=$CPUCOUNT; N=$C; RTOL=1e-80;
TARGET_TEST_S=30;# Try for S sec/run
REPEAT_TEST_N=10;# Repeat each test N times
ITERS_MIN=10;
#
if [ -d "/hpnobackup1/dwagner5/femera-test/cube" ]; then
  MESHDIR=/hpnobackup1/dwagner5/femera-test/cube
else
  MESHDIR=$EXEDIR/"cube"
fi
echo "Mesh Directory: "$MESHDIR"/"
#
if [[ `hostname` == k2* ]]; then #FIXME Silly little hack
  if [ "$P" -eq 1 ]; then
    MEM=16000000000
  else
    MEM=23000000000
  fi
  module add gnuplot_5.0.5
fi
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
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=$CPUCOUNT
#
for P in $PLIST; do
  case $P in
  1) DOF_PER_ELEM=" 1 / 2 "; BYTE_PER_DOF=270; PSTR=tet4;  ;;
  2) DOF_PER_ELEM=4;   BYTE_PER_DOF=100; PSTR=tet10; ;;
  3) DOF_PER_ELEM=14;  BYTE_PER_DOF=80; PSTR=tet20; ;;
  # Mem estimate is with 40 parts. Add about 5% for optimal partitioning.
  esac
  #
  PROFILE=$PERFDIR/"profile-basic-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".pro"
  #LOGFILE=$PERFDIR/"uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".log"
  CSVFILE=$PERFDIR/"profile-basic-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  CSVSMALL=$PERFDIR/"small-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  CSVPROFILE=$PERFDIR/"profile-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  #
  # if [ -f $PROFILE ]; then
  #   NODE_MAX=`grep -m1 -i nodes $PROFILE | awk '{print $1}'`
  #   UDOF_MAX=$(( $NODE_MAX * 3 ))
  #   ELEM_MAX=`grep -m1 -i elements $PROFILE | awk '{print $1}'`
  # else
    #FIXME Partitioner uses too much RAM.
    UDOF_MAX=$(( $MEM / $BYTE_PER_DOF ))
    NODE_MAX=$(( $UDOF_MAX / 3))
    ELEM_MAX=$(( $UDOF_MAX / $DOF_PER_ELEM ))
  # fi
  MDOF_MAX=$(( $UDOF_MAX / 1000000 ))
  echo Largest Test Model: $ELEM_MAX $PSTR, $NODE_MAX Nodes, $MDOF_MAX MDOF
  #
  case $P in
  1)
    HSEQ="5 6 10 17 23 30 38 52 65 84 113 141 183 246" # 310"
    # too big to part in 90 GB:  421 531 669 / 250000000 500000000 1000000000
    H_MD=52; H_MD_DOF="500 kDOF"
    H_LG=113; H_LG_DOF="5 MDOF"
    H_XL=246; H_XL_DOF="50 MDOF"
    ;;
  2)
    HSEQ="2 3 4 7 8 11";
    HSEQ=$HSEQ" 15 19 26 33 42 57 71 90 121 157";
    # HSEQ=$HSEQ" 17 22 29 38 48 63 82 103 135";
    HSEQ=$HSEQ" 195 265 338";
    H_MD=33; H_MD_DOF="1 MDOF"
    H_LG=71; H_LG_DOF="10 MDOF"
    H_XL=157; H_XL_DOF="100 MDOF"
    ;;
  3)
    HSEQ="1 2 3 4 6   8 10 13 17 23 28 39 48 61 80 100" # 138 174 220"
    HSEQ=$HSEQ" 133 175 222";
    H_MD=23; H_MD_DOF="1 MDOF"
    H_LG=48; H_LG_DOF="10 MDOF"
    H_XL=100; H_XL_DOF="100 MDOF"
    ;;
  esac
  #
  if [ ! -f $PROFILE ]; then
    # First, get a rough idea of DOF/sec to estimate test time
    if [ ! -f $CSVFILE ]; then
      C=$CPUCOUNT
      H=$H_LG
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
      echo Estimating performance at $H_LG_DOF...
      $PERFDIR/mesh-part.sh $H $P $N $C "$PHYS" "$MESHDIR"
      echo Running $ITERS_MIN iterations of $MESHNAME...
      $EXEFMR -v1 -c$C -i$ITERS_MIN -r$RTOL -p $MESH >> $CSVFILE
    fi
  fi
  if [ -f $CSVFILE ]; then
    echo "Femera Performance Profile" > $PROFILE
  #  echo "Writing maximum problem size estimate: "$PROFILE"..." >> $LOGFILE
    echo >> $PROFILE
    echo "        Maximum Elastic Model Size Estimate" >> $PROFILE
    echo "  ------------------------------------------------" >> $PROFILE
    printf "     %9i : Maximum "$PSTR" elements\n" $ELEM_MAX >> $PROFILE
    printf "     %9i : Maximum nodes\n" $NODE_MAX >> $PROFILE
    printf "     %9i : Maximum MDOF\n" $MDOF_MAX >> $PROFILE
    #
    # echo "femerq-"$CPUMODEL"-"$CSTR >> $PROFILE
    # grep -m1 -i "model name" /proc/cpuinfo >> $PROFILE
    #
    MEM_GB="`free -g  | grep Mem | awk '{print $2}'`"
    printf "     %9i : GB memory\n" $MEM_GB >> $PROFILE
    #
    MDOFS=`head -n1 $CSVFILE | awk -F, '{ print $13/1e6 }'`
    NELEM=`head -n1 $CSVFILE | awk -F, '{ print $1 }'`
    NNODE=`head -n1 $CSVFILE | awk -F, '{ print $2 }'`
    NUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3 }'`
    MUDOF=`head -n1 $CSVFILE | awk -F, '{ print $3/1e6 }'`
    NPART=`head -n1 $CSVFILE | awk -F, '{ print $4 }'`
    ITERS=`head -n1 $CSVFILE | awk -F, '{ print $5 }'`
    NCPUS=`head -n1 $CSVFILE | awk -F, '{ print $9 }'`
  #  echo "Writing initial performance estimate: "$PROFILE"..." >> $LOGFILE
    echo >> $PROFILE
    echo "     Initial Elastic Model Performance Estimate" >> $PROFILE
    echo "  ------------------------------------------------" >> $PROFILE
    printf "        %6.1f : Initial test performance [MDOF/s]\n" $MDOFS >> $PROFILE
    printf "        %6.1f : Initial test system size [MDOF]\n" $MUDOF >> $PROFILE
    printf "%12i   : Initial model nodes\n" $NNODE >> $PROFILE
    printf "%12i   : Initial "$PSTR" elements\n" $NELEM >> $PROFILE
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
  #  echo "Writing basic profile test parameters: "$PROFILE"..." >> $LOGFILE
    echo >> $PROFILE
    echo "     Basic Performance Profile Test Parameters" >> $PROFILE
    echo "  ------------------------------------------------" >> $PROFILE
    printf "%6i     : Partitions = Threads = Physical Cores\n" $CPUCOUNT >> $PROFILE
    printf "%6i     : Basic test repeats\n" $REPEAT_TEST_N >> $PROFILE
    printf "  %6.1f   : Basic test solve time [sec]\n" $TARGET_TEST_S>>$PROFILE
    printf "%6i     : Basic minimum iterations\n" $ITERS_MIN >> $PROFILE
    printf "     %5.0e : Basic relative residual tolerance\n" $RTOL >> $PROFILE
    #
    if false; then
      echo Removing old partitioned meshes...
      for PP in $(seq 1 3 ); do
      for H in $HSEQ; do
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
    #CSVLINES=`wc -l < $CSVFILE`
    #BASIC_TEST_N=$(( $TRY_COUNT * $REPEAT_TEST_N + 1 ))
    #if [ "$CSVLINES" -lt "$BASIC_TEST_N" ]; then
      echo Running basic profile tests...
      C=$CPUCOUNT
      for H in $HSEQ; do
        MESHNAME="uhxt"$H"p"$P"n"
        MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
        NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
        NDOF=$(( $NNODE * 3 ))
        NDOF90=$(( $NDOF * 9 / 10 ))
        echo $MESHNAME has $NDOF DOF.
        MESHNAME="uhxt"$H"p"$P"n"$N
        MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
        if [ $NDOF -lt $UDOF_MAX ]; then
          $PERFDIR/mesh-part.sh $H $P $N $C "$PHYS" "$MESHDIR"
          TESTS_DONE=`grep -c ",$NNODE,$NDOF," $CSVFILE`
          if [ $TESTS_DONE -lt $REPEAT_TEST_N ]; then
            ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
            if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
            if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
            echo Warming up...
              $EXEFMR -v1 -c$C -i$ITERS_MIN -r$RTOL -p $MESH # > /dev/null
            echo "Running "$ITERS" iterations of "$MESHNAME" ("$NDOF" DOF),"\
              $REPEAT_TEST_N" times..."
            for I in $(seq 1 $REPEAT_TEST_N ); do
              $EXEFMR -v1 -c$C -i$ITERS -r$RTOL -p $MESH >> $CSVFILE
            done
          fi
        fi
      done
    #fi
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
  #    echo "Plotting basic profile data: "$CSVFILE"..." >> $LOGFILE
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
      plot '"$CSVFILE"'\
      using 3:(\$4 != \$9 ? 1/0:\$13/1e6)\
      with points pointtype 0\
      title '"$CPUCOUNT" Partitions';\
      "\
      | tee -a $PROFILE ;#| grep --no-group-separator -C25 --color=always '\*'
    else
      echo >> $PROFILE
    fi
  #  echo "Writing basic profile results: "$PROFILE"..." >> $LOGFILE
    echo "        Basic Performance Profile Test Results" >> $PROFILE
    echo "  --------------------------------------------------" >> $PROFILE
    printf "        %6.1f : Basic performance maximum [MDOF/s]\n" $MAX_MDOFS >> $PROFILE
    printf "%12i   : Basic system size [DOF]\n" $MAX_SIZE >> $PROFILE
    printf "%12i   : Basic model nodes\n" $MAX_NODES >> $PROFILE
    printf "%12i   : Basic "$PSTR" Elements\n" $MAX_ELEMS >> $PROFILE
    #
    # Find the max. performing model
    for I in $HSEQ; do
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P/$MESHNAME
      CHECK_NNODE=`grep -m1 -A1 -i node $MESH".msh" | tail -n1`
      if [ $CHECK_NNODE -eq $MAX_NODES ]; then
        MED_UDOF=$(( $CHECK_NNODE * 3 ));
        MED_MESHNAME=$MESHNAME
        MED_MESH=$MESH
        MED_H=$H
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
  #  echo "Writing medium model partitioning test parameters: "$PROFILE"..." >> $LOGFILE
    echo >> $PROFILE
    echo "  Medium Model Partitioning Test Parameters" >> $PROFILE
    echo "  -----------------------------------------" >> $PROFILE
    printf " %9i   : Medium test model size [DOF]\n" $MED_NUDOF >> $PROFILE
    printf " %9i   : Medium test model nodes\n" $MED_NNODE >> $PROFILE
    printf " %9i   : Medium test "$PSTR" elements\n" $MED_NELEM >> $PROFILE
    printf " %9i   : Medium test iterations\n" $MED_ITERS >> $PROFILE
    printf " %9i   : Medium test repeats\n" $REPEAT_TEST_N >> $PROFILE
    #
  fi
done;# P loop