#!/bin/bash

NR_OF_RUNS=$1
RESULTS_IMAGE=()
RESULTS_BLOCKS=()
RESULTS_FILES=()
TOTAL_GO=0

cd GO_sources
rm -rf out/blocks/*
rm -rf out/files/*
rm -rf out/picture/*
for (( c=1; c<=$NR_OF_RUNS; c++ ))
do
   REZ_GO=$(go run image_filter.go)
   echo "$REZ_GO";
   RESULTS_IMAGE+=($REZ_GO)

   REZ_GO=$(go run make_blocks.go)
   echo "$REZ_GO";
   RESULTS_BLOCKS+=($REZ_GO)

   REZ_GO=$(go run copy_files.go)
   echo "$REZ_GO";
   RESULTS_FILES+=($REZ_GO)

   rm -rf out/blocks/*
   rm -rf out/files/*
   rm -rf out/picture/*
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