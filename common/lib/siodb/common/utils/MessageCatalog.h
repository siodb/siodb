// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "MessageSeverity.h"

// STL headers
#include <string>
#include <unordered_map>

// Boost headers
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

namespace siodb::utils {

/** Single message in the message catalog */
class Message {
public:
    /**
     * Initializes object of class Message.
     * @param id Message ID.
     * @param severity Message severity class.
     * @param text Message text.
     */
    Message(int id, MessageSeverity severity, std::string&& text, std::uint64_t sourceLineNo)
        : m_id(id)
        , m_severity(severity)
        , m_text(std::move(text))
        , m_sourceLineNo(sourceLineNo)
    {
    }

    /**
     * Returns message ID.
     * @return Message ID.
     */
    int getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns message severity class.
     * @return Message severity class.
     */
    MessageSeverity getSeverity() const noexcept
    {
        return m_severity;
    }

    /**
     * Returns message text.
     * @return Message text.
     */
    const std::string& getText() const noexcept
    {
        return m_text;
    }

    /**
     * Returns message source line number.
     * @return Message source line number.
     */
    std::uint64_t getSourceLineNo() const noexcept
    {
        return m_sourceLineNo;
    }

private:
    /** Message ID */
    const int m_id;

    /** Message severity */
    const MessageSeverity m_severity;

    /** Message text */
    const std::string m_text;

    /** Message source line */
    const std::uint64_t m_sourceLineNo;
};

/** In-memory message catalog */
class MessageCatalog {
public:
    /**
     * Initializes object of class MessageCatalog.
     * @param messageCatalogFilePath Path of the message catalog file.
     */
    explicit MessageCatalog(const std::string& messageCatalogFilePath);

    /**
     * Looks up message with certain ID.
     * @param messageId Message ID.
     * @return Message object if found nullptr if not found.
     */
    template<class MessageId>
    const Message* find(MessageId messageId) const noexcept
    {
        const auto it = m_messages.find(static_cast<int>(messageId));
        return it == m_messages.cend() ? nullptr : it->second.get();
    }

    /**
     * Initializes default MessageCatalog.
     * @param messageCatalogFilePath Path of the message catalog file.
     */
    static void initDefaultCatalog(const std::string& messageCatalogFilePath);

    /**
     * Returns default message catalog instance.
     * @return default message catalog instance.
     * @throw std::runtime_error if default message catalog instance is not initialized.
     */
    static const MessageCatalog& getDefaultCatalog();

private:
    /** Initializes severity classes map */
    static void initializeSeverities();

private:
    /** Severity type */
    using SeverityMap = std::unordered_map<std::string, MessageSeverity>;

private:
    /** Message map */
    std::unordered_map<int, std::shared_ptr<Message>> m_messages;

    /** Severity classes map */
    static boost::atomic_shared_ptr<SeverityMap> m_severities;

    /** Default message catalog */
    static std::unique_ptr<MessageCatalog> m_defaultCatalog;

    /** Maximum allowed severity class name length */
    static constexpr const std::size_t kMaxSeverityNameLength = 16;
};

/** Thrown to indicate message catalog parsing issue */
class MessageCatalogParseError : public std::runtime_error {
public:
    /**
     * Initializes object of class MessageCatalogParseError.
     * @param what Explanatory message.
     */
    explicit MessageCatalogParseError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class MessageCatalogParseError.
     * @param what Explanatory message.
     */
    explicit MessageCatalogParseError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb::utils
