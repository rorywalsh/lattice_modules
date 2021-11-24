#pragma once

#include <iostream>
#include <cmath>
#include <vector>


template <typename... Types>
void ignoreParameters (Types&&...) noexcept {}

/* parameter structure - takes parameter name and
 a vector holding [min, max, increment, defaultValue]*/
struct ExternalParameter
{
    std::vector<float> range;
    std::string parameterName;
    ExternalParameter(std::string name, std::vector<float> paramRange)
    :parameterName(name), range(paramRange){}
};



/* External processing class. The name of the node as it appears in the host is set by the library name.*/
struct LatticeProcessor
{
public:
    LatticeProcessor(){};
    
    /* Called by the host to create parameters. Returns a vector of ExternalPatamers */
    static std::vector<ExternalParameter> createParameters();
    /* This function is called by the host before playback/performance */
    void prepareProcessor(int sr, int block);
    /* Main processing function - paramValues is a list of parameter values passed from the host in
     order of their creation */
    void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues);
};


