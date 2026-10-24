#echo show_warnings.sh $*
grep -H '[[]-W' $* | sed -E 's/build.log://'
exit 0
