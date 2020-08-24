// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DatabaseError.h"

// Common project headers
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/utils/MessageCatalog.h>

// Boost headers
#include <boost/format.hpp>
#include <boost/format/exceptions.hpp>

namespace siodb::iomgr::dbengine {

template<class MessageId, class... Args>
CompoundDatabaseError::ErrorRecord makeDatabaseError(MessageId messageId, Args&&... args)
{
    auto catalog = utils::MessageCatalog::getDefaultCatalog();
    CompoundDatabaseError::ErrorRecord error;
    const auto message = catalog.find(messageId);
    if (!message) {
        error.m_errorCode = 1;
        std::ostringstream os;
        os << "Message not found: id=" << static_cast<int>(messageId);
        error.m_message = os.str();
        return error;
    }

    error.m_errorCode = static_cast<int>(messageId);
    try {
        error.m_message =
                ((boost::format(message->getText())) % ... % std::forward<Args>(args)).str();
    } catch (boost::io::bad_format_string& ex) {
        error.m_errorCode = 5;
        std::ostringstream os;
        os << "Bad message format in the message #" << static_cast<int>(messageId) << ": "
           << message->getText();
        error.m_message = os.str();
    } catch (boost::io::too_few_args& ex) {
        error.m_errorCode = 6;
        std::ostringstream os;
        os << "Too few parameters for the message #" << static_cast<int>(messageId) << ": expected "
           << ex.get_expected() << ", but got " << ex.get_cur();
        error.m_message = os.str();
    } catch (boost::io::too_many_args& ex) {
        error.m_errorCode = 7;
        std::ostringstream os;
        os << "Too many parameters for the message #" << static_cast<int>(messageId)
           << ": expected " << ex.get_expected() << ", but got " << ex.get_cur();
        error.m_message = os.str();
    } catch (boost::io::out_of_range& ex) {
        error.m_errorCode = 8;
        std::ostringstream os;
        os << "Parameter index is out of range for the message #" << static_cast<int>(messageId)
           << ": index is " << ex.get_index() << ", but range is (" << ex.get_beg() << ", "
           << ex.get_end() << ")";
        error.m_message = os.str();
    }
    return error;
}

/**
 * Throws database exception with message formatted using given arguments.
 * Looks up message in a given message catalog.
 * @param catalog Message catalog
 * @param messageId Message ID.
 * @param args Message formatting arguments.
 */
template<class MessageId, class... Args>
[[noreturn]] void throwDatabaseError(
        const utils::MessageCatalog& catalog, MessageId messageId, Args&&... args)
{
    const auto message = catalog.find(messageId);
    if (!message) {
        throw UserVisibleDatabaseError(1, stdext::string_builder() << "Message not found: id="
                                                                   << static_cast<int>(messageId));
    }

    std::string s;
    try {
        s = ((boost::format(message->getText())) % ... % std::forward<Args>(args)).str();
    } catch (boost::io::bad_format_string& ex) {
        std::ostringstream os;
        os << "Bad message format in the message #" << static_cast<int>(messageId) << ": "
           << message->getText();
        throw InternalDatabaseError(5, os.str());
    } catch (boost::io::too_few_args& ex) {
        std::ostringstream os;
        os << "Too few parameters for the message #" << static_cast<int>(messageId) << ": expected "
           << ex.get_expected() << ", but got " << ex.get_cur();
        throw InternalDatabaseError(6, os.str());
    } catch (boost::io::too_many_args& ex) {
        std::ostringstream os;
        os << "Too many parameters for the message #" << static_cast<int>(messageId)
           << ": expected " << ex.get_expected() << ", but got " << ex.get_cur();
        throw InternalDatabaseError(7, os.str());
    } catch (boost::io::out_of_range& ex) {
        std::ostringstream os;
        os << "Parameter index is out of range for the message #" << static_cast<int>(messageId)
           << ": index is " << ex.get_index() << ", but range is (" << ex.get_beg() << ", "
           << ex.get_end() << ")";
        throw InternalDatabaseError(8, os.str());
    }

    // See more on this here https://stackoverflow.com/a/25859856/1540501
    if (DatabaseError::isMessageIdInRange(messageId, DatabaseError::kIOErrorCodeRange))
        throw DatabaseIOError(messageId, s);
    else if (DatabaseError::isMessageIdInRange(messageId, DatabaseError::kInternalErrorCodeRange))
        throw InternalDatabaseError(messageId, s);
    else
        throw UserVisibleDatabaseError(messageId, s);
}

/**
 * Throws database exception with message formatted using given arguments.
 * Looks up message in the default message catalog.
 * @param messageId Message ID.
 * @param args Message formatting arguments.
 */
template<class MessageId, class... Args>
[[noreturn]] inline void throwDatabaseError(MessageId messageId, Args&&... args)
{
    throwDatabaseError(
            utils::MessageCatalog::getDefaultCatalog(), messageId, std::forward<Args>(args)...);
}

}  // namespace siodb::iomgr::dbengine
