echo 'setting C2 environment'
export C2_LIBDIR=$PWD/../c2_libs
export C2_PLUGINDIR=$HOME/c2c/plugins
export CC=clang
export CXX=clang++
complete -W '-a -A -b -c -C -d -f -h -m -q -Q -r -s -S -t -T -v --check --create --fast --help --help-recipe --showlibs --targets --test' c2c
