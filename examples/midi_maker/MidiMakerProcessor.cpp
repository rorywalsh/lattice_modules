
#include "MidiMakerProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


MidiMakerProcessor::MidiMakerProcessor()
{
    for( int i = 0 ; i < 8  ; i++)
    {
        notes.push_back(getRandomNote(48, 60));
    }
}

LatticeProcessorModule::ChannelData MidiMakerProcessor::createChannels()
{
    return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData MidiMakerProcessor::createParameters()
{
    addParameter({ "Number of Notes in Loop", {1, 32, 8, 1, 1}});
    addParameter({ "Tempo", {0, 20, 1, 0.2, 1}});
    addParameter({ "Lowest Note", {20, 128, 48, 1, 1}});
    addParameter({ "Range in semitones", {0, 60, 12, 1, 1}});
	addParameter({ "Generate new pattern", {0, 1, 1, 1, 1},  Parameter::Type::Trigger });
	addParameter({ "Permit silences", {0, 1, 0, 1, 1},  Parameter::Type::Switch });
	addParameter({ "Play Midi", {0, 1, 0, 1, 1},  Parameter::Type::Switch });
    return ParameterData(getParameters(), getNumberOfParameters());
}

void MidiMakerProcessor::hostParameterChanged(const char* parameterID, float /*newValue*/)
{
    if(getParameterNameFromId(parameterID) == "Generate new pattern")
    {
		std::vector<int> newNotes;
		canUpdate.store(false);
 
        for( int i = 0 ; i < getParameter("Number of Notes in Loop")  ; i++)
        {
            auto addSilence = (int)getParameter("Permit silences");
            if(addSilence > 0)
            {
                if(getRandomNote(0, 100) > 20)
                    newNotes.push_back(getRandomNote((int)getParameter("Lowest Note"), (int)getParameter("Range in semitones")));
                else
					newNotes.push_back(0);
            }
            else
            {
				newNotes.push_back(getRandomNote((int)getParameter("Lowest Note"), (int)getParameter("Range in semitones")));
            }
        }
		notes = newNotes;
		canUpdate.store(true);
		okToDraw = true;
    }
}

void MidiMakerProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
    samplingRate = sr;
}


void MidiMakerProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void MidiMakerProcessor::processMidi(float** /*buffer*/, int /*numChannels*/, std::size_t blockSize, const HostData, std::vector<LatticeMidiMessage>& midiMessages)
{
	int numSamples = blockSize;

    if (getParameter("Play Midi") == 1 && canUpdate.load())
    {
        for (int i = 0; i < numSamples; i++, sampleIndex++)
        {
            const int newNote = noteIndex <= notes.size() ? notes[noteIndex] : 0;
            if (sampleIndex == 1)
            {
                midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f)); //turn off previous note.
                if(playNote == true && newNote != 0)
                    midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOn, 1, newNote, .5f)); // add new previous note.
                lastNotePlayed = newNote;
                playNote = !playNote;
                    
                noteIndex = (noteIndex < notes.size()-1 ? noteIndex+1 : 0);
            }
            sampleIndex = sampleIndex > samplingRate/getParameter("Tempo") ? 0 : sampleIndex + 1;
        }
    }
    else
    {
        midiMessages.push_back(LatticeMidiMessage(LatticeMidiMessage::Type::noteOff, 1, lastNotePlayed, .0f));
    }
}

const char* MidiMakerProcessor::getSVGXml()
{
	const float width = 200;
	const float height = 40;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));


	for (int i = 0; i < notes.size(); i++)
	{
		double x = remap(float(i), 0.f, static_cast<float>(notes.size()), 0.f, width);
		double y = remap(notes[i], (int)getParameter("Lowest Note"), (int)getParameter("Lowest Note") + (int)getParameter("Range in semitones"), 0, height);
		doc << svg::Rectangle(svg::Point(x, height-y), width/notes.size(), y, svg::Fill(svg::Color("#00ABD1")), svg::Stroke(1, svg::Color("#77C1A4"), 2));
	}

	svgText =  doc.toString();
    return svgText.c_str();
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MidiMakerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new MidiMakerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif