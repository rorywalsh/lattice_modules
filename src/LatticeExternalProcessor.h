#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <map>



template <typename... Types>
void ignoreParameters (Types&&...) noexcept {}

/* parameter structure - takes parameter name and
 a vector holding [min, max, increment, defaultValue, skew]*/
//String name, float min, float max, float value, float interval, float skew = 1
struct ExternalParameter
{
    std::vector<float> range;
    std::string parameterName;
    ExternalParameter(std::string name, std::vector<float> paramRange)
    :parameterName(name), range(paramRange){}
};



/* External processing sub-class */
class ExternalProcessor
{
    
    std::map<std::string, std::atomic<float>*> paramMap;
    
public:
    ExternalProcessor(){};
    
    /* Called by the host to create parameters. Returns a vector of ExternalPatamers */
    virtual void createParameters(std::vector<ExternalParameter> &parameters) = 0;
    /* This function is called by the host before playback/performance */
    virtual void prepareProcessor(int sr, int block) = 0;
    /* Main processing function - paramValues is a list of parameter values passed from the host in
     order of their creation */
    virtual void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) = 0;
    /* Called by the host when a paremeter changes */
    virtual void hostParameterChanged(const std::string& parameterID, float newValue) = 0;
    /* Used to query the current value of a parameter - the names correspond to the names used
        in the createParameters() function */
    float getParameter(std::string name)    {        return paramMap.at(name)->load();    }
    /* called by the host when a note is started */
    virtual void startNote(int midiNoteNumber, float velocity)    {            }
    /* called by the host when a note is stoped */
    virtual void stopNote (float velocity)    {            }
    /* Used to get frequency from Midi note */
    double getMidiNoteInHertz (const int noteNumber, const double Atuning)    {        return Atuning * std::pow (2.0, (noteNumber - 69) / 12.0);    }
    
    /* Called by the host to fill paramValues*/
    virtual void mapParameters(std::vector<std::atomic<float>*> paramValues, std::vector<std::string> paramNames)
    {
        for(int i = 0 ; i < paramNames.size() ; i++)
            paramMap[paramNames[i]] = paramValues[0];
    }
    
    /* Called by host when a Midi note is triggered */
    void setMidiNoteNumber(int noteNumber)  { midiNoteNumber = noteNumber;      }
    /* Used to query current Midi note */
    int getMidiNoteNumber()                 { return midiNoteNumber;            }

    
private:
    int midiNoteNumber = 0;
};

typedef ExternalProcessor* create_t();
typedef void destroy_t(ExternalProcessor*);
