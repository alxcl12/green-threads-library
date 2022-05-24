package main

import (
	"bufio"
	"fmt"
	"os"
	"time"
)

const WIDTH = 800;
const HEIGHT = 600;


type RGBImage struct {
    r []int
    g []int
	b []int
}

var rgbImage RGBImage;

func make_block_sepia(iBlock int, jBlock int){
	for i := 0; i < 8; i++{
		for j := 0; j < 8; j++{
			pos := ((i + iBlock) * WIDTH) + j + jBlock;

			nR := float32(rgbImage.r[pos]) * 0.39 + float32(rgbImage.g[pos]) * 0.75 + float32(rgbImage.b[pos]) * 0.19;
            nG := float32(rgbImage.r[pos]) * 0.35 + float32(rgbImage.g[pos]) * 0.69 + float32(rgbImage.b[pos]) * 0.17;
            nB := float32(rgbImage.r[pos]) * 0.27 + float32(rgbImage.g[pos]) * 0.53 + float32(rgbImage.b[pos]) * 0.13;

			rgbImage.r[pos] = int(nR)
			rgbImage.g[pos] = int(nG)
			rgbImage.b[pos] = int(nB)

			if (rgbImage.r[pos] > 255) {
                rgbImage.r[pos] = 255;
            }
            if (rgbImage.g[pos] > 255) {
                rgbImage.g[pos] = 255;
            }
            if (rgbImage.b[pos] > 255) {
                rgbImage.b[pos] = 255;
            }
		}
	}
}

func read_picture(filename string) {
    file, _ := os.Open(filename)

    bufr := bufio.NewReader(file)
	_,_,_ = bufr.ReadLine()
	_,_,_ = bufr.ReadLine()
	_,_,_ = bufr.ReadLine()
	_,_,_ = bufr.ReadLine()

	for i := 0; i < HEIGHT; i++ {
		for j := 0; j < WIDTH; j++ {
			r, _ :=bufr.ReadByte();
			g, _ :=bufr.ReadByte();
			b, _ :=bufr.ReadByte();

			rgbImage.r[(i*WIDTH) + j] = int(r);
			rgbImage.g[(i*WIDTH) + j] = int(g);
			rgbImage.b[(i*WIDTH) + j] = int(b);
		}
	}
}

func main() {
	rgbImage.r = make([]int, WIDTH*HEIGHT*4);
	rgbImage.g = make([]int, WIDTH*HEIGHT*4);
	rgbImage.b = make([]int, WIDTH*HEIGHT*4);
	read_picture("../input/picture/nt-P6.ppm");

	t := time.Now()
	start := t.UnixMilli()

	for i := 0; i < HEIGHT; i+=8 {
		for j := 0; j < WIDTH; j+=8 {
			go make_block_sepia(i, j);
		}
	}

	t = time.Now()
	stop := t.UnixMilli()

	diff := stop - start;
	fmt.Println(diff);

	out,_ := os.Create("out/image/test.ppm")
	f := bufio.NewWriter(out)

	f.WriteString("P6\n# CREATOR: GIMP PNM Filter Version 1.1\n800 600\n255\n")

	for i := 0; i < HEIGHT; i++ {
		for j := 0; j < WIDTH; j++ {
			f.WriteByte(byte(rgbImage.r[(i*WIDTH) + j]))
			f.WriteByte(byte(rgbImage.g[(i*WIDTH) + j]))
			f.WriteByte(byte(rgbImage.b[(i*WIDTH) + j]))
		}
	}
}


