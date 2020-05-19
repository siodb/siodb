// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <map>
#include <string>

// Boost header
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

/** Message record */
struct Message {
    Message()
        : m_id(-1)
    {
    }

    /** Message identifier */
    std::int64_t m_id;

    /** Message named constant */
    std::string m_symbol;

    /** Message severity name */
    std::string m_severity;

    /** Message text */
    std::string m_text;
};

/** Index tag */
struct ByIdTag {
};

/** Index tag */
struct BySymbolTag {
};

/** List of messages */
typedef boost::multi_index::multi_index_container<Message,
        boost::multi_index::indexed_by<
                boost::multi_index::ordered_unique<boost::multi_index::tag<ByIdTag>,
                        boost::multi_index::member<Message, decltype(Message::m_id),
                                &Message::m_id>>,
                boost::multi_index::ordered_unique<boost::multi_index::tag<BySymbolTag>,
                        boost::multi_index::member<Message, decltype(Message::m_symbol),
                                &Message::m_symbol>>>>
        MessageContainer;
