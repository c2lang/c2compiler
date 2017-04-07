#!/bin/bash

#check if script is sourced or in a sub-shell
[[ "$_" != "$0" ]] && EXECUTION_MANNER=sourced || EXECUTION_MANNER=subshell

OPTIND=1 #reset getopts
C2_PATH=$HOME/llvm-c2/bin
PERMANENT=false

while getopts "pd:h" opt; do
    case "$opt" in
    p) PERMANENT=true
       ;;
    d) C2_PATH=$OPTARG
       ;;
    h) echo "Usage: env.sh [-d DIR] [--permanent]"
        if [ "$EXECUTION_MANNER" = sourced ] ; then
          return
        else
          echo "the script should be ran with the \"source\" command"
          exit
        fi
       ;;
esac
shift
done

if [ "$EXECUTION_MANNER" = sourced ] ; then
  echo 'setting C2 environment'

  if [ "$PERMANENT" = true ] ; then
    echo "appending to $HOME/.bashrc"
    echo "alias c2c=$PWD/build/c2c/c2c" >> $HOME/.bashrc
    echo "export C2_LIBDIR=$PWD/c2libs" >> $HOME/.bashrc
    echo "export PATH=$PATH:$C2_PATH" >> $HOME/.bashrc
  fi

  alias c2c=$PWD/build/c2c/c2c
  export C2_LIBDIR=$PWD/c2libs
  export PATH=$PATH:$C2_PATH
else
  echo "error: the script should be ran with the \"source\" command or dot"
fi
