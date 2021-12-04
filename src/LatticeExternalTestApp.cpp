#include "sndfile.h"
#include <filesystem>
#include "LatticeExternalProcessor.h"
#include <cstdlib>
#ifdef MacOS
#include <dlfcn.h>
#endif


struct Range{
    Range(float mi, float ma, float val):min(mi), max(ma), value(val){}
    
    float getRandomValue()
    {
        return min + (float(std::rand())/float(RAND_MAX))*max-min;
    }
    float min, max;
    float value = 0;
};

bool cmdOptionExists(const std::string& commandLine, const std::string& option)
{
    auto found = commandLine.find(option);
    return found != -1 ? true : false;
}


int main(int argc, const char **argv)
{
    std::srand(std::time(nullptr));
    std::cout << "Usage: ./LatticeTestApp inputModule inputFile outputFile.wav [--randomValues]";
    std::vector<std::string> allArgs(argv, argv + argc);
    std::string commandLine;
    for (auto const& s : allArgs) { commandLine += s; }
    bool randomiseValues = false;
    
    if(cmdOptionExists(commandLine, "--randomValues"))
    {
        randomiseValues = true;
        std::cout << "Randomising default values...";
    }
    
    ExternalProcessor* extProc;
    SF_INFO sfinfo;
    SNDFILE *fpin, *fpout;
    int bufferSize = 22050;
    std::filesystem::path moduleFile = argv[1];
    std::filesystem::path inFile = argv[2];
    std::filesystem::path outFile = argv[3];
    void* handle;
    std::vector<std::atomic<float>*> paramValues;
    
#ifdef MacOS
    handle = dlopen(std::filesystem::absolute(moduleFile).u8string().c_str(), RTLD_LAZY);
    if (!handle) {
        std::cout << "Couldn't open module: " << std::filesystem::absolute(inFile).u8string();
        return 0;
    }
    dlerror();
    create_t* createProcessor = (create_t*) dlsym(handle, "create");
    const char* dlsym_error = dlerror();
    if (dlsym_error)
    {
        std::cerr << "Cannot load symbol create: " << dlsym_error << '\n';
    }
    
    extProc = createProcessor();

    std::vector<ExternalParameter> parameters;
    extProc->createParameters(parameters);
    std::vector<std::string> paramNames;
    std::vector<Range> paramRanges;
    for ( auto p : parameters)
    {
        if(p.range.size()!=5)
            std::cout << "Range parameters are not set correctly for " << std::filesystem::absolute(inFile).u8string();

        paramNames.push_back(p.parameterName);
        paramRanges.push_back(Range(p.range[0], p.range[1], p.range[2]));
        std::atomic<float> attomicParam( p.range[2] );
        paramValues.push_back(&attomicParam);
    }
    
    extProc->mapParameters(paramValues, paramNames);
#endif
    
    fpin = sf_open(std::filesystem::absolute(inFile).u8string().c_str(), SFM_READ, &sfinfo);
    fpout = sf_open(std::filesystem::absolute(outFile).u8string().c_str(), SFM_WRITE, &sfinfo);
    
    int n = 0;
    float windspeed = 0;
    if(sfinfo.channels <= 2)
    {
        std::vector<float> buffer(bufferSize);
        do {

            for(int i = 0 ; i < paramValues.size() ; i++)
            {
                std::atomic<float> value( paramRanges[i].getRandomValue() );
                *paramValues[i] = value.load();
            }
            n = sf_read_float(fpin, buffer.data(), bufferSize);
            auto d = buffer.data();
            extProc->process(&d, 1, bufferSize, paramValues);
            sf_write_float(fpout, d, n);
        } while(n);
    }
    else{
        std::cout << "Number of channels not supported";
        return 0;
    }

    sf_close(fpin);
    sf_close(fpout);

    return 0;
}
