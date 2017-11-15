#!/usr/bin/env bash

g++ main.cpp -std=c++11 -O3 -o sandpiles
g++ main.cpp -std=c++11 -DSAND_8 -O3 -o sandpiles8
