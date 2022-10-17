#! /bin/sh
cecho () {
  local _color=$1; shift
  echo -e "$(tput setaf $_color)$@$(tput sgr0)"
}

bold=$(tput bold); normal=$(tput sgr0);

bold() {
  local _color=$1; 
  echo -e ${bold}$(cecho $_color $2)${normal}
}

# you can also define some variables
black=0; red=1; green=2; yellow=3; blue=4; pink=5; cyan=6; white=7;

# usage
cecho $green "success!"
cecho $cyan "[INFO]:"
cecho $green $(bold $blue "Tello")

# err wrapping function
err () {
  cecho 1 "$@" >&2;
}

err "fail!"

tests=$(find . -regextype "egrep" \
               -iregex ".*-test-.*sb")


for f in $tests; do
  echo $f;
  # ./repl $f
done

bold $green "--------------------------------------------------------"
bold $green "[!] All tests passed!"

# if ./somecommand | grep -q 'string'; then
#  echo "matched"
# fi
