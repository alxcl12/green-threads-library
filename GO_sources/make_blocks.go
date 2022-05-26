package main

import (
	"bufio"
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	"sync"
	"time"
)

type position struct {
    i int
    j int
}

const WIDTH = 800;
const HEIGHT = 600;

var matrix [HEIGHT][WIDTH]int;

func write_block(wg *sync.WaitGroup, iBlock int, jBlock int, count int){
	var filename_out = "out/blocks/block" + strconv.Itoa(count)

	out, _ := os.Create(filename_out)

	bufr := bufio.NewWriter(out)
	var polynom float64 = 0;
	for i := 0; i < 8; i++{
		for j := 0; j < 8; j++{
			fmt.Fprintf(bufr, "%d ", matrix[iBlock + i][jBlock + j])
			if(i==j){
				polynom += math.Pow(float64(matrix[iBlock+i][jBlock+j]), float64(i))
			}
		}
		fmt.Fprintf(bufr, "\n")
		bufr.Flush()
	}
	fmt.Fprintf(bufr, "%f", polynom)
	bufr.Flush()

	out.Close()
	wg.Done()
}

func read_matrix(filename string) {
    file, _ := os.Open(filename)

    bufr := bufio.NewReader(file)

	for i := 0; i < HEIGHT; i++ {
		for j := 0; j < WIDTH; j++ {
			fmt.Fscanf(bufr, "%d", &matrix[i][j])
		}
	}
}

func main() {
	read_matrix("../input/matrix/matrix.in");

	var positions [7500]position;
	var counter int = 0;

	for i := 0; i < HEIGHT; i+=8 {
		for j := 0; j < WIDTH; j+=8 {
			positions[counter].i = i;
			positions[counter].j = j;
			counter++;
		}
	}

	rand.Seed(time.Now().UnixNano())
	rand.Shuffle(len(positions), func(i, j int) { positions[i], positions[j] = positions[j], positions[i] })

	wg := new(sync.WaitGroup)

	t := time.Now()
	start := t.UnixMilli()

	for i := 0; i < len(positions); i++{
		wg.Add(1)
		go write_block(wg, positions[i].i, positions[i].j, i)
	}
	wg.Wait()

	t = time.Now()
	stop := t.UnixMilli()

	diff := stop - start;
	fmt.Println(diff);
}


