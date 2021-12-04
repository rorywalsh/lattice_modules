#include "sndfile.h"
#include <filesystem>
#include "LatticeExternalProcessor.h"
#ifdef MacOS
#include <dlfcn.h>
#endif

int main(int argc, const char **argv)
{
    ExternalProcessor* extProc;
    SF_INFO sfinfo;
    SNDFILE *fpin, *fpout;
    int bufferSize = 64;
    std::filesystem::path moduleFile = argv[1];
    std::filesystem::path inFile = argv[2];
    std::filesystem::path outFile = argv[3];
    void* handle;
    
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
#endif
    
    fpin = sf_open(std::filesystem::absolute(inFile).u8string().c_str(), SFM_READ, &sfinfo);
    fpout = sf_open(std::filesystem::absolute(outFile).u8string().c_str(), SFM_WRITE, &sfinfo);
    std::vector<std::atomic<float>*> paramValues;
    int n = 0;
    
    if(sfinfo.channels == 1)
    {
        std::vector<float> buffer(bufferSize);
        do {
            n = sf_read_float(fpin, buffer.data(), bufferSize);
            auto d = &buffer[0];
            extProc->process(&d, 1, bufferSize, paramValues);
            sf_write_float(fpout, d, n);
        } while(n);
    }
    else if(sfinfo.channels == 2)
    {
        std::vector<float> buffer(bufferSize);
        do {
            n = sf_read_float(fpin, buffer.data(), bufferSize);
            auto d = &buffer[0];
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
