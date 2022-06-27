
#include "MidiArpProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiArpProcessor::MidiArpProcessor()
{
 
}

LatticeProcessorModule::ChannelData MidiArpProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData MidiArpProcessor::createParameters()
{
    addParameter({ "Speed", {0, 1, .5, 0.01, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiArpProcessor::hostParameterChanged(const char* parameterID, float /*newValue*/)
{

}

void MidiArpProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;
    notes.clear();
}


void MidiArpProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiArpProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{
	int numSamples = blockSize;
    
    // get note duration
    auto noteDuration = static_cast<int> (std::ceil (samplingRate * 0.25f * (0.1f + (1.0f - (getParameter("Speed"))))));

    for (auto& message : midiMessages)
    {
        if(message.msgType == LatticeMidiMessage::Type::noteOn)
            notes.insert (message.note);
        else if(message.msgType == LatticeMidiMessage::Type::noteOn)
            notes.erase(message.note);
    }

    //midiMessages.clear();

    if ((time + numSamples) >= noteDuration)
    {
        auto offset = std::max (0, std::min((int) (noteDuration - time), numSamples - 1));

        if (lastNotePlayed > 0)
        {
            midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f, offset));
            lastNotePlayed = -1;
        }

        if (notes.size() > 0)
        {
            currentNote = (currentNote + 1) % notes.size();
            std::set<int>::iterator it = notes.begin();
            std::advance(it, currentNote);
            lastNotePlayed = *it;
            midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, lastNotePlayed, .5f, offset));
        }

    }

    time = (time + numSamples) % noteDuration;
}
   

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiArpProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
    extern "C" LatticeProcessorModule* create(){             return new MidiArpProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
