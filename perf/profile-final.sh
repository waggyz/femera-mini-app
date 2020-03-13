#!/bin/bash
if [ -n "$1" ]; then PLIST=$1; else PLIST="2 3 1" ; fi
if [ -n "$2" ]; then PHYS=$2; else PHYS=elas-iso; fi
if [ -n "$3" ]; then CSTR=$3; else CSTR=gcc; fi
if [ -n "$4" ]; then MEM=$(( $4 * 1000000000 ));
  else MEM=`free -b  | grep Mem | awk '{print $7}'`; fi
if [ -n "$5" ]; then CPUMODEL=$5; else CPUMODEL=`./cpumodel.sh`; fi
if [ -n "$6" ]; then CPUCOUNT=$6; else CPUCOUNT=`./cpucount.sh`; fi
#
if [ -d "/u/dwagner5/femera-mini-develop" ]; then
  EXEDIR="/u/dwagner5/femera-mini-develop"
else
  EXEDIR=`pwd`
fi
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
  1) DOF_PER_ELEM=" 1 / 2 "; BYTE_PER_DOF=270; PSTR=tet4; ;;
  2) DOF_PER_ELEM=4;   BYTE_PER_DOF=100; PSTR=tet10; ;;
  3) DOF_PER_ELEM=14;  BYTE_PER_DOF=80; PSTR=tet20; ;;
  # Mem estimate is with 40 parts. Add about 5% for optimal partitioning.
  esac
  UDOF_MAX=$(( $MEM / $BYTE_PER_DOF ))
  NODE_MAX=$(( $UDOF_MAX / 3))
  ELEM_MAX=$(( $UDOF_MAX / $DOF_PER_ELEM ))
  MDOF_MAX=$(( $UDOF_MAX / 1000000 ))
  echo Largest Test Model: $ELEM_MAX $PSTR, $NODE_MAX Nodes, $MDOF_MAX MDOF
  case $P in
  1)
    HSEQ="5 6 10 17 23 30 38 52 65 84 113 141 183 246" # 310"
    # too big to part in 90 GB:  421 531 669 / 250000000 500000000 1000000000
    H_MD=52; H_MD_DOF="500 kDOF"
    H_LG=113; H_LG_DOF="5 MDOF"
    H_XL=246; H_XL_DOF="50 MDOF"
    case $CPUMODEL in
    6148)    MINSIZE_MD=480819; MINPART=40; ELEM_PER_PART=20000; DOF_PER_PART=10000; ;;
    6148b)   MINSIZE_MD=480819; MINPART=80; ELEM_PER_PART=20000; DOF_PER_PART=10000; ;;
    E5-2640) ;;
    E5-2650) ;;
    E5-2670) MINSIZE_MD=141456; MINPART=64; ELEM_PER_PART=109000; DOF_PER_PART=52525 ;;
    X5675) ;;
    *) MINSIZE_MD=480000; MINPART=240; ELEM_PER_PART=4584;  DOF_PER_PART=2000; ;;
    esac
    ;;
  2)
    HSEQ="2 3 5 7 8 11";
    HSEQ=$HSEQ" 15 19 26 33 42 57 71 90 121 157";
    # HSEQ=$HSEQ" 17 22 29 38 48 63 82 103 135";
    HSEQ=$HSEQ" 195 265 338";
    H_MD=33; H_MD_DOF="1 MDOF"
    H_LG=71; H_LG_DOF="10 MDOF"
    H_XL=157; H_XL_DOF="100 MDOF"
    case $CPUMODEL in
    # 6148)    MINSIZE_MD=679635; MINPART=240; ELEM_PER_PART=8900; DOF_PER_PART=35767; ;;
    # 6148b)   MINSIZE_MD=679635; MINPART=160; ELEM_PER_PART=7800; DOF_PER_PART=31300; ;;
    6148)    MINSIZE_MD=679635; MINPART=240; ELEM_PER_PART=10750; DOF_PER_PART=43000; ;;
    6148b)   MINSIZE_MD=526956; MINPART=240; ELEM_PER_PART=9160;  DOF_PER_PART=36640; ;;
    E5-2640) ;;
    E5-2650) ;;
    E5-2670) MINSIZE_MD=324354; MINPART=160; ELEM_PER_PART=4584;  DOF_PER_PART=18336; ;;
    X5675) ;;
    *) MINSIZE_MD=480000; MINPART=240; ELEM_PER_PART=4584;  DOF_PER_PART=2000; ;;
    esac
    ;;
  3)
    HSEQ="1 2 3 4 6   8 10 13 17 23 28 39 48 61 80 100" # 138 174 220"
    HSEQ=$HSEQ" 133 175";
    #HSEQ=$HSEQ" 222";# 222:1GDOF too big to part in 90 GB
    H_MD=23; H_MD_DOF="1 MDOF"
    H_LG=48; H_LG_DOF="10 MDOF"
    H_XL=100; H_XL_DOF="100 MDOF"
    case $CPUMODEL in
    6148)    MINSIZE_MD=xxx; MINPART=xxx; ELEM_PER_PART=100; DOF_PER_PART=1400 ;;
    6148b)   MINSIZE_MD=xxx; MINPART=xxx; ELEM_PER_PART=100; DOF_PER_PART=1400 ;;
    E5-2640) ;;
    E5-2650) ;;
    E5-2670) MINSIZE_MD=349863; MINPART=240; ELEM_PER_PART=1850; DOF_PER_PART=25164 ;;
    X5675) ;;
    *) MINSIZE_MD=480000; MINPART=240; ELEM_PER_PART=4584;  DOF_PER_PART=2000; ;;
    esac
    ;;
  esac
  #
  PROFILE=$PERFDIR/"profile-final-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".pro"
  CSVFILE=$PERFDIR/"profile-final-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  CSVBASIC=$PERFDIR/"profile-basic-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  CSVPROFILE=$PERFDIR/"profile-"$PSTR"-"$PHYS"-"$CPUMODEL"-"$CSTR".csv"
  #
  if [ ! -f "$CSVFILE" ]; then
    if [ -f "$CSVBASIC" ]; then
      head -n1 "$CSVBASIC" > "$CSVFILE"
    else
      # Get a rough idea of DOF/sec to estimate test time
      C=$CPUCOUNT
      H=$H_MD
      MESHNAME="uhxt"$H"p"$P"n"$N
      MESH=$MESHDIR"/uhxt"$H"p"$P"/"$MESHNAME
      echo Estimating performance at $H_MD_DOF...
      "$PERFDIR"/"mesh-part.sh" $H $P $N $C "$PHYS" "$MESHDIR"
      echo Running $ITERS_MIN iterations of $MESHNAME...
      "$EXEFMR" -v1 -c$C -i$ITERS_MIN -r$RTOL -p "$MESH" > "$CSVFILE"
    fi
  fi
  if [ -f "$CSVFILE" ]; then
    INIT_MUDOF=`head -n1 "$CSVFILE" | awk -F, '{ print int($3/1e6) }'`
    INIT_MDOFS=`head -n1 "$CSVFILE" | awk -F, '{ print int(($13+5e5)/1e6) }'`
    INIT_DOFS=`head -n1 "$CSVFILE" | awk -F, '{ print int($13+0.5) }'`
    echo "Initial performance estimate: "$INIT_MDOFS" MDOF/s at "$INIT_MUDOF" MDOF"
    echo Running final profile tests...
    C=$CPUCOUNT
    for H in $HSEQ; do
      MESHNAME="uhxt"$H"p"$P"n"
      MESH="$MESHDIR"/"uhxt"$H"p"$P"/"$MESHNAME
      "$PERFDIR"/"mesh-part.sh" $H $P $C $C "$PHYS" "$MESHDIR"
      if [ -f "$MESH"".msh" ]; then
        NNODE=`grep -m1 -A1 -i node "$MESH"".msh" | tail -n1`
        NDOF=$(( $NNODE * 3 ))
        NDOF90=$(( $NDOF * 9 / 10 ))
        if [ $NDOF -gt $MINSIZE_MD ]; then
        if [ $NDOF -lt $UDOF_MAX ]; then
          echo $MESHNAME has $NDOF DOF.
          N=$(( $MINPART + $(( $NDOF / $DOF_PER_PART )) ))
          N=$(( $(( $N / $C + $(( $C / 2 )) )) * $C ))
          #NOTE Round up to nearest multiple of C
          if [ $N -lt $MINPART ]; then N=$MINPART; fi
          if [ $C -lt 4 ]; then CC=4; else CC=$C; fi
          CSVSLICE="$PERFDIR"/"slice-"$CC".csv"
          if [ -f "$CSVSLICE" ];then
            SLICESTR=`awk -F, -v N=$N \
              'BEGIN{OFS=",";} ($1>=N){print$1,$2,$3,$4;exit}' "$CSVSLICE"`
            #echo $SLICESTR
            IFS=, read N SX SY SZ <<< "$SLICESTR"
            #echo Slice $N "(" $SX $SY $SZ ")"
          fi
          MESHNAME="uhxt"$H"p"$P"n"$N
          MESH="$MESHDIR"/"uhxt"$H"p"$P"/"$MESHNAME
           "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ "$PHYS" "$MESHDIR"
          TESTS_DONE=`grep -c ",$NNODE,$NDOF," "$CSVFILE"`
          if [ $TESTS_DONE -lt $REPEAT_TEST_N ]; then
            ITERS=`printf '%f*%f/%f\n' $TARGET_TEST_S $INIT_DOFS $NDOF | bc`
            if [ $ITERS -lt $ITERS_MIN ]; then ITERS=$ITERS_MIN; fi
            if [ $ITERS -gt $NDOF90 ]; then ITERS=$NDOF90; fi
            echo Warming up...
              "$EXEFMR" -v1 -c$C -i$ITERS_MIN -r$RTOL -p "$MESH" > /dev/null
            echo "Running "$ITERS" iterations of "$MESHNAME" ("$NDOF" DOF),"\
              $REPEAT_TEST_N" times..."
            for I in $(seq 1 $REPEAT_TEST_N ); do
              "$EXEFMR" -v1 -c$C -i$ITERS -r$RTOL -p "$MESH" >> "$CSVFILE"
            done
          fi
        fi
        fi
      fi
    done
    SIZE_PERF_MAX=`awk -F, -v c=$CPUCOUNT -v max=0\
      '($9==c)&&($13>max)&&($4==$9){max=$13;perf=int(($13+5e5)/1e6);size=$3}\
      END{print int((size+50)/100)*100,int(perf+0.5)}'\
      "$CSVFILE"`
    MAX_MDOFS=${SIZE_PERF_MAX##* }
    MAX_SIZE=${SIZE_PERF_MAX%% *}
    echo "Maximum final performance: "$MAX_MDOFS" MDOF/s"\
    at $MAX_SIZE" DOF, parts = cores = "$CPUCOUNT"."
    #
    NODE_ELEM_MAX=`awk -F, -v c=$CPUCOUNT -v max=0\
      '($9==c)&&($13>max)&&($4==$9){max=$13;nelem=$1;nnode=$2}\
      END{print nelem,nnode}'\
      "$CSVFILE"`
    MAX_ELEMS=${NODE_ELEM_MAX%% *}
    MAX_NODES=${NODE_ELEM_MAX##* }
    if [ ! -z "$HAS_GNUPLOT" ]; then
    #    echo "Plotting final profile data: "$CSVFILE"..." >> $LOGFILE
      gnuplot -e  "\
      set terminal dumb noenhanced size 79,25;\
      set datafile separator ',';\
      set tics scale 0,0;\
      set logscale x;\
      set xrange [1e3:1.05e9];\
      set yrange [0:];\
      set key inside top right;\
      set title 'Femera Final Elastic Performance Tests [MDOF/s]';\
      set xlabel 'System Size [DOF]';\
      set label at "$MAX_SIZE", "$MAX_MDOFS" \"* Max\";\
      plot '"$CSVFILE"'\
      using 3:(\$4 != \$9 ? 1/0:\$13/1e6)\
      with points pointtype 0\
      title '"$CPUCOUNT" Partitions';\
      " \
       | tee -a "$PROFILE" ;#| grep --no-group-separator -C25 --color=always '\*'
    else
      echo >> "$PROFILE"
    fi
    #  echo "Writing final profile results: "$PROFILE"..." >> $LOGFILE
    echo "        Final Performance Profile Test Results" >> "$PROFILE"
    echo "  --------------------------------------------------" >> "$PROFILE"
    printf "        %6.1f : Final performance maximum [MDOF/s]\n" $MAX_MDOFS >> "$PROFILE"
    printf "%12i   : Final system size [DOF]\n" $MAX_SIZE >> "$PROFILE"
    printf "%12i   : Final model nodes\n" $MAX_NODES >> "$PROFILE"
    printf "%12i   : Final "$PSTR" Elements\n" $MAX_ELEMS >> "$PROFILE"
    #
  fi
done;# P loop