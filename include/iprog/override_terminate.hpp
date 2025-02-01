//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2025 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef terminateIprogsThreads

#ifdef USE_IPROGRAMS_TERMINATE

// Use my home grown implementation.
extern "C" void Terminate(const char*, ...);

#ifdef _MSC_VER
#define terminateIprogsThreads(blame) Terminate("Error in iprogsthreads at %s:%d, function %s", __FILE__, __LINE__, __func__)
#else
#define terminateIprogsThreads(blame) Terminate("Error in iprogsthreads at %s:%d, function %s", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

#else

#define terminateIprogsThreads(blame) std::terminate()

#endif

#endif

