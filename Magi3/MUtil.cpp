/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Include core headers
#include "MUtil.h"

void __UtilAssert(const char* FileName, int LineNumber, bool Assertion, const char* FailText, ...)
{
    // Validate assertion
    if(Assertion)
        return;
    
    // Validate the given error string
    if(FailText == NULL)
        FailText = "Undefined Error Text.";
    
    // Find the clean file name
    const char* CleanFileName = NULL;
    
    // Don't find the simple name if the string is null
    if(FileName != NULL)
    {
        // Thanks windows, for being such an evil file system
        // Note that the "+1" moves the pointer past the slash
        #ifdef _WIN32
            CleanFileName = strrchr(FileName, '\\') + 1;
        #else
            CleanFileName = strrchr(FileName, '/') + 1;
        #endif
    }
    
    // Grab the variable argument list (See the "..." in the arg. list)
    va_list args;
    va_start(args, FailText);
    
    // Generate the failure string
    char FailString[256];
    vsprintf(FailString, FailText, args);
    
    // Release args list
    va_end(args);
    
    // Copy the given string
    char buffer[256];
    sprintf(buffer, "> Assertion in file %s:%d\n> Assertion message: \"%s\"\n", CleanFileName, LineNumber, FailString);
    
    // If windows... Write this out in a message box...
    #ifdef _WIN32
        
        // Present the message, break if user presses yes
        MessageBox(NULL, buffer, "Magi3 - Assertion Error", MB_ICONERROR);
        
    // Else, in a UNIX-like system... Write this via command line.
    #else
        
        // Write out to command line
        printf("> Magi3 - Assertion Error\n");
        printf("%s", buffer);
        
        // Print out the stack trace if this is GCC / glibc
        printf("> Dumping stack:\n\n");
        
        // Get the current backtrace
        void* backtrace_data[32];
        int backtrace_count;
        
        // Get and print
        backtrace_count = backtrace(backtrace_data, 32);
        backtrace_symbols_fd(backtrace_data, backtrace_count, STDOUT_FILENO);
        
    #endif
    
    // Exit the application
    exit(0);
}

// Static: it is an internal function
static void __UtilAssert(const char* FileName, int LineNumber, const char* FailText, ...)
{
    // Get multi-args list
    va_list args;
    va_start(args, FailText);
    
    // Force an assertion failure
    __UtilAssert(FileName, LineNumber, false, FailText, args);
    
    // Release args list
    va_end(args);
}

void UtilInit()
{
    // Initialize GLEW which is used for OpenGL 1.2+ Win32 support
    #ifdef _WIN32
        GLenum err = glewInit();
        UtilAssert(err == GLEW_OK, "Unable to initialize Glew: %s", glewGetErrorString(err));
    #endif
}

void UtilSleep(float SleepTime)
{
    #ifdef _WIN32
        Sleep(1000.0f * SleepTime); // Windows milli-second sleeps
    #else
        usleep(useconds_t(1000000.0f * SleepTime)); // Unix micro-second sleep
    #endif
}

UtilRand::UtilRand(unsigned int seed)
{
    // Save the seed
    this->seed = seed;
}

UtilRand::UtilRand(const char* seed)
{
    // If given a null - set to no seed
    if(seed == NULL)
    {
        this->seed = 0;
        return;
    }
    
    // Generate a seed based off of the Jenkins-one-at-a-time-hash
    unsigned int hash = 0;
    for(unsigned int i = 0; i < strlen(seed); i++)
    {
        hash += seed[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    // Save the hash index as the seed
    this->seed = hash;
}

unsigned int UtilRand::Rand()
{
    // Note: we don't do the mod since we just work between 0 and UINT_MAX
    seed = (a * seed + c);
    return seed;
}
