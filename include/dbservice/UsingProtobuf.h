#pragma once
//------------------------------------------------------------------------------
/**
    @class CUsingDependency
    
    (C) 2016 n.lee
*/
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>	// include Windows.h to avoid "GetMessage" conflict between windows and protobuf
#endif

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/reflection.h>


/*EOF*/