
#include "LatticeExternalProcessor.h"

#include <iterator>

class Processor : public ExternalProcessor
{
public:
    Processor()
    {}
    
    void createParameters(std::vector<ExternalParameter> &parameters) override
    {
        ExternalParameter param{ "Gain", {0, 1, .1, 1, .5}}; // { min, max, increment, defaultValue, skew }
        parameters.push_back(param);
    }
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue)
    {
        
    }
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, int block) override
    {
        ignoreParameters(sr, block);
    }
    
    
    void triggerParameterUpdate(const std::string& parameterID, float newValue)
    {
        updateParameter(parameterID, newValue);
    }
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) override
    {
        for ( int i = 0 ; i < blockSize ; i++)
        {
            for ( int chan = 0 ; chan < numChannels; chan++)
            {
                buffer[chan][i] = buffer[chan][i] * getParameter("Gain");                
            }
        }
    }
    
private:
 
};

// the class factories
extern "C" ExternalProcessor* create(){             return new Processor;         }
extern "C" void destroy(ExternalProcessor* p){      delete p;                     }

