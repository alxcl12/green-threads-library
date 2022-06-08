for n in {1..100}; do
    diff input/files/tst$n C_sources/out/files/out$n
    if [ $? != 0 ]
    then
        echo "Error file with index $n does not match!"
    fi
done
echo "Ok"