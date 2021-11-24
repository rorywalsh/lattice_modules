
#include "LatticeExternalProcessor.h"

/*  Called by the host to create set of parameters */
void LatticeProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    ExternalParameter param { "Gain", {0, 1, 0.001, 0.001, 1}}; // { min, max, increment, defaultValue, skew }
    parameters.push_back(param);
}

/*  This function is called by the host before playback/performance */
void LatticeProcessor::prepareProcessor(int sr, int block)
{
    ignoreParameters(sr, block);
}

/*  Main processing function called continously by the host on the audio thread.
    paramValues is a list of parameter values passed from the host in order of their creation */
void LatticeProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
    for ( int i = 0 ; i < blockSize ; i++)
    {
        for ( int chan = 0 ; chan < numChannels; chan++)
        {
            float gain = (*paramValues[0]);
            buffer[chan][i] = buffer[chan][i] * gain;
        }
    }
}
