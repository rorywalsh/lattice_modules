#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class FilePlayerProcessor : public LatticeProcessorModule
{
public:
    FilePlayerProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    void hostParameterChanged(const char* parameterID, const char* newValue) override;
    void hostParameterChanged(const char* parameterID, float newValue) override;
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    void createDescription(std::string& description)
    {
        description = "";
    }

    void startNote(int noteNumber, float velocity) override;

    /** called by the host when a note is stoped
     * @param velocity Midi velocity in the range of 0 to 1
    */
    void stopNote(float velocity) override;

    /* override this method if you want to draw to the Lattice generic editor viewport */
    const char* getSVGXml() override;

    /* override this method and return true if you wish to enable drawing on the generic editor viewport */
    bool canDraw() override {
        auto draw = okToDraw;
        okToDraw = false;
        return draw;
    }

    bool acceptsMidiInput() override
    {
        return true;
    }

    static float remap(float value, float from1, float to1, float from2, float to2)
    {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }
    
    const char* getModuleName() override {    return "Soundfile Player";     }
    
private:
    bool noteOff = false;
    bool oneShot = false;
    int numSamples = 0;
    int releaseSegment = 0;
    float sampleIncrement = 1;
    std::vector<float> soundfileSamples;
    bool okToDraw = true;
    std::string svgText;
    bool isPlaying = false;
    float sampleIndex = 0;
    bool fileLoaded = false;
    
};

