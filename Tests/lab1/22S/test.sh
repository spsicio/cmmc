#!/bin/bash
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
RESET="\033[0m"
PARSER=../../../Code/parser

if [[ -z $PARSER ]]
then
    echo "Usage: ./test.sh paser_path"
    exit
fi

echo -e $YELLOW"2022S Test Set For Lab-1 Group-1.1"$RESET

function treeTest() # input file, expect file
{
    INPUT=$1
    EXPECT=$2
    TMP=/tmp/parser.output
    $PARSER $INPUT > $TMP
    RES=$(diff $TMP $EXPECT)
    if [[ -z $RES ]]
    then
        echo -e "Test ${INPUT##*/} : "$GREEN"PASS"$RESET
    else
        echo -e "Test ${INPUT##*/} : "$RED"FAILED"$RESET
        diff --side-by-side $TMP $EXPECT
    fi
}

function errorTest() # input file, expect file
{
    INPUT=$1
    EXPECT=$2
    TMP1=/tmp/parser.output
    $PARSER $INPUT > $TMP1
    TMP2=/tmp/parser.processed
    awk -F'[ :]'  '{print $1,$2,$3,$4,$5,$6}' $TMP1 > $TMP2
    RES=$(diff $TMP2 $EXPECT)
    if [[ -z $RES ]]
    then
        echo -e "Test ${INPUT##*/} : "$GREEN"PASS"$RESET
    else
        echo -e "Test ${INPUT##*/} : "$RED"FAILED"$RESET
        diff --side-by-side $TMP2 $EXPECT
    fi
}

TREE_TESTS=$(find inputs/tree -name *.cmm | sort)
ERROR_TESTS=$(find inputs/error -name *.cmm | sort)
HAS_ERROR_TESTS=$(find inputs/has_error -name *.cmm | sort)

echo -e $YELLOW"--- Syntax Tree Testcases ---"$RESET

for t in $TREE_TESTS
do
    e=${t##*/}
    e="expects/"${e%.*}".exp"
    # echo $t $e
    treeTest $t $e
done

echo -e $YELLOW"--- Error Reporting Testcases ---"$RESET

for t in $ERROR_TESTS
do
    e=${t##*/}
    e="expects/"${e%.*}".exp"
    # echo $t $e
    errorTest $t $e
done

echo -e $YELLOW"--- Error Detecting Testcases ---"$RESET

for t in $HAS_ERROR_TESTS
do
    TMP=/tmp/parser.output
    $PARSER $t > $TMP
    RES=$(cat $TMP | grep "Error")
    if [[ -z $RES ]]
    then
        echo -e "Test ${t##*/} : "$RED"FAILED"$RESET
        cat $TMP
    else
        echo -e "Test ${t##*/} : "$GREEN"PASS"$RESET
    fi
done

