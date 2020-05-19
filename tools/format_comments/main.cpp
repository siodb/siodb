// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <fstream>
#include <iostream>
#include <list>
#include <string>

// Boost headers
#include <boost/algorithm/string/trim.hpp>

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Not enough arguments.\nUsage " << argv[0] << " FILES ..." << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];
        std::list<std::string> lines;

        {
            std::ifstream ifs(filename);
            if (!ifs.is_open()) {
                std::cerr << "Can't open file " << filename << " for reading." << std::endl;
                return 2;
            }
            std::cout << filename << std::endl;
            std::string s;
            while (std::getline(ifs, s)) {
                lines.push_back(std::move(s));
            }
            if (!ifs.eof()) {
                std::cerr << "Can't read file " << filename << std::endl;
                return 2;
            }
        }

        bool inComment = false;
        std::size_t commentSize = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            auto s = boost::trim_copy(*it);
            if (inComment) {
                if (s == "*/") {
                    // end of comment
                    inComment = false;
                    if (commentSize == 1) {
                        auto itt = it--;
                        lines.erase(itt);
                        itt = it--;
                        std::string z = std::move(*it);
                        boost::trim_right(z);
                        lines.erase(it);
                        it = itt;
                        s = boost::trim_copy(*it);
                        if (!s.empty() && s[0] == '*') {
                            s = s.substr(1);
                            boost::trim(s);
                        }
                        if (!s.empty() && s[s.length() - 1] == '.') {
                            s = s.substr(0, s.length() - 1);
                            boost::trim(s);
                        }
                        s = z + ' ' + s + " */";
                        it->swap(s);
                    }
                } else {
                    ++commentSize;
                }
            } else if (s == "/**" || s == "/*") {
                inComment = true;
                commentSize = 0;
            }
        }

        {
            std::ofstream ofs(filename);
            if (!ofs.is_open()) {
                std::cerr << "Can't open file " << filename << " for writing." << std::endl;
                return 2;
            }
            for (const auto& line : lines)
                ofs << line << '\n';
            ofs << std::flush;
        }
    }
    return 0;
}
