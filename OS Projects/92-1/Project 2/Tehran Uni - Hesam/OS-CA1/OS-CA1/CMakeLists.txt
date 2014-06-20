cmake_minimum_required(VERSION 2.6)
project(OS-CA1)
add_executable(server Server/Server.c Common/Objects/DataObjects.c Common/Utilities/FileUtil.c Common/Utilities/IOUtil.c Common/Utilities/StringUtil.c)
add_executable(client Client/Client.c  Common/Utilities/FileUtil.c Common/Utilities/IOUtil.c Common/Utilities/StringUtil.c)
