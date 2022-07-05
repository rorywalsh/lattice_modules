
#include "MidiArpProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiArpProcessor::MidiArpProcessor()
{
    modes.push_back("Straight Up");
    modes.push_back("Down");
    modes.push_back("Up and Down");
    //modes.push_back("Unsorted");
    modes.push_back("Random");
}

LatticeProcessorModule::ChannelData MidiArpProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData MidiArpProcessor::createParameters()
{
    addParameter({ "Enabled", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    addParameter({ "BPM", {20, 520, 120, 0.01, 1}, Parameter::Type::Slider, true});
    addParameter({ "Straight Up", {0, 1, 1, 1, 1}, Parameter::Type::Switch});
    addParameter({ "Down", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    addParameter({ "Up and Down", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    //addParameter({ "Unsorted", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    addParameter({ "Random", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    addParameter({ "Octave Range", {1, 3, 1, 1, 1}, Parameter::Type::Slider});
    addParameter({ "One-shot", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
 
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiArpProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
   
    const std::string paramName = getParameterNameFromId(parameterID);
    
    if(paramName == "BPM" || paramName == "Transpose")
        return;
    
    currentNoteIndex = 0;
    
    
    for (int i = 0 ; i < modes.size() ; i++)
    {
        if(modes[i] == paramName)
        {
            arpType = i;
            updateHostParameter(modes[i].c_str(), 1.f);
        }
        else
        {
            updateHostParameter(modes[i].c_str(), 0.f);
        }
    }
    
}

void MidiArpProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;
    notes.clear();
    unorderedNotes.clear();
    currentNoteIndex = 0;
}


void MidiArpProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiArpProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{
	int numSamples = blockSize;

    if(getParameter("Enabled") == 0.f)
        return;
    
    auto noteDuration = static_cast<int> (samplingRate / ((getParameter("BPM") / 60.f)));

    for (auto& message : midiMessages)
    {
        if(message.msgType == LatticeMidiMessage::Type::noteOn)
        {
            if(arpType == Type::unsorted)
                unorderedNotes.insert (message.note);
            else
                notes.insert (message.note);
            
        }
        else if ( message.msgType == LatticeMidiMessage::Type::noteOff )
        {
            if ( arpType == Type::unsorted )
                unorderedNotes.erase (message.note);
            else
                notes.erase (message.note);
        }
    }

    midiMessages.clear();
    
    for (int i = 0; i < numSamples; i++)
    {
        if (sampleIndex == 1)
        {
            int range = getParameter("Octave Range") - 1;
            std::cout << currentNoteIndex << std::endl;
            if(currentNoteIndex == notes.size()-1)
            {
                octaveIndex = octaveIndex < range ? octaveIndex+1 : 0;
                std::cout << octaves[octaveIndex] << std::endl;
            }
            
            if (shouldStopNote)
            {
                midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f));
                shouldStopNote = false;
            }

            if (notes.size() > 0 || unorderedNotes.size() > 0)
            {
                if ( arpType != Type::unsorted )
                {
                    switch(arpType)
                    {
                        case(Type::up):
                            currentNoteIndex = (currentNoteIndex < notes.size() - 1 ? currentNoteIndex + 1 : 0);
                            break;
                        case(Type::down):
                            currentNoteIndex = (currentNoteIndex > 1 ? currentNoteIndex - 1 : notes.size() - 1 );
                            break;
                        case(Type::upAndDown):
                            currentNoteIndex += incr;
                            if(currentNoteIndex == 0 || currentNoteIndex >= notes.size()-1)
                                incr = -incr;
                            break;
                        case(Type::random):
                            currentNoteIndex = rand() % notes.size();
                            break;
                        default:
                            currentNoteIndex = 0;
                    };
                    
                    std::set<int>::iterator itA = notes.begin();
                    std::advance(itA, std::min(std::max(0, currentNoteIndex), (int)notes.size()));
                    
                    if(lastNotePlayed != *itA)
                    {
                        midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, octaves[octaveIndex] + *itA, .5f));
                        lastNotePlayed = *itA;
                    }
                    else
                    {
                        //if a new note is injected below the current lowest note we need to iterate to avoid repeated notes..
                        std::set<int>::iterator itB = notes.begin();
                        std::advance(itB, std::min(1, (int)notes.size()-1));
                        midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, octaves[octaveIndex] + *itB, .5f));
                        lastNotePlayed = *itB;
                    }
                    
                    if(getParameter("One-shot") == 1 && currentNoteIndex == notes.size()-1)
                        notes.clear();
                }
                
                shouldStopNote = true;
            }
        }
        sampleIndex = sampleIndex >= noteDuration ? 0 : sampleIndex + 1;
    }

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
