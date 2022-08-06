# !/bin/bash

make clean-db

for ((i=0;i<7;i++))
do
    ./target/generate
done
