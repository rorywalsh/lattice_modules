#pragma once

#include <iostream>
#include <cmath>
#include <vector>


template <typename... Types>
void ignoreParameters (Types&&...) noexcept {}

/* parameter structure - takes parameter name and
 a vector holding [min, max, increment, defaultValue, skew]*/
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
public:
    ExternalProcessor(){};
    
    /* Called by the host to create parameters. Returns a vector of ExternalPatamers */
    virtual void createParameters(std::vector<ExternalParameter> &parameters) = 0;
    /* This function is called by the host before playback/performance */
    virtual void prepareProcessor(int sr, int block) = 0;
    /* Main processing function - paramValues is a list of parameter values passed from the host in
     order of their creation */
    virtual void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) = 0;
    
};

typedef ExternalProcessor* create_t();
typedef void destroy_t(ExternalProcessor*);
