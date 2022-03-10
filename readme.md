## Lattice Module API

This repo contains the Lattice Module API. This API provides an interface to the Lattice prototyping environment. This repo makes use the following 3rd party frameworks:

https://github.com/vlazzarini/aurora

https://github.com/adishavit/simple-svg


Examples:
* Gain - this is a bare-bones processing module with a single gain control
* SimpleSynth - a bare-bones synth example. 
* WaveformViewer - a bare-bones example of how to draw SVGs directly to a viewport on a Lattice node. 


To build:

```
git clone https://github.com/rorywalsh/lattice_external
cd lattice_external
git submodule update --init --recursive
mkdir build && cd build
cmake .. 
cmake --build .
```