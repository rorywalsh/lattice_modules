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

/* parameter structure - takes parameter name and
 a vector holding [min, max, increment, defaultValue, skew]*/
 //String name, float min, float max, float value, float interval, float skew = 1
struct ModuleParameter
{
	std::string parameterName;
	std::vector<double> range;
    ModuleParameter(std::string name, std::vector<double> paramRange)
		:parameterName(name), range(paramRange) {}
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
						 int /*blockSize*/, 
						 const HostInfo = {}) {}
	
    /* Called by the host when a paremeter changes. The parameterID in this instance is a combination of the unique name for the module,
     assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName method to extract
     the parameter name - note that in the case of audio FX, you can just called getparameter() from your process block*/
	virtual void hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
    {
        //const std::string parameterName =
    }

	virtual void createDescription(std::string& description)
	{
		description = "Override this method to give a description to your processor";
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
            parameterNames.push_back(names[i]);
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
        auto itr = find(parameterNames.begin(), parameterNames.end(), name);
        std::size_t index = static_cast<std::size_t>(std::distance(parameterNames.begin(), itr) - 1);
        return paramValues[index]->load();
    }
    
    /* this method will extract the parameter name, as defined in createParameters(), from the unique
     paramaterId assigned to each parameter by the host */
    std::string getParameterNameFromId(std::string parameterId)
    {
        return parameterId.substr(parameterId.find("-")+2);
    }
    
	/* override this method if you want to draw to the Lattice generic editor viewport */
	virtual std::string getSVGXml() { return ""; }

	/* override this method and return true if you wish to enable drawing on the generic editor viewport. 
	Note that you should only return true when you need the graphics updated. Leaving this permanently set 
	to true will have a negative effect on performance. */
	virtual bool canDraw() { return false; }

private:
	int midiNoteNumber = 0;
    std::string nodeName;
	std::function<void(const std::string&, float)> paramCallback;
    std::vector<std::string> parameterNames;
    std::vector<std::atomic<float>*> paramValues;
};

typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
