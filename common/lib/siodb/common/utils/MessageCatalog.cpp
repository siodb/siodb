// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "MessageCatalog.h"

// Project headers
#include "../stl_ext/sstream_ext.h"

// STL headers
#include <fstream>

// Boost headers
#include <boost/algorithm/string/trim.hpp>
#include <boost/smart_ptr/make_shared.hpp>

namespace siodb::utils {

boost::atomic_shared_ptr<MessageCatalog::SeverityMap> MessageCatalog::m_severities;
std::unique_ptr<MessageCatalog> MessageCatalog::m_defaultCatalog;

MessageCatalog::MessageCatalog(const std::string& messageCatalogFilePath)
{
    // Fill severities map
    initializeSeverities();

    // Open message catalog file
    std::ifstream ifs(messageCatalogFilePath);
    if (!ifs.is_open()) {
        throw std::runtime_error(
                stdext::concat("Can't open message catalog file ", messageCatalogFilePath));
    }

    // Read message catalog file line by line
    std::uint64_t lineNo = 0;
    std::string line;
    while (std::getline(ifs, line)) {
        ++lineNo;

        // Skip empty and comment lines.
        boost::trim(line);
        if (line.empty() || line[0] == '#') continue;

        try {
            // Find separator after message ID
            const auto sep1Pos = line.find_first_of(',');
            if (sep1Pos == std::string::npos)
                throw MessageCatalogParseError("Can't find message ID separator");

            // Find separator after message severity class
            const auto sep2Pos = line.find_first_of(',', sep1Pos + 1);
            if (sep1Pos == std::string::npos)
                throw MessageCatalogParseError("Can't find message severity class separator");

            // Parse message ID
            int id;
            try {
                auto s = line.substr(0, sep1Pos);
                boost::trim(s);
                std::size_t pos = 0;
                id = std::stoi(s, &pos);
                if (pos != s.length()) throw std::invalid_argument("invalid number");
            } catch (std::exception& ex) {
                throw MessageCatalogParseError(stdext::concat("Invalid message ID: ", ex.what()));
            }

            // Parse message severity class
            MessageSeverity severity;
            {
                auto severities = m_severities.load();
                if (!severities) {
                    throw MessageCatalogParseError(
                            "Message severity class mapping is not availbale");
                }

                // Do not include leading and traliing ','
                auto s = line.substr(sep1Pos + 1, sep2Pos - (sep1Pos + 1));
                boost::trim(s);
                if (s.length() > kMaxSeverityNameLength)
                    throw MessageCatalogParseError("Message severity class name is too long");
                const auto it = severities->find(s);

                if (it == severities->end()) {
                    throw MessageCatalogParseError(
                            stdext::concat("Unknown message severity class '", s, '\''));
                }
                severity = it->second;
            }

            // Parse message text
            std::string text;
            {
                text = line.substr(sep2Pos + 1);
                boost::trim(text);
                if (text.empty()) throw MessageCatalogParseError("Message text is empty");
            }

            // Check message ID uniqueness
            const auto it = m_messages.find(id);
            if (it != m_messages.end()) {
                throw MessageCatalogParseError(stdext::concat("Duplicate message ID ", id,
                        " (previous one was defined at the line ", it->second->getSourceLineNo(),
                        ')'));
            }

            // Add message to map
            auto message = std::make_shared<Message>(id, severity, std::move(text), lineNo);
            m_messages.emplace(id, message);
        } catch (MessageCatalogParseError& ex) {
            throw std::runtime_error(
                    stdext::concat(messageCatalogFilePath, '(', lineNo, "): ", ex.what()));
        }
    }
}

void MessageCatalog::initDefaultCatalog(const std::string& messageCatalogFilePath)
{
    if (m_defaultCatalog)
        throw std::runtime_error("Default message catalog is already initialized");
    m_defaultCatalog = std::make_unique<MessageCatalog>(messageCatalogFilePath);
}

const MessageCatalog& MessageCatalog::getDefaultCatalog()
{
    if (!m_defaultCatalog) throw std::runtime_error("Default message catalog is not initialized");
    return *m_defaultCatalog;
}

void MessageCatalog::initializeSeverities()
{
    if (m_severities.load()) return;
    auto severities = boost::make_shared<SeverityMap>();
    severities->emplace("Debug", MessageSeverity::kDebug);
    severities->emplace("Trace", MessageSeverity::kTrace);
    severities->emplace("Info", MessageSeverity::kInfo);
    severities->emplace("Warning", MessageSeverity::kWarning);
    severities->emplace("Error", MessageSeverity::kError);
    severities->emplace("Fatal", MessageSeverity::kFatal);
    m_severities.store(severities);
}

}  // namespace siodb::utils
