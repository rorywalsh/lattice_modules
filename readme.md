### Lattice Processor Module API

Lattice is an audio prototyping framework. This API provides a means of creating DSP modules that can be used in the Lattice audio prototyping environment. Anyone familiar with plugin standards such as VST, AU, ladspa, etc, should be able to find their way around quickly enough.

All module parameters can be accessed in the Lattice graph through a generic editor window. They can also be exposed to the host by adding them to them directly to the Lattice plugin editor.

Modules can act as effects processors (audio/MIDI), or synths. When loaded as a synth, Lattice allocating a unique instance of the module to each voice. You can set the number of voices, thus allowing both mono and polyphonic voicing.

Although the API itself is free of any dependencies, most of the example here use the Aurora header only DSP library. Some of the example also use a forked version of the Simple-SVG library for generating SVG graphics.

#### Examples:

**`chorus`**: a stereo chorus effect

**`delay`**: a simple delay line with feedback

**`flanger`**: a simple flanger

**`freverb`**: a play on the popular freeverb reverb algorithm

**`gain`**: a simple two channel gain module with waveform viewer

**`one_pole_lp`**: a simple-pole low pass filter

**`simple_synth`**: a bare-bone synth with ADSR and selectable waveforms

**`two_pole_svf`**: a two-pole state variable filter

To build:

```
git clone https://github.com/rorywalsh/lattice_external
cd lattice_external
git submodule update --init --recursive
git submodule foreach git pull origin main
mkdir build && cd build
cmake .. 
cmake --build .
```

If accessing the modules from the Azure DevOps build, you will need to validate the modules before Lattice can load them. To do this, navigate to the binaries folder and run:
```
chmod +x validateModules.sh
./validateModules.sh
```


