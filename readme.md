### Lattice Processor Module API

Lattice is an audio prototyping framework. This API provides a means of creating DSP modules that can be used in the Lattice audio prototyping environment. Anyone familiar with plugin standards such as VST, AU, ladspa, etc, should be able to find their way around quickly enough.

All module parameters can be accessed in the Lattice graph through a generic editor window. They can also be exposed to the host by adding them to them directly to the Lattice plugin editor.

Modules can act as effects processors (audio/MIDI), or synths. When loaded as a synth, Lattice allocating a unique instance of the module to each voice. You can set the number of voices, thus allowing both mono and polyphonic voicing.

Although the API itself is free of any dependencies, most of the example here use the Aurora header only DSP library. Some of the example also use a forked version of the Simple-SVG library for generating SVG graphics.

#### Examples:
bl_synth: demonstrates a a simple band-limited polyphonic synth with selectable waveforms, an ADSR, and PWM.

chorus: a stereo chorus effect

delay: a simple delay line with feedback

drive_synth: a non band-limited polyphonic wave-shaper synth

flanger: a simple flanger

four_pole_lp: a resonant, four-pole low pass filter

ks_synth: a simple Karplus Strong synth

midi_maker: a basic MIDI pattern generator

one_pole_lp: a one-pole low pass filter

param_eq: a single band parametric EQ

simple_synth: a bare-bone synth with ADSR and selectable waveforms

two_pole_svf: a two-pole state variable filter

waveform_viewer: a bare-bones example of how to draw SVGs directly to a viewport on a Lattice module.

To build:
```
git clone https://github.com/rorywalsh/lattice_external
cd lattice_external
git submodule foreach git pull origin main
mkdir build && cd build
cmake .. 
cmake --build .
```
```
