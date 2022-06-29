
#include "MidiArpProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiArpProcessor::MidiArpProcessor()
{
    modes.push_back("Straight Up");
    modes.push_back("Down");
    modes.push_back("Up and Down");
    modes.push_back("Unsorted");
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
    addParameter({ "Unsorted", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
    addParameter({ "Random", {0, 1, 0, 1, 1}, Parameter::Type::Switch});
 
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiArpProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
   
    const std::string paramName = getParameterNameFromId(parameterID);
    
    if(paramName == "BPM" || paramName == "Transpose")
        return;
    
    currentNote = 0;
    
    auto it = find(modes.begin(), modes.end(), paramName);
    int index = it - modes.begin();
    
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
    currentNote = 0;
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
            {
                unorderedNotes.erase (message.note);
            }
            else
            {
                notes.erase (message.note);
            }
        }
    }

    midiMessages.clear();
    
    if ((time + numSamples) > noteDuration)
    {
        auto offset = std::max (0, std::min((int) (noteDuration - time), numSamples - 1));

        if (lastNotePlayed > 0)
        {
            midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f, offset));
            lastNotePlayed = -1;
        }

        if (notes.size() > 0 || unorderedNotes.size() > 0)
        {
            if ( arpType != Type::unsorted )
            {
                switch(arpType)
                {
                    case(Type::up):
                        currentNote = (currentNote < notes.size() - 1 ? currentNote + 1 : 0);
                        break;
                    case(Type::down):
                        currentNote = (currentNote > 0 ? currentNote - 1 : notes.size() - 1 );
                        break;
                    case(Type::upAndDown):
                        currentNote += incr;
                        if(currentNote == 0 || currentNote >= notes.size()-1)
                            incr = -incr;
                        break;
                    case(Type::random):
                        currentNote = rand() % notes.size();
                        break;
                    default:
                        currentNote = 0;
                };
                
                std::set<int>::iterator it = notes.begin();
                std::advance(it, std::min(currentNote, (int)notes.size()));
                lastNotePlayed = *it;
            }
            else
            {
                currentNote = (currentNote < unorderedNotes.size() - 1 ? currentNote + 1 : 0);
                std::unordered_set<int>::iterator it = unorderedNotes.begin();
                std::advance(it, std::min(currentNote, (int)unorderedNotes.size()));
                lastNotePlayed = *it;
            }
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
