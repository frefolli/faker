#!/bin/bash

# If runned without ARGS it defaults to "make contest-10m > output-contest-10m.txt"
# If runned without OUTPUT arg it defaults to "make $INPUT > output-$INPUT.txt"

INPUT=$1
OUTPUT=$2

if [ -z $INPUT ]; then
    INPUT=contest-10m
fi

if [ -z $OUTPUT ]; then
    OUTPUT=output-$INPUT.txt
fi

nohup make $INPUT > $OUTPUT &
