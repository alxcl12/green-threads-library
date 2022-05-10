package main

import (
	"fmt"
	"io"
	"os"
	"strconv"
	"sync"
)

func copy(wg *sync.WaitGroup, index int) {
	var filename_in = "../files/tst" + strconv.Itoa(index)
	var filename_out = "../files/ost" + strconv.Itoa(index)

	//fmt.Println(filename_in)
	in, err := os.Open(filename_in)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer in.Close()

	out, err := os.Create(filename_out)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer out.Close()

	// create a buffer to keep chunks that are read

	buffer := make([]byte, 1024)

	for {
		// read a chunk
		n, err := in.Read(buffer)
		if err != nil && err != io.EOF {
			panic(err)
		}
		if n == 0 {
			break
		}

		out.Write(buffer)

		// // out the file content
		// fmt.Println(string(buffer[:n]))

	}
	in.Close()
	out.Close()

	wg.Done()
}

func main() {
	wg := new(sync.WaitGroup)

	for i := 1; i < 100; i++ {
		wg.Add(1)
		go copy(wg, i)
	}

	wg.Wait()
}
