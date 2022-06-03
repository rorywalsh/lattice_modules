
#include "MidiFilterProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiFilterProcessor::MidiFilterProcessor() : out(64)
{

}

LatticeProcessorModule::ChannelData MidiFilterProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData MidiFilterProcessor::createParameters()
{
    addParameter({ "Note number min", {0, 127, 0, 1, 1}});
    addParameter({ "Note number max", {0, 127, 127, 1, 1}});
    addParameter({ "Velocity min", {0, 1, 0, 0.001, 1}});
    addParameter({ "Velocity max", {0, 1, 1, 0.001, 1}});
    addParameter({ "Channel", {0, 16, 0, 1, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiFilterProcessor::hostParameterChanged(const char* parameterID, float /*newValue*/)
{
}

void MidiFilterProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
}

/*
void MidiFilteProcessor::startNote(int noteNumber, float velocity)
{
  mess.type = LatticeMidiMessage::Type::noteOn;
  mess.velocity = velocity;
  mess.note = noteNumber;
}

void MidiFilterProcessor::stopNote (float velocity)
{
  mess.type = LatticeMidiMessage::Type::noteOff;
  mess.velocity = velocity;
  mess.note = noteNumber;
}
*/

void MidiFilterProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiFilterProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{
     int range_max = getParameter("Note number max");
     int range_min = getParameter("Note number min");
     float vel_max = getParameter("Velocity max");
     float vel_min = getParameter("Velocity min");
     out.resize(0);
     for(auto &mess : midiMessages) {
      if((mess.note <= range_max && mess.note >= range_min) &&
	 (mess.velocity <= vel_max && mess.velocity >= vel_min) &&
	 (mess.channel == getParameter("Channel") || getParameter("Channel") == 0)) 
	 out.push_back(mess);
     }
      midiMessages.resize(out.size());
      std::copy(out.begin(), out.end(), midiMessages.begin());     	 
}



// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiFilterProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new MidiFilterProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
