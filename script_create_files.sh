cd input/files


for n in {1..100}; do
    dd if=/dev/urandom of=tst$( printf %d "$n" ) bs=1k count=$(( RANDOM + 1024 ))
done