#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <atomic>
#include <functional>
/* parameter structure - takes parameter name and
 a vector holding [min, max, increment, defaultValue, skew]*/
 //String name, float min, float max, float value, float interval, float skew = 1
struct ExternalParameter
{
	std::string parameterName;
	std::vector<float> range;
	ExternalParameter(std::string name, std::vector<float> paramRange)
		:parameterName(name), range(paramRange) {}
};


/* External processing sub-class */
class LatticeNodeProcessor
{

	std::map<std::string, std::atomic<float>*> paramMap;

public:
	LatticeNodeProcessor() {}

	/* Called by the host to create channel layout. Override and setup inputs/outputs */
	virtual void createChannelLayout(std::vector<std::string>& inputs, std::vector<std::string>& outputs)
	{
		inputs.push_back("Input 1");
		inputs.push_back("Input 2");
		outputs.push_back("Output 1");
		outputs.push_back("Output 2");
	}
	/* Called by the host to create parameters. OVerride and return a vector of ExternalPatamers */
	virtual void createParameters(std::vector<ExternalParameter>& parameters)
	{
		parameters.push_back({ "Gain", {0, 1, 0.001, 0.001, 1} });
	}

	/* This function is called by the host before playback/performance */
	virtual void prepareProcessor(int sr, int block) {};
	/* Main processing function - paramValues is a list of parameter values passed from the host in
	 order of their creation */
	virtual void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) {};
	/* Called by the host when a paremeter changes */
	virtual void hostParameterChanged(const std::string& parameterID, float newValue) {};

	virtual void createDescription(std::string& description)
	{
		description = "Override this method to give a description to your processor";
	};

	/* called by host to register parameter callback function */
	void registerCallback(const std::function<void(const std::string&, float)>& func)
	{
		paramCallback = func;
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
		return paramMap.at(name)->load();
	}

	/* called by the host when a note is started */
	virtual void startNote(int /*midiNoteNumber*/, float /*velocity*/)
	{

	}

	/* called by the host when a note is stoped */
	virtual void stopNote(float /*velocity*/)
	{

	}

	double getMidiNoteInHertz(const int noteNumber, const double Atuning)
	{
		return Atuning * std::pow(2.0, (noteNumber - 69) / 12.0);
	}

	/* Called by the host to fill paramValues*/
	virtual void mapParameters(std::vector<std::atomic<float>*> paramValues, std::vector<std::string> paramNames)
	{
		for (int i = 0; i < paramNames.size(); i++)
			paramMap[paramNames[i]] = paramValues[i];
	}

	void setMidiNoteNumber(int noteNumber) { midiNoteNumber = noteNumber; }
	int getMidiNoteNumber() { return midiNoteNumber; }


private:
	int midiNoteNumber = 0;
	std::function<void(const std::string&, float)> paramCallback;
};

typedef LatticeNodeProcessor* create_t();
typedef void destroy_t(LatticeNodeProcessor*);
