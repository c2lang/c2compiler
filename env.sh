if [ -f libs/libc/stdio.c2i ] ; then
   echo 'setting C2 development environment'
   export C2_LIBDIR=$PWD/libs
   export C2_PLUGINDIR=$PWD/output/plugins
else
   echo 'setting C2 installation environment'
   export C2_LIBDIR=~/c2_libs
   export C2_PLUGINDIR=~/c2_plugins
fi
export CC=clang
#complete -W '-a -A -b -c -C -d -f -h -m -q -Q -r -s -S -t -T -v --check --create --fast --help --help-recipe --showlibs --showplugins --targets --test' c2c
