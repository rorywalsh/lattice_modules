## Lattice Module API

This repo contains the Lattice Module API. This API provides an interface to the Lattice prototyping environment. This repo makes use the following 3rd party frameworks:

https://github.com/vlazzarini/aurora
https://github.com/adishavit/simple-svg


Examples:
* Gain - this is a bare-bones processing module with a single gain control
* GainDebug - same as above, but shows how debug information can be placed on the module's editor in Lattice
* SimpleSynth - a bare-bones synth example. 

To build:

```
git clone https://github.com/rorywalsh/lattice_external
cd lattice_external
mkdir build && cd build
cmake .. 
cmake --build .
```