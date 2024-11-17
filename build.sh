before_double_dash=()
after_double_dash=()
found_double_dash=false
for arg in "${@:2}"; do
    if [ "$arg" == "--" ]; then
        found_double_dash=true
    elif [ "$found_double_dash" == false ]; then
        before_double_dash+=("$arg")
    else
        after_double_dash+=("$arg")
    fi
done
`dirname $BASH_SOURCE`/build.py "$1" -r ${after_double_dash[@]} -- ${before_double_dash[@]}
