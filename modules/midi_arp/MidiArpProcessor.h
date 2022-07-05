#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include <set>
#include <unordered_set>

class MidiArpProcessor : public LatticeProcessorModule
{
public:
    MidiArpProcessor();
    
    ChannelData createChannels() override;
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void processMidi(float** buffer, int numChannels, std::size_t blockSize, const HostData data, std::vector<LatticeMidiMessage>& midiMessages) override;

    const char* getModuleName() override {    return "Midi Arpeggiator";     }
    
    int getRandomNote(int lowestNote, int range)
    {
        return rand() % range + lowestNote;
    }


    int getModuleType() override
    {
        return ModuleType::MidiProcessor::modifier;
    }

private:
    
    enum Type{
        up = 0,
        down,
        upAndDown,
        unsorted,
        random
    };
    
    int arpType;
    
	int sampleIndex = 0;
    int samplingRate = 44100;
    int noteIndex = 0;
    int currentNoteIndex = 0;
    int lastNotePlayed = -1;
    int time = 0;
    int incr = 1;
    int octaves[3] = {0, 12, 24};
    int octaveIndex = 0;
    //std::vector<LatticeMidiMessage> notes;
    std::set<int> notes;
    std::unordered_set<int> unorderedNotes;
    std::vector<std::string> modes;
    bool shouldStopNote = false;
};

