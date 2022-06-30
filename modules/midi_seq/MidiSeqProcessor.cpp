
#include "MidiSeqProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiSeqProcessor::MidiSeqProcessor()
{
    for ( int i = 0 ; i < 16 ; i++)
    {
        noteParams.push_back("Notes " + std::to_string(i+1));
        velParams.push_back("Velocity " + std::to_string(i+1));
    }
    
}

LatticeProcessorModule::ChannelData MidiSeqProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData MidiSeqProcessor::createParameters()
{
    for ( int i = 0 ; i < numSteps ; i++)
    {
        addParameter({ noteParams[i].c_str(), {12, 120, 60+i, 1, 1}, Parameter::Type::Slider, true });
        addParameter({ velParams[i].c_str(), {0, 1, .5f, 0.001f, 1}, Parameter::Type::Slider, true });
    }

    addParameter({ "BPM", {1, 600, 120, 1, 1}, Parameter::Type::Slider, true });
    addParameter({ "Randomise Notes", {0, 10, 0, 1, 1}, Parameter::Type::Trigger});
    addParameter({ "Randomise Velocities", {0, 1, 0, 1, 1}, Parameter::Type::Trigger});
    addParameter({ "Play", {0, 1, 0, 1, 1},  Parameter::Type::Switch, true });
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiSeqProcessor::hostParameterChanged(const char* parameterID, float /*newValue*/)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Randomise Notes")
    {
        for( int i = 0 ; i < numSteps ; i++)
            updateHostParameter(noteParams[i].c_str(), 48 + rand() % 24);

    }
    else if(paramName == "Randomise Velocities")
    {
        for( int i = 0 ; i < numSteps ; i++)
        {
            float vel = float(rand() % 100) / 100.f;
            updateHostParameter(velParams[i].c_str(), vel > 0.4 ? vel : 0);
        }
    }
}

void MidiSeqProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;
}


void MidiSeqProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiSeqProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{
	int numSamples = blockSize;


    if (getParameter("Play"))
    {
        for (int i = 0; i < numSamples; i++, sampleIndex++)
        {
            if (sampleIndex == 1)
            {
                const int newNote = getParameter(noteParams[noteIndex]);
                const float newVel = getParameter(velParams[noteIndex]);
                midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f)); //turn off previous note.
                midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, newNote, newVel)); // add new previous note.
                lastNotePlayed = newNote;
                noteIndex = (noteIndex < numSteps ? noteIndex+1 : 0);
            }
            sampleIndex = sampleIndex > (samplingRate/(getParameter("BPM")/60)) ? 0 : sampleIndex + 1;
        }
    }
    else
    {
        midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f));
    }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiSeqProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new MidiSeqProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
