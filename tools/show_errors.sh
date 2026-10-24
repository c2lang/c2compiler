#echo show_errors.sh $*
grep -H 'error:' $* | sed -E 's/build.log://'
grep -H '[[]-W' $* | sed -E 's/build.log://'
exit 0
