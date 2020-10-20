This is a modified version of a submission for a Twitter image encoding challenge found here:
https://stackoverflow.com/a/929360


## Requirements
* TCLAP (https://github.com/eile/tclap)
* Magic++ (libgraphicsmagick1-dev)

## Install 
* Clone the repo.
  1. `git clone https://github.com/NeuroWinter/NanoBrunch.git`
  2. Change directory to the project `cd NanoBrunch`
* Make the project.
  1. `cmake CMakeLists.txt`
* If there are problems try and find what dependencies are missing.

## Running the project
1. Encode the image.
  * `./nanobrunch -e -i {Your input image} -o {OutputFilename}`
2. Decode the image.
  * `./nanobrunch -d -i {OutputFilename from step one} -o {FinalImageName}.png`

## Flags:
You can only use one of these at a time.
* `-e` Encode the input image.
* `-d` Decode the input image.

## Parameters:
* `-i` Path and filename of the input image.
* `-o` Path and filename of the output image.
* `-x` Steps in X.
  * I am not too sure what this does at this point.
* `-y` Steps in Y.
  * I am not too sure what this does at this point.
* `-r` Steps in red.
  * I am not too sure what this does at this point.
* `-g` Steps in green.
  * I am not too sure what this does at this point.
* `-b` Steps in blue.
  * I am not too sure what this does at this point.
* `-k` Blocks in X.
  * if you increase this number there will be more sections of the image on the X axis that are split up into fractal representations of that block.
* `-l` Blocks in Y.
  * if you increase this number there will be more sections of the image on the Y axis that are split up into fractal representations of that block.
* `-a` Weight for colour A.
  * A higher weight here will make the final image more blocky. 
* `-c` Weight for colour B.
  * A higher weight here will make the final image more 'fractaly'. 
* `-t` Iterations for decoding.
  * This doesn't do too much.
* `-z` Zoom level.
  * This is a great setting to play with. Both encoding and decoding uses this to find fractals.
* `-s` Output resolution.
  * Force the output resolution to a set size.
* `--clr` Colour space.
  * Supported Colour types are: CMY, CYMK, GRAY, HCL, HCLp, HSB, HSI, HSL, HSV, HWB, HWB, LAB, LCH, LCHAB, LCHUV, LOG, LMS, LUV, OHTA, RGB, SCRGB, SRGB, XYY, XYZ, YCB, YCC, YDB, YIQ, YPB, YUV. 

