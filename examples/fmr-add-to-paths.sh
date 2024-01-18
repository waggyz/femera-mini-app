#!/bin/bash
#
# PREFIX=/home/anon/local
# source $PREFIX/fmrpaths $PREFIX
#
PREFIX="$1"
if [ -z "$2" ]; then
  CPUMODEL=`fmrmodel`
else
  CPUMODEL="$2"
fi
SUB="$PATH/bin";
if [[ ! "$PATH" == *"$SUB"* ]]; then
  PATH=$SUB:$PATH
fi
SUB="$PATH/$CPUMODEL/bin";
if [[ ! "$PATH" == *"$SUB"* ]]; then
  PATH=$SUB:$PATH
fi
SUB="$PREFIX/lib64";
if [[ ! "$PATH" == *"$SUB"* ]]; then
  export LD_LIBRARY_PATH=$SUB:$PATH
fi
SUB="$PREFIX/lib";
if [[ ! "$PATH" == *"$SUB"* ]]; then
  export LD_LIBRARY_PATH=$SUB:$PATH
fi
SUB="$PREFIX/$CPUMODEL/lib";
if [[ ! "$PATH" == *"$SUB"* ]]; then
  export LD_LIBRARY_PATH=$SUB:$PATH
fi
unset SUB
unset PREFIX
unset CPUMODEL
