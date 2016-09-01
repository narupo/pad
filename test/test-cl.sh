#!/bin/bash
valgrind ./test-cl parsestr "123 \"223 323\" &> abebe '|' '123 \'223'  123\"223\"323 \"223\n323\t\"  '423\n523\t' -a -abc value -a=ABC -b=\"ABC DEF\" --A --ABC VALUE | --eq=EQ --EQ=\"EQ \\n \\\"EQ\" > /tmp/null"
if [ "$?" -ne 0 ]; then
	echo "failed."
	exit 1
fi
