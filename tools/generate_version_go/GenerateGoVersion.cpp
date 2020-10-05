// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "GenerateGoVersion.h"

// Project headers
#include "Version.h"

// Common project headers
#include <siodb/common/stl_wrap/filesystem_wrapper.h>

// CRT headers
#include <cstdlib>
#include <cstring>

// STL headers
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

// Boost headers
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

int main(int argc, char** argv)
{
    std::cout << "Siodb Version.go Generation Utility v." << GGV_VERSION
              << ".\nCopyright (C) Siodb GmbH, " << GGV_COPYRIGHT_YEARS
              << ". All rights reserved.\n"
              << "Compiled on " << __DATE__ << ' ' << __TIME__ << std::endl;

    std::string inputFilePath, outputFilePath;

    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "-i") == 0) {
            inputFilePath = argv[++i];
        } else if (std::strcmp(argv[i], "-o") == 0) {
            outputFilePath = argv[++i];
        } else {
            std::cerr << "Unrecognized option: " << argv[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (inputFilePath.empty()) {
        std::cerr << "Missing input file path." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    if (outputFilePath.empty()) {
        std::cerr << "Missing output file path." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    std::unordered_map<std::string, std::string> defines;

    std::ifstream ifs(inputFilePath);
    if (!ifs.is_open()) {
        std::cerr << "Can't open input file " << inputFilePath << std::endl;
        return 2;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (boost::algorithm::starts_with(line, "#define ")) {
            std::vector<std::string> tokens;
            boost::split(tokens, line, boost::is_space());
            tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                                 [](const auto& s) noexcept { return s.empty(); }),
                    tokens.end());
            if (tokens.size() < 3) continue;
            defines.emplace(std::move(tokens.at(1)), std::move(tokens.at(2)));
        }
    }

    if (!ifs.eof()) {
        std::cerr << "Can't read input file " << inputFilePath << std::endl;
        return 2;
    }
    ifs.close();

    const auto tmpFileInfo = makeTemporaryFile();
    if (std::get<1>(tmpFileInfo) < 0) {
        std::cerr << "Can't open temporary file: " << std::strerror(std::get<2>(tmpFileInfo))
                  << std::endl;
        return 2;
    }
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink> ofs(
            std::get<1>(tmpFileInfo), boost::iostreams::close_handle);
    if (!ofs.is_open()) {
        std::cerr << "Can't open temporary file" << std::endl;
        return 2;
    }

    const std::string majorVersionKey("SIODB_VERSION_MAJOR");
    const std::string minorVersionKey("SIODB_VERSION_MINOR");
    const std::string patchVersionKey("SIODB_VERSION_PATCH");
    const std::string copyrightYearsKey("SIODB_COPYRIGHT_YEARS");

    ofs << R"text(// THIS FILE IS GENERATED AUTOMATICALLY. PLEASE DO NOT EDIT.
// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.)text";
    ofs << "\npackage main\n\n";
    ofs << "var " << majorVersionKey << " = " << defines[majorVersionKey] << '\n';
    ofs << "var " << minorVersionKey << " = " << defines[minorVersionKey] << '\n';
    ofs << "var " << patchVersionKey << " = " << defines[patchVersionKey] << '\n';
    ofs << "var " << copyrightYearsKey << " = " << defines[copyrightYearsKey] << '\n';

    ofs << std::flush;
    ofs.close();

    if (::rename(std::get<0>(tmpFileInfo).c_str(), outputFilePath.c_str())) {
        const int errorCode = errno;
        std::cerr << "Can't rename temporary file " << std::get<0>(tmpFileInfo) << " into "
                  << outputFilePath << ": " << std::strerror(errorCode) << std::endl;
        return 2;
    }

    return 0;
}

void printUsage(const char* program)
{
    std::cerr << "Usage:\n" << program << " -i INPUT_FILE -o OUTPUT_FILE" << std::endl;
}

bool renameFile(const std::string& src, const std::string& to)
{
    boost::system::error_code ec;
    fs::rename(src, to, ec);
    if (!ec) return true;

    if (ec.value() == EXDEV) {
        fs::copy_file(src, to, ec);
        if (!ec) {
            fs::remove(src, ec);
            if (!ec) return true;
        }
    }

    std::cerr << "Can't rename temporary file " << src << " into " << to << ": " << ec.message()
              << std::endl;
    return false;
}

std::tuple<std::string, int, int> makeTemporaryFile()
{
    std::string tmpFilePath;
    const char* tmpDir = ::getenv("TMP");
    if (tmpDir && *tmpDir) {
        tmpFilePath = tmpDir;
        if (tmpFilePath.back() != '/') tmpFilePath += '/';
    } else
        tmpFilePath = "/tmp/";
    tmpFilePath += "siodb_generate_version_go-XXXXXX";
    const int fd = ::mkstemp(tmpFilePath.data());
    const int errorCode = errno;
    return std::make_tuple(std::move(tmpFilePath), fd, errorCode);
}
