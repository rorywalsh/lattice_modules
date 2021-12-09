#include "sndfile.h"
#include <filesystem>
#include "LatticeProcessorModule.h"
#include <cstdlib>
#ifdef MacOS
#include <dlfcn.h>
#endif

//==================================================================
// Simple class for generating random parameters
// =================================================================
struct RandomParameter{
    int min = 0;
    int max = 0;
    float prevSample = 0;
    std::vector<float> table;
    RandomParameter(float mi, float ma)
    :min(mi), max(ma)
    {
        table.resize(65536);
        std::vector<int> frequencies(10);
        for (int i = 0 ; i < frequencies.size() ; i++){
            frequencies[i] = 1 + float(std::rand())/float(RAND_MAX) * 5;
        }
        
        for (int i = 0; i < table.size(); i++)
        {
            for (int y = 0; y < frequencies.size(); y++) {
                auto freq = frequencies[y];
                float t = sin(2*M_PI*freq*((float)i/(float)table.size()));
                table[i] += t / frequencies.size();
            }
        }
    }
    
    float getValue(int index)
    {
        int i = index % table.size();
        return min + abs(table[index]) * abs(max-min);
    }
};

//==================================================================
// Simple command line parser
// =================================================================
bool cmdOptionExists(const std::string& commandLine, const std::string& option)
{
    auto found = commandLine.find(option);
    return found != -1 ? true : false;
}

//==================================================================
// Main test application.
// =================================================================

int main(int argc, const char **argv)
{
    std::cout << "Usage: ./LatticeTestApp inputModule inputFile outputFile.wav [--randomValues]";
    if(argc < 4)
        return 0;

    std::srand(std::time(nullptr));


    std::vector<std::string> allArgs(argv, argv + argc);
    std::string commandLine;
    for (auto const& s : allArgs) { commandLine += s; }
    bool randomiseValues = false;
    
    if(cmdOptionExists(commandLine, "--randomValues"))
    {
        randomiseValues = true;
        std::cout << "Randomising default values...";
    }
    
    LatticeProcessorModule* extProc;
    SF_INFO sfinfo;
    SNDFILE *fpin, *fpout;
    int bufferSize = 10;
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
    std::vector<RandomParameter> randomParameters;
    for ( auto p : parameters)
    {
        if(p.range.size()!=5)
            std::cout << "Range parameters are not set correctly for " << std::filesystem::absolute(inFile).u8string();

        paramNames.push_back(p.parameterName);
        randomParameters.push_back(RandomParameter(p.range[0], p.range[1]));
        std::atomic<float> attomicParam( p.range[2] );
        paramValues.push_back(&attomicParam);
    }
    
    extProc->mapParameters(paramValues, paramNames);
#endif
    
    fpin = sf_open(std::filesystem::absolute(inFile).u8string().c_str(), SFM_READ, &sfinfo);
    fpout = sf_open(std::filesystem::absolute(outFile).u8string().c_str(), SFM_WRITE, &sfinfo);
    
    int n = 0;
    int pIndex = 0;
    float windspeed = 0;
    if(sfinfo.channels <= 2)
    {
        std::vector<float> buffer(bufferSize);
        do {
            if(randomiseValues)
            {
                for(int i = 0 ; i < paramValues.size() ; i++)
                {
                    std::atomic<float> value = randomParameters[i].getValue(pIndex++);
                    *paramValues[i] = value.load();
                }
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
