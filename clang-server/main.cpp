/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/25.16:24:55 */

/*
 * Copyright (c) 2013-2017 yaruopooner [https://github.com/yaruopooner]
 *
 * This file is part of ac-clang.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/


#include "CommandLine.hpp"
#include "ClangServer.hpp"



/*================================================================================================*/
/*  Global Function Definitions Section                                                           */
/*================================================================================================*/


enum
{
    kStreamBuffer_UnitSize = 1 * 1024 * 1024, 
    kStreamBuffer_MinMB    = 1, 
    kStreamBuffer_MaxMB    = 5, 
};

enum 
{
    kOption_Help, 
    kOption_Version, 
    kOption_LogFile, 
    kOption_STDIN_BufferSize, 
    kOption_STDOUT_BufferSize, 
    kOption_InputData, 
    kOption_OutputData, 
};


namespace
{

std::string GetClangVersion( void )
{
    CXString            version_text  = clang_getClangVersion();
    const std::string   clang_version = clang_getCString( version_text );

    clang_disposeString( version_text );

    return clang_version;
}

}



int main( int _argc, char *_argv[] )
{
    // std::ios_base::sync_with_stdio( false );

    // parse options
    const std::string   server_version     = CLANG_SERVER_VERSION;
    const std::string   clang_version      = ::GetClangVersion();
    const std::string   generate           = CMAKE_GENERATOR "/" CMAKE_HOST_SYSTEM_PROCESSOR;
    std::string         logfile;
    std::string         input_data_type;
    std::string         output_data_type;
    size_t              stdin_buffer_size  = kStreamBuffer_MinMB;
    size_t              stdout_buffer_size = kStreamBuffer_MinMB;

    {
        CommandLine::Parser        declare_options;

        declare_options.AddOption( kOption_Help, "help", "h", "Display available options.", 
                                   CommandLine::IOptionDetail::kFlag_Once );
        declare_options.AddOption( kOption_Version, "version", "v", "Display current version.", 
                                   CommandLine::IOptionDetail::kFlag_Once );
        // declare_options.AddOption< std::string >( kOption_LogFile, "logfile", "l", "Enable IPC records output.(for debug)",
        //                                           ( CommandLine::IOptionDetail::kFlag_Once | CommandLine::IOptionDetail::kFlag_HasValue ), "file path" );
        declare_options.AddOption< uint32_t >( kOption_STDIN_BufferSize, "stdin-buffer-size", "sibs", "STDIN buffer size. <size> is 1 - 5 MB", 
                                               ( CommandLine::IOptionDetail::kFlag_Once | CommandLine::IOptionDetail::kFlag_HasValue ), "size", 
                                               CommandLine::RangeReader< uint32_t >( kStreamBuffer_MinMB, kStreamBuffer_MaxMB ) );
        declare_options.AddOption< uint32_t >( kOption_STDOUT_BufferSize, "stdout-buffer-size", "sobs", "STDOUT buffer size. <size> is 1 - 5 MB", 
                                               ( CommandLine::IOptionDetail::kFlag_Once | CommandLine::IOptionDetail::kFlag_HasValue ), "size", 
                                               CommandLine::RangeReader< uint32_t >( kStreamBuffer_MinMB, kStreamBuffer_MaxMB ) );
        declare_options.AddOption< std::string >( kOption_InputData, "input-data", "idata", "input data type. <type> is s-expression | json", 
                                                  ( CommandLine::IOptionDetail::kFlag_Once | CommandLine::IOptionDetail::kFlag_HasValue ), "type" );
        declare_options.AddOption< std::string >( kOption_OutputData, "output-data", "odata", "output data type. <type> is s-expression | json", 
                                                  ( CommandLine::IOptionDetail::kFlag_Once | CommandLine::IOptionDetail::kFlag_HasValue ), "type" );

        if ( declare_options.Parse( _argc, _argv ) )
        {
            for ( const auto& option_value : declare_options.GetOptionWithValueArray() )
            {
                switch ( option_value->GetId() )
                {
                    case    kOption_Help:
                        declare_options.PrintUsage( "clang-server [options] <values>" );
                        return 0;
                    case    kOption_Version:
                        std::cout << server_version << " (" << generate << ")" << std::endl;
                        std::cout << clang_version << std::endl;
                        return 0;
                    case    kOption_LogFile:
                        if ( option_value->IsValid() )
                        {
                            const std::string  result = declare_options.GetValue< std::string >( option_value );

                            logfile = result;
                        }
                        break;
                    case    kOption_STDIN_BufferSize:
                        if ( option_value->IsValid() )
                        {
                            const uint32_t  result = declare_options.GetValue< uint32_t >( option_value );

                            stdin_buffer_size = result;
                        }
                        break;
                    case    kOption_STDOUT_BufferSize:
                        if ( option_value->IsValid() )
                        {
                            const uint32_t  result = declare_options.GetValue< uint32_t >( option_value );

                            stdout_buffer_size = result;
                        }
                        break;
                    case    kOption_InputData:
                        if ( option_value->IsValid() )
                        {
                            const std::string  result = declare_options.GetValue< std::string >( option_value );

                            input_data_type = result;
                        }
                        break;
                    case    kOption_OutputData:
                        if ( option_value->IsValid() )
                        {
                            const std::string  result = declare_options.GetValue< std::string >( option_value );

                            output_data_type = result;
                        }
                        break;
                }
            }
            declare_options.PrintWarnings();
        }
        else
        {
            declare_options.PrintErrors();
            return 1;
        }
    }


    // stream buffer expand
    // std::shared_ptr< char >   stdin_buffer( new char[ kStreamBuffer_UnitSize ], std::default_delete< char[] >() );
    // std::shared_ptr< char >   stdout_buffer( new char[ kStreamBuffer_UnitSize ], std::default_delete< char[] >() );

    stdin_buffer_size  *= kStreamBuffer_UnitSize;
    stdout_buffer_size *= kStreamBuffer_UnitSize;

#if 0
    // std::string     object( "(:db-name \"!drive_d!trico!engine!anzu!src!trico!x64!Release!2013!x86_amd64\" :project-file \"d:/trico/engine/anzu/src/trico.vcxproj\" :platform \"x64\" :configuration \"Release\" :version \"2013\" :toolset \"x86_amd64\")" );
    // std::string     object( "(:db-name \"!drive_d!trico!engine!anzu!src!trico!x64!Release!2013!x86_amd64\" (:project-file \"d:/trico/engine/anzu/src/trico.vcxproj\" [:platform \"x64\"]) :configuration \"Release\" :version 2013.1 :toolset \"x86_amd64\")" );
    std::string     text( "(:db-name \"!drive_d!trico!engine!anzu!src!trico!x64!Release!2013!x86_amd64\" () :configuration \"Release\" :version 2013.1 :toolset \"x86_amd64\")" );
    
    Lisp::Node::Parser       parser;
    Lisp::Node::Object       node_object;
    
    parser.Parse( text.c_str(), node_object );

    // node_object.Clear();

    parser.Parse( text.c_str(), node_object );

#endif


    // server instance
    ClangFlagConverters         flag_converter;
    ClangServer::Specification  initial_spec( stdin_buffer_size, stdout_buffer_size, 0, 0, logfile );
    ClangServer                 server( initial_spec );

    server.ParseCommand();


    return 0;
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
