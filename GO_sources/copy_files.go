package main

import (
	"fmt"
	"io"
	"os"
	"strconv"
	"sync"
	"time"
)

func copy(wg *sync.WaitGroup, index int) {
	var filename_in = "../input/files/tst" + strconv.Itoa(index)
	var filename_out = "out/files/out" + strconv.Itoa(index)

	in, _ := os.Open(filename_in)
	out, _ := os.Create(filename_out)

	buffer := make([]byte, 1024)

	for {
		n, err := in.Read(buffer)
		if err != nil && err != io.EOF {
			panic(err)
		}
		if n == 0 {
			break
		}

		out.Write(buffer)
	}
	in.Close()
	out.Close()

	wg.Done()
}

func main() {
	wg := new(sync.WaitGroup)

	t := time.Now()
	start := t.UnixMilli()

	for i := 1; i < 100; i++ {
		wg.Add(1)
		go copy(wg, i)
	}

	wg.Wait()

	t = time.Now()
	stop := t.UnixMilli()

	diff := stop - start;
	fmt.Println(diff);

}
