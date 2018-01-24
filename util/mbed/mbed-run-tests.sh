#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $SCRIPT_DIR/colors

cd $SCRIPT_DIR
cd ../..
set -e

make robot-test-i2c
make robot-test-io-expander
make robot-test-piezo
make robot-test-fpga

msg="${BOLD}All firmware hw-test targets successfully built!${AOFF}${WHITE}${GREENBG} "
printf "$WHITE $GREENBG $PUTLN \n $PUTLN \n"
printf "%*s\n" $(((${#msg}+$(tput cols))/2)) "$msg"
printf "$PUTLN \n $PUTLN$R\n"
