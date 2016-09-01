#!/bin/bash
for cmd in string-cmds.*
do
	cat $cmd | ./string
done
