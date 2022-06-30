
#include "MidiDelayProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiDelayProcessor::MidiDelayProcessor()
{
}

LatticeProcessorModule::ChannelData MidiDelayProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData MidiDelayProcessor::createParameters()
{
    addParameter({ "Delay Time", {0, 10, 0, 0.01, 1}, Parameter::Type::Slider, true});
    addParameter({ "Velocity", {0, 1, 0.5f, 0.001, 1}, Parameter::Type::Slider, true});
    addParameter({ "Transpose", {-24, 24, 0, 1, 1} });
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiDelayProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    
}

void MidiDelayProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;

}


void MidiDelayProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiDelayProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{    
    auto delay = static_cast<int> (samplingRate * (getParameter("Delay Time")));

   
    for (auto& message : midiMessages)
    {
        if(message.msgType == LatticeMidiMessage::Type::noteOn)
            outgoingNotes.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, message.note + getParameter("Transpose"), getParameter("Velocity"), message.offset + sampleIndex + delay));
        
        else if ( message.msgType == LatticeMidiMessage::Type::noteOff )
            outgoingNotes.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, message.note + getParameter("Transpose"), getParameter("Velocity"), message.offset + sampleIndex+delay));
        
    }

    midiMessages.clear();
    for (int i = 0; i < blockSize; i++, sampleIndex++)
    {
        for (int i = outgoingNotes.size()-1 ; i >= 0 ; i--)
        {
            if (sampleIndex >= outgoingNotes[i].offset)
            {
                if(outgoingNotes[i].note > 0)
                {
                    midiMessages.push_back(LatticeMidiMessage(outgoingNotes[i].msgType, 1, outgoingNotes[i].note, getParameter("Velocity")));
                }
                outgoingNotes.erase(outgoingNotes.begin() + i);
            }
        }
    }


}
   

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiDelayProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
    extern "C" LatticeProcessorModule* create(){             return new MidiDelayProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
