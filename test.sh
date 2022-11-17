#! /bin/sh

######################################################################

black=0; red=1; green=2; yellow=3; blue=4; pink=5; cyan=6; white=7;
bold=$(tput bold); normal=$(tput sgr0);

cecho () {
  local _color=$1; shift
  echo -e "$(tput setaf $_color)$@$(tput sgr0)"
}

bold() {
  local _color=$1;
  echo -e ${bold}$(cecho $_color $2)${normal}
}

######################################################################

runtest() {
  local _testname=$1
  # https://stackoverflow.com/questions/34964332
  $(timeout 0.5 ./examples/repl $_testname &> /dev/null)
}

runtest_native() {
  local _testname=$1
  # https://stackoverflow.com/questions/34964332
  $(timeout 0.5 ./examples/repl $_testname --native 2> /dev/null \
    | qbe | as -o out && gcc out)
  $(timeout 0.5 ./a.out &> /dev/null)
}

collect_debug_artifacts() {
  local _testname=$(basename -s .sb $1)
  local _dirname="artifacts/$_testname"
  bold $blue "Debug artifacts in '$(bold $white $_dirname)'\n"
  mkdir -p "$_dirname/dots"
  (
    local _base=$(pwd)
    cd $_dirname
    $(timeout 0.5 $_base/examples/repl $_base/$1 --debug &> log)
  )
  cp graph.sh $_dirname
}

######################################################################

report_test() {
  local f=$1
  local name=$(cecho $white "[!] Running test $(basename $f)")
  printf '%-60s ... ' "$name"

  if runtest_native $f; then
    printf $(bold $green "PASS!") && echo
  else
    printf $(bold $red "FAIL!") && echo
    # collect_debug_artifacts $f
  fi
}

######################################################################

tests=$(find . -regextype "egrep" -iregex ".*-test-.*sb" | sort)

for f in $tests; do
  report_test $f
done

bold $green "-----------------------------------------------------------"
bold $green "All tests passed! (ideally)"

######################################################################
