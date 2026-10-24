#echo show_diags.sh $*
grep -H -E '(error|warning):' $* | sed -E 's/build.log://'
exit 0
