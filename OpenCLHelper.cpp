// Copyright Hugh Perkins 2013,2014 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include <cstring>
#include <stdexcept>
using namespace std;

#include "OpenCLHelper.h"

#include "CLArrayFloat.h"
#include "CLArrayInt.h"
#include "CLIntWrapper.h"
#include "CLFloatWrapperConst.h"
#include "CLWrapper.h"
#include "CLKernel.h"

CLArrayFloat *OpenCLHelper::arrayFloat(int N ) {
    return new CLArrayFloat( N, this );
}

CLArrayInt *OpenCLHelper::arrayInt(int N ) {
    return new CLArrayInt( N, this );
}

CLIntWrapper *OpenCLHelper::wrap(int N, int *source ) {
    return new CLIntWrapper( N, source, this );
}

CLFloatWrapper *OpenCLHelper::wrap(int N, float *source ) {
    return new CLFloatWrapper( N, source, this );
}

CLFloatWrapperConst *OpenCLHelper::wrap(int N, float const*source ) {
    return new CLFloatWrapperConst( N, source, this );
}

CLKernel *OpenCLHelper::buildKernel( string kernelfilepath, string kernelname ) {
    return buildKernel( kernelfilepath, kernelname, "" );
}

CLKernel *OpenCLHelper::buildKernel( string kernelfilepath, string kernelname, string options ) {
    std::string path = kernelfilepath.c_str();
    std::string source = getFileContents(path);
    return buildKernelFromString( source, kernelname, options, kernelfilepath );
}

CLKernel *OpenCLHelper::buildKernelFromString( string source, string kernelname, string options, string sourcefilename ) {
    size_t src_size = 0;
    const char *source_char = source.c_str();
    src_size = strlen( source_char );
//    cl_program program = new cl_program();
    cl_program program = clCreateProgramWithSource(*context, 1, &source_char, &src_size, &error);
    checkError(error);

//    error = clBuildProgram(program, 1, &device, "-cl-opt-disable", NULL, NULL);
//    std::cout << "options: [" << options.c_str() << "]" << std::endl;
    error = clBuildProgram(program, 1, &device, options.c_str(), NULL, NULL);

    char* build_log;
    size_t log_size;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    checkError(error);
    build_log = new char[log_size+1];
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
    checkError(error);
    build_log[log_size] = '\0';
    string buildLogMessage = "";
    if( log_size > 2 ) {
        buildLogMessage = "build log: " + sourcefilename + "\n" + build_log;
        cout << buildLogMessage << endl;
    }
    delete[] build_log;
    checkError(error);

    cl_kernel kernel = clCreateKernel(program, kernelname.c_str(), &error);
    switch( error ) {
        case CL_SUCCESS:
            break;
        case -46:
            throw std::runtime_error( "Invalid kernel name, code -46, kernel " + kernelname + "\n" + buildLogMessage );
            break;
        default:
            throw std::runtime_error( "Something went wrong with clCreateKernel, code " + toString( error ) + "\n" + buildLogMessage );
            break;
    }
    checkError(error);
//    clReleaseProgram(program);
    return new CLKernel(this, program, kernel);
}

