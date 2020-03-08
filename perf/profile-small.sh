#!/bin/bash
if [ -n "$1" ]; then PLIST=$1; else PLIST="2 3 1" ; fi
if [ -n "$2" ]; then PHYS=$2; else PHYS=elas-iso; fi
if [ -n "$3" ]; then CSTR=$3; else CSTR=gcc; fi
if [ -n "$4" ]; then MEM=$(( $4 * 1000000000 ));
  else MEM=`free -b  | grep Mem | awk '{print $7}'`; fi
if [ -n "$5" ]; then CPUMODEL=$5; else CPUMODEL=`./cpumodel.sh`; fi
if [ -n "$6" ]; then CPUCOUNT=$6; else CPUCOUNT=`./cpucount.sh`; fi
#
if [ -d "/u/dwagner5" ]; then
  EXEDIR="/u/dwagner5/femera-mini-develop"
else
  EXEDIR=`pwd`
fi
PERFDIR=$EXEDIR/"perf"
#
GMSH2FMR=$EXEDIR/"gmsh2fmr-"$CPUMODEL"-"$CSTR
EXEFMR=$EXEDIR/"femera-mmq-"$CPUMODEL"-"$CSTR
#
C=$CPUCOUNT; N=$C; RTOL=1e-80;
TARGET_TEST_S=30;# Try for S sec/run
REPEAT_TEST_N=10;# Repeat each test N times
ITERS_MIN=100;
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
export OMP_PROC_BIND=spread,close
export OMP_NUM_THREADS=$CPUCOUNT
export OMP_NESTED=true
export OMP_MAX_ACTIVE_LEVELS=2
#
for P in $PLIST; do
  case $P in
  1) DOF_PER_ELEM=" 1 / 2 "; BYTE_PER_DOF=270; PSTR=tet4;  ;;
  2) DOF_PER_ELEM=4;   BYTE_PER_DOF=100; PSTR=tet10; ;;
  3) DOF_PER_ELEM=14;  BYTE_PER_DOF=80; PSTR=tet20; ;;
  # Mem estimate is with 40 parts. Add about 5% for optimal partitioning.
  esac
  #
  PROFILE=$PERFDIR/"profile-small-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".pro"
  #LOGFILE=$PERFDIR/"uhxt-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".log"
  CSVFILE=$PERFDIR/"profile-small-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  #CSVSMALL=$PERFDIR/"small-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  CSVPROFILE=$PERFDIR/"profile-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  #
  if [ 1 -eq 1 ];then
    rm "$PROFILE"
    rm "$CSVFILE"
  fi
  #
  case $P in
  1)
    HSEQ="5 6 10 17 23 30 38 52 65 84"
    H_MD=52; H_MD_DOF="500 kDOF"; MD_DOF=500000
    ;;
  2)
    HSEQ="2 3 5 7 8 11 15 19 22 26 33 42";
    # HSEQ=$HSEQ" 17 22 29 38";
    H_MD=33; H_MD_DOF="1 MDOF"; MD_DOF=1000000
    ;;
  3)
    HSEQ="1 2 3 4 6 7 8 10 13 17 23 28"
    H_MD=23; H_MD_DOF="1 MDOF"; MD_DOF=1000000
    ;;
  esac
  #
  if [ ! -e "$PROFILE" ]; then
    # First, get a rough idea of DOF/sec to estimate test time
    if [ ! -e "$CSVFILE" ]; then
      C=$CPUCOUNT
      H=$H_MD
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P"/""$MESHNAME"
      echo Estimating performance at $H_MD_DOF...
      "$PERFDIR"/"mesh-part.sh" $H $P $N $C "$PHYS" "$MESHDIR"
      echo Running $ITERS_MIN iterations of $MESHNAME...
      "$EXEFMR" -v1 -c$C -i$ITERS_MIN -r$RTOL -p "$MESH" >> "$CSVFILE"
    fi
  fi
  if [ -f "$CSVFILE" ]; then
    echo "Femera Performance Profile" > "$PROFILE"
    #
    MEM_GB="`free -g  | grep Mem | awk '{print $2}'`"
    printf "     %9i : GB memory\n" $MEM_GB >> "$PROFILE"
    #
    MDOFS=`head -n1 "$CSVFILE" | awk -F, '{ print $13/1e6 }'`
    NELEM=`head -n1 "$CSVFILE" | awk -F, '{ print $1 }'`
    NNODE=`head -n1 "$CSVFILE" | awk -F, '{ print $2 }'`
    NUDOF=`head -n1 "$CSVFILE" | awk -F, '{ print $3 }'`
    MUDOF=`head -n1 "$CSVFILE" | awk -F, '{ print $3/1e6 }'`
    NPART=`head -n1 "$CSVFILE" | awk -F, '{ print $4 }'`
    ITERS=`head -n1 "$CSVFILE" | awk -F, '{ print $5 }'`
    NCPUS=`head -n1 "$CSVFILE" | awk -F, '{ print $9 }'`
  #  echo "Writing initial performance estimate: "$PROFILE"..." >> $LOGFILE
    echo >> "$PROFILE"
    echo "     Small Elastic Model Performance Estimate" >> "$PROFILE"
    echo "  ------------------------------------------------" >> "$PROFILE"
    printf "        %6.1f : Small test performance [MDOF/s]\n" $MDOFS >> "$PROFILE"
    printf "        %6.1f : Small test system size [MDOF]\n" $MUDOF >> "$PROFILE"
    printf "%12i   : Small model nodes\n" $NNODE >> "$PROFILE"
    printf "%12i   : Small "$PSTR" elements\n" $NELEM >> "$PROFILE"
    printf "%12i   : Small test partitions\n" $NPART >> "$PROFILE"
    printf "%12i   : Small test threads\n" $NCPUS >> "$PROFILE"
    printf "%12i   : Small test iterations\n" $ITERS >> "$PROFILE"
    #echo "Mesh            : " FIXME Put initial mesh filename here
  fi
  if [ -f "$CSVFILE" ]; then
    INIT_KUDOF=`head -n1 "$CSVFILE" | awk -F, '{ print int($3/1e3) }'`
    INIT_MDOFS=`head -n1 "$CSVFILE" | awk -F, '{ print int(($13+5e5)/1e6) }'`
    INIT_DOFS=`head -n1 "$CSVFILE" | awk -F, '{ print int($13+0.5) }'`
    echo "Small performance estimate: "$INIT_MDOFS" MDOF/s at "$INIT_KUDOF" KDOF"
    #
    echo >> "$PROFILE"
    echo "     Small Performance Profile Test Parameters" >> "$PROFILE"
    echo "  ------------------------------------------------" >> "$PROFILE"
    printf "%6i     : Small partitions\n" 1 >> "$PROFILE"
    printf "%6i     : Small test repeats\n" $REPEAT_TEST_N >> "$PROFILE"
    printf "  %6.1f   : Small test solve time [sec]\n" $TARGET_TEST_S>>"$PROFILE"
    printf "%6i     : Small minimum iterations\n" $ITERS_MIN >> "$PROFILE"
    printf "     %5.0e : Small relative residual tolerance\n" $RTOL >> "$PROFILE"
    #
    echo Running small profile tests...
    # M: Total number of models in one run
    # X: Number of concurrent models
    # C: Number of threads/model
    # C * X = CPUCOUNT
    # S: Number of sequential models run by each core
    TOTAL_DOF=$(( $INIT_DOFS * $TARGET_TEST_S ))
    echo Target $(( $TOTAL_DOF / 1000000 )) MDOF processed each test.
    for H in $HSEQ; do
      MESHNAME="uhxt"$H"p"$P"n"
      MESH=$MESHDIR"/uhxt"$H"p"$P"/""$MESHNAME"
      "$PERFDIR"/"mesh-part.sh" $H $P $C $CPUCOUNT "$PHYS" "$MESHDIR"
      if [ -e "$MESH"".msh" ]; then
        NNODE=`grep -m1 -A1 -i node "$MESH"".msh" | tail -n1`
        NDOF=$(( $NNODE * 3 ))
        NDOF90=$(( $NDOF * 9 / 10 ))
        echo $MESHNAME has $NDOF DOF.
        for X in $(seq 2 64); do
          C=$(( $CPUCOUNT / $X ))
          if [ $(( $C * $X )) -eq $CPUCOUNT ]; then
            CONCURRENT_DOF=$(( $X * $NDOF ))
            if [ $(( $MD_DOF / 101 )) -lt $CONCURRENT_DOF ];then
            if [ $CONCURRENT_DOF -lt $(( $MD_DOF * 11 )) ];then
              MESHNAME="uhxt"$H"p"$P"n"$C
              MESH=$MESHDIR"/uhxt"$H"p"$P"/""$MESHNAME"
              "$PERFDIR"/"mesh-part.sh" $H $P $C $CPUCOUNT "$PHYS" "$MESHDIR"
              #
              ITERS=$(( $INIT_DOFS * $TARGET_TEST_S / $NDOF ))
              if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
              if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
              #
              M=$(( $TOTAL_DOF / $NDOF / $ITERS ))
              if [ $M -le $X ];then
                M=$X;
                ITERS=$(( $TOTAL_DOF / $NDOF / $M ))
                if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
                if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
              fi
              # MODELS_PER_TEST=$(( $C * $X ))
              # TOTAL_MODELS=$(( $MODELS_PER_TEST * $REPEAT_TEST_N ))
              TOTAL_MODELS=$(( $M * $REPEAT_TEST_N ))
              TESTS_DONE=`grep -c ",$NNODE,$NDOF,$C," $CSVFILE`
              echo Found $TESTS_DONE of $TOTAL_MODELS models solved...
              if [ $TESTS_DONE -lt $(( $TOTAL_MODELS * 120 / 100 )) ];then
                S=$(( $M / $X ))
                echo Warming up...
                  "$EXEFMR" -v1 -c$C -m$(( $C * $X )) -n$X -i$ITERS_MIN -r$RTOL -p "$MESH" > /dev/null
                echo "Running "$REPEAT_TEST_N" repeats of "$S"x"$X" concurrent "$NDOF" DOF models..."
                START=`date +%s.%N`
                for I in $(seq 1 $REPEAT_TEST_N ); do
                  "$EXEFMR" -v1 -c$C -m$M -n$X -i$ITERS -r$RTOL -p "$MESH" >> "$CSVFILE"
                done
                STOP=`date +%s.%N`
                TIME_SEC=`printf "%f-%f\n" $STOP $START | bc`
              fi
              #
              MDOF_TOTAL=`awk -F, -v n=$NNODE -v c=$C -v dof=0\
              '($2==n)&&($9==c){dof=dof+$3*$5;}\
                END{print dof/1000000;}' "$CSVFILE"`
              TOTAL_MDOFS=`printf "%f/%f\n" $MDOF_TOTAL $TIME_SEC | bc`
              echo "Overall: "$TOTAL_MDOFS" MDOF/s ("$MDOF_TOTAL\
              "MDOF in "$TIME_SEC" sec)"
              #
              SOLVE_MDOFS=`awk -F, -v nnode=$NNODE -v c=$C -v nrun=0 -v mdofs=0 -v x=$X\
              '($2==nnode)&&($9==c){nrun=nrun+1;mdofs=mdofs+$13;}\
                END{print mdofs/nrun/1000000*x;}' "$CSVFILE"`
              echo " Solver: "$SOLVE_MDOFS" MDOF/s at "$X"x "$NDOF" DOF"\
                "models ("$(( $X * $NDOF ))" DOF concurrent)..."
            fi
            fi
          fi
        done;# X loop
      fi
    done;# H loop
  fi
done;# P loop