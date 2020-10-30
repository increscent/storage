#!/bin/bash

make
printf "CREATE TABLE table1 (\
        column1 INTEGER PRIMARY KEY, \
        column2 TEXT default 'hello' \
    ); \n0\n" \
    | ./sqlite db.sqlite
