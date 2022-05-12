package main

import (
	"image"
	"image/color"
	"os"

	"github.com/lmittmann/ppm"
)

var WIDTH = 800;
var HEIGHT = 600;


func make_block_sepia(img image.Image, new *image.RGBA,iBlock int, jBlock int){

	for i := 0; i < 8; i++{
		for j := 0; j < 8; j++{
			col := img.At(iBlock+i, jBlock+j)
			r, g, b, a := col.RGBA()
			r = uint32(float32(r) * 0.39 + float32(g) * 0.75 + float32(b) * 0.19);
			g = uint32(float32(r) * 0.35 + float32(g) * 0.69 + float32(b) * 0.17);
			b = uint32(float32(r) * 0.27 + float32(g) * 0.53 + float32(b) * 0.13);
			new.Set(iBlock+i, jBlock+j, color.RGBA{uint8(r),uint8(g),uint8(b),uint8(a)})

		}
	}
}

func main() {
	r,err := os.Open("../nt-P6.ppm")

	if err != nil {
		panic(err)
	}
	defer r.Close()
	pic, err := ppm.Decode(r);

	new := image.NewRGBA(pic.Bounds())
	for i := 0; i < HEIGHT; i+=8 {
		for j := 0; j < WIDTH; j+=8 {
			go make_block_sepia(pic, new, i, j);
		}
	}

	out,err := os.Create("test.ppm")
	ppm.Encode(out, new);
}


