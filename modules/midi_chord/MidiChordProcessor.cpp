
#include "MidiChordProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiChordProcessor::MidiChordProcessor()
{
    chords.push_back("Major");
    intervals.push_back({{3, 5, 12}});
    chords.push_back("Major 6th");
    intervals.push_back({{3, 5, 6}});
    chords.push_back("Major 7th");
    intervals.push_back({{3, 5, 7}});
    chords.push_back("Minor");
    intervals.push_back({{2, 5, 12}});
    chords.push_back("Minor 6th");
    intervals.push_back({{2, 5, 6}});
    chords.push_back("Minor 7th");
    intervals.push_back({{2, 5, 7}});
    chords.push_back("Dominant 7th");
    intervals.push_back({{3, 5, 6}});
    chords.push_back("Diminished");
    intervals.push_back({{2, 4, 12}});
    
}

LatticeProcessorModule::ChannelData MidiChordProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData MidiChordProcessor::createParameters()
{
    for ( int i = 0 ; i < chords.size() ; i++)
    {
        addParameter({ chords[i].c_str(), {0, 1, 1, 1, 1}, Parameter::Type::Switch, true});
    }
    
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiChordProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    
    for (int i = 0 ; i < chords.size() ; i++)
    {
        if(chords[i] == paramName)
        {
            updateHostParameter(chords[i].c_str(), 1.f);
            chordIndex = i;
        }
        else
        {
            updateHostParameter(chords[i].c_str(), 0.f);
        }
    }
    
}

void MidiChordProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;
    outgoingNotes.clear();
    notes.clear();
}


void MidiChordProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiChordProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{    

 
    for (auto& message : midiMessages)
    {
        if(message.msgType == LatticeMidiMessage::Type::noteOn)
        {
            outgoingNotes.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, message.note, message.velocity, message.offset + sampleIndex));
            notes.push_back(message.note);
        }
        
        else if ( message.msgType == LatticeMidiMessage::Type::noteOff )
        {
            outgoingNotes.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, message.note, 0, message.offset + sampleIndex));
            auto it = find(notes.begin(), notes.end(), message.note);
            notes.erase(it);
        }
        
    }

    midiMessages.clear();
    
    addNotesToMidiBuffer(midiMessages, chordIndex);

}
   
void MidiChordProcessor::addNotesToMidiBuffer(std::vector<LatticeMidiMessage>& midiMessages, int index)
{
    for (int i = outgoingNotes.size()-1 ; i >= 0 ; i--)
    {
        if(outgoingNotes[i].note < 128)
        {
                midiMessages.push_back(LatticeMidiMessage(outgoingNotes[i].msgType, 1, outgoingNotes[i].note, outgoingNotes[i].velocity, outgoingNotes[i].offset));
                midiMessages.push_back(LatticeMidiMessage(outgoingNotes[i].msgType, 1, outgoingNotes[i].note + intervals[chordIndex][0], outgoingNotes[i].velocity, outgoingNotes[i].offset));
                midiMessages.push_back(LatticeMidiMessage(outgoingNotes[i].msgType, 1, outgoingNotes[i].note + intervals[chordIndex][0], outgoingNotes[i].velocity, outgoingNotes[i].offset));
                midiMessages.push_back(LatticeMidiMessage(outgoingNotes[i].msgType, 1, outgoingNotes[i].note + intervals[chordIndex][2], outgoingNotes[i].velocity, outgoingNotes[i].offset));
        }
        
        outgoingNotes.erase(outgoingNotes.begin() + i);
    }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiChordProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
    extern "C" LatticeProcessorModule* create(){             return new MidiChordProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
