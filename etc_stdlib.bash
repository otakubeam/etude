script_path=$(readlink -e "$0")
script_dir=$(dirname "$script_path")

export ETUDE_STDLIB="$script_dir"/stdlib
"$script_dir"/etc "$@"
