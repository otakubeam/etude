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

runtest_native() {
  local _testname=$1
  # https://stackoverflow.com/questions/34964332
  $(timeout 0.5                          \
      ./etc $_testname    2> /dev/null   \
        | qbe             2> /dev/null   \
        | as -o out       2> /dev/null   \
        && gcc out        2> /dev/null)  \

  $(timeout 0.5 ./a.out &> /dev/null)
}

collect_debug_artifacts() {
  local _testname=$(basename $1)
  local _dirname="artifacts/$_testname"
  bold $blue "Debug artifacts in '$(bold $white $_dirname)'\n"

  (
    local _base=$(pwd)

    mkdir -p $_dirname
    cd $_dirname

    # Copy test file
    cp $_base/$1.et .

    $(timeout 0.5                         \
        $_base/etc $_base/$1    2>> log   \
          | qbe                 2>> log   \
          | as -o out           2>> log   \
          && gcc out            2>> log)  \

    $(timeout 0.5 ./a.out 1> output 2>> log)

  )
}

report_test() {
  local f=$1
  local name=$(cecho $white "[!] Running test $(basename $f)")
  printf '%-60s ... ' "$name"

  if runtest_native $f; then
    printf $(bold $green "PASS!") && echo
  else
    printf $(bold $red "FAIL!") && echo
    collect_debug_artifacts $f
  fi
}

######################################################################

tests=$(find . -regextype "egrep" -iregex ".*-test-.*.et" | sort)

for f in $tests; do
  report_test ${f%.et}
done

# test -e out a.out && \
    rm out a.out

bold $green "-----------------------------------------------------------"
bold $green "All tests passed! (ideally)"

######################################################################
