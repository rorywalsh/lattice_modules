#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class MIDICVProcessor : public LatticeProcessorModule
{
public:
    MIDICVProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    void startNote(int noteNumber, float velocity) override;

    void stopNote(float velocity) override;

    const char* getModuleName() override {    return "MIDI converter";     }   
    
    
private:
    ParamSmooth smf;
    ParamSmooth smv; 
    float freq;
    float amp;
    int ncnt = 0;
    bool gate = false;
    float sr = 44100;
};

