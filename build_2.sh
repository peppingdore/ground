before_double_dash=()
after_double_dash=()
found_double_dash=false
for arg in $@; do
    if [ "$arg" == "--" ]; then
        found_double_dash=true
    elif [ "$found_double_dash" == false ]; then
        before_double_dash+=("$arg")
    else
        after_double_dash+=("$arg")
    fi
done
`dirname $BASH_SOURCE`/build.py "$0" -r ${after_double_dash[@]} -- ${before_double_dash[@]}
exit

##if 0
	# source `dirname "$0"`/build_2.sh
##endif
