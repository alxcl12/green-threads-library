#!/bin/bash

NR_OF_RUNS=$1
RESULTS_IMAGE=()
RESULTS_BLOCKS=()
RESULTS_FILES=()
TOTAL_GO=0

cd C_sources
for (( c=1; c<=$NR_OF_RUNS; c++ ))
do
   REZ_C=$(./image_filter)
   echo "$REZ_C";
   RESULTS_IMAGE+=($REZ_C)

   REZ_C=$(./make_blocks)
   echo "$REZ_C";
   RESULTS_BLOCKS+=($REZ_C)

   REZ_C=$(./copy_files)
   echo "$REZ_C";
   RESULTS_FILES+=($REZ_C)

   make clean
   make
done


sum_image=0
sum_blocks=0
sum_files=0
for i in "${RESULTS_IMAGE[@]}"
do
	sum_image=`expr $sum_image + $i`
done

for i in "${RESULTS_BLOCKS[@]}"
do
	sum_blocks=`expr $sum_blocks + $i`
done

for i in "${RESULTS_FILES[@]}"
do
	sum_files=`expr $sum_files + $i`
done


average_image=`expr $sum_image / $NR_OF_RUNS`
average_blocks=`expr $sum_blocks / $NR_OF_RUNS`
average_files=`expr $sum_files / $NR_OF_RUNS`

echo "Image: "$average_image"ms"
echo "Blocks: "$average_blocks"ms"
echo "Files: "$average_files"ms"