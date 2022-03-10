#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <atomic>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* parameter structure - takes parameter name, a
 a vector holding [min, max, increment, defaultValue, skew], a label name
 and a ParamType which provides a hint for the Lattice generic editor as to what
 the preferred GUI component is */
struct ModuleParameter
{
	enum ParamType {
		Trigger = -99,
		Switch = -98,
		Slider = -97
	};

	std::string parameterName;
	std::vector<double> range;
	ModuleParameter(std::string name, std::vector<double> paramRange, std::string paramLabel = "", ModuleParameter::ParamType cat = ParamType::Slider)
		:parameterName(name), range(paramRange), label(paramLabel), paramCategory(cat) {}

	std::string label = {};
	ParamType paramCategory = ParamType::Slider;
};

/* host info - one of these is passed to the process
 method each time is it called. Can be used to query
 everything from current BPM to whether or not the
 host is playing. (Note: only available in effect plugins)
*/
struct HostInfo
{
	double bpm = 120;
	int timeSigNumerator = 4;
	int timeSigDenomiator = 4;
	long int timeInSamples = 0;
	bool isPlaying = false;
	bool isRecording = false;
	bool isLooping = false;
	double timeInSeconds = 0;
};

struct LatticeMidiMessage {
    enum Type {
        noteOn=1,
        noteOff,
    };
    
    Type msgType;
    int note, channel;
    float velocity;
    
    LatticeMidiMessage(LatticeMidiMessage::Type type, int chan, int noteNum, float vel)
    : msgType(type), note(noteNum), channel(chan), velocity(vel)
    {}

};

/* External processing sub-class */
class LatticeProcessorModule
{

public:
	LatticeProcessorModule() {}

	/* Called by the host to create channel layout. Override and setup inputs/outputs */
	virtual void createChannelLayout(std::vector<std::string>& inputs, std::vector<std::string>& outputs)
	{
		inputs.push_back("Input 1");
		inputs.push_back("Input 2");
		outputs.push_back("Output 1");
		outputs.push_back("Output 2");
	}

	/* Called by the host to create parameters. Override and return a vector of ExternalPatamers */
	virtual void createParameters(std::vector<ModuleParameter>& parameters)
	{
		parameters.push_back({ "Parameter 1", {0, 1, 0.001f, 0.001f, 1} });
	}

	/* This function is called by the host before playback/performance */
	virtual void prepareProcessor(int /*sr*/, int /*block*/) {}

    
    /* Main processing function - hostInfo is a structure containing information from the host */
    virtual void process(float** /*buffer*/,
        int /*numChannels*/,
        int /*blockSize*/) {}
    
	/* Main processing function - hostInfo is a structure containing information from the host */
	virtual void process(float** /*buffer*/,
		int /*numChannels*/,
		int /*blockSize*/,
		const HostInfo) {}

	/* Main processing function - with MIDI data passed from the host */
	virtual void process(float** /*buffer*/,
		int /*numChannels*/,
		int /*blockSize*/,
		std::vector<LatticeMidiMessage>& /*midiMessages*/) {}

    /* Main processing function - with MIDI data passed from the host and hostInfo */
    virtual void process(float** /*buffer*/,
        int /*numChannels*/,
        int /*blockSize*/,
        std::vector<LatticeMidiMessage>& /*midiMessages*/,
        const HostInfo) {}
    
	/* Called by the host when a paremeter changes. The parameterID in this instance is a combination of the unique name for the module,
	 assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName method to extract
	 the parameter name - note that in the case of audio FX, you can just called getparameter() from your process block*/
	virtual void hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
	{
		//const std::string parameterName =
	}

	virtual std::string getNodeInformation()
	{
		return "Override this method to give a description to your processor";
	}

	/* called by the host when a note is started */
	virtual void startNote(int /*midiNoteNumber*/, float /*velocity*/)
	{

	}

	/* Override this method to provide a unique name for the module*/
	virtual std::string getModuleName()
	{
		return "ModuleName";
	}

	/* called by the host when a note is stoped */
	virtual void stopNote(float /*velocity*/)
	{

	}

	/* Called by the host to register parameters and assign callback function for parameter updates*/
	virtual void registerParameters(std::vector<std::atomic<float>*> paramVals,
		std::vector<std::string> names,
		const std::function<void(const std::string&, float)>& func)
	{
		std::size_t i = 0;
		for (auto& p : paramVals)
		{
			paramValues.push_back(p);
			parameterNames.push_back(getParameterNameFromId(names[i]));
			i++;
		}

		paramCallback = func;
	}

	/* returns the note number in Hetrz, according to A tuning*/
	double getMidiNoteInHertz(const int noteNumber, const double Atuning = 440)
	{
		return Atuning * std::pow(2.0, (noteNumber - 69) / 12.0);
	}


	/* called by the host to set the current MIDI note number */
	void setMidiNoteNumber(int noteNumber)
	{
		midiNoteNumber = noteNumber;
	}

	/* retireves the current MIDI note number */
	int getMidiNoteNumber()
	{
		return midiNoteNumber;
	}

	/* Call this to inform the host that a parameter has been updated.
	   Be careful that you don't call this too often! */
	void updateParameter(const std::string& parameterID, float newValue)
	{
		if (paramCallback != nullptr)
			paramCallback(parameterID, newValue);
	}

	/* Used to query the current value of a parameter - the names correspond to the names used
		in the createParameters() function */
	float getParameter(std::string name)
	{
        //these are not picking up teh correct anme...
        int index = 0;
        for(const auto& m : parameterNames)
        {
            if(m == name)
                break;
            index++;
        }
        return paramValues[index]->load();
	}

	/* this method will extract the parameter name, as defined in createParameters(), from the unique
	 paramaterId assigned to each parameter by the host */
	std::string getParameterNameFromId(std::string parameterId)
	{
		return parameterId.substr(parameterId.find("-") + 2);
	}

	/* override this method if you want to draw to the Lattice generic editor viewport */
	virtual std::string getSVGXml() { return ""; }

	/* override this method and return true if you wish to enable drawing on the generic editor viewport.
	Note that you should only return true when you need the graphics updated. Leaving this permanently set
	to true will have a negative effect on performance. */
	virtual bool canDraw() { return false; }
    
    virtual void createDescription(std::string& description) {   return "";  }

private:
	int midiNoteNumber = 0;
	std::string nodeName;
	std::function<void(const std::string&, float)> paramCallback;
	std::vector<std::string> parameterNames;
	std::vector<std::atomic<float>*> paramValues;
};

#ifdef JUCE_MAC
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#else
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#endif
