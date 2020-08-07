// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

namespace siodb::iomgr::dbengine {

Variant Variant::operator|(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() | other.getInt8();
                case VariantType::kUInt8: return getInt8() | other.getUInt8();
                case VariantType::kInt16: return getInt8() | other.getInt16();
                case VariantType::kUInt16: return getInt8() | other.getUInt16();
                case VariantType::kInt32: return getInt8() | other.getInt32();
                case VariantType::kUInt32: return getInt8() | other.getUInt32();
                case VariantType::kInt64: return getInt8() | other.getInt64();
                case VariantType::kUInt64: return getInt8() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() | other.getInt8();
                case VariantType::kUInt8: return getUInt8() | other.getUInt8();
                case VariantType::kInt16: return getUInt8() | other.getInt16();
                case VariantType::kUInt16: return getUInt8() | other.getUInt16();
                case VariantType::kInt32: return getUInt8() | other.getInt32();
                case VariantType::kUInt32: return getUInt8() | other.getUInt32();
                case VariantType::kInt64: return getUInt8() | other.getInt64();
                case VariantType::kUInt64: return getUInt8() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() | other.getInt8();
                case VariantType::kUInt8: return getInt16() | other.getUInt8();
                case VariantType::kInt16: return getInt16() | other.getInt16();
                case VariantType::kUInt16: return getInt16() | other.getUInt16();
                case VariantType::kInt32: return getInt16() | other.getInt32();
                case VariantType::kUInt32: return getInt16() | other.getUInt32();
                case VariantType::kInt64: return getInt16() | other.getInt64();
                case VariantType::kUInt64: return getInt16() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() | other.getInt8();
                case VariantType::kUInt8: return getUInt16() | other.getUInt8();
                case VariantType::kInt16: return getUInt16() | other.getInt16();
                case VariantType::kUInt16: return getUInt16() | other.getUInt16();
                case VariantType::kInt32: return getUInt16() | other.getInt32();
                case VariantType::kUInt32: return getUInt16() | other.getUInt32();
                case VariantType::kInt64: return getUInt16() | other.getInt64();
                case VariantType::kUInt64: return getUInt16() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() | other.getInt8();
                case VariantType::kUInt8: return getInt32() | other.getUInt8();
                case VariantType::kInt16: return getInt32() | other.getInt16();
                case VariantType::kUInt16: return getInt32() | other.getUInt16();
                case VariantType::kInt32: return getInt32() | other.getInt32();
                case VariantType::kUInt32: return getInt32() | other.getUInt32();
                case VariantType::kInt64: return getInt32() | other.getInt64();
                case VariantType::kUInt64: return getInt32() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() | other.getInt8();
                case VariantType::kUInt8: return getUInt32() | other.getUInt8();
                case VariantType::kInt16: return getUInt32() | other.getInt16();
                case VariantType::kUInt16: return getUInt32() | other.getUInt16();
                case VariantType::kInt32: return getUInt32() | other.getInt32();
                case VariantType::kUInt32: return getUInt32() | other.getUInt32();
                case VariantType::kInt64: return getUInt32() | other.getInt64();
                case VariantType::kUInt64: return getUInt32() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() | other.getInt8();
                case VariantType::kUInt8: return getInt64() | other.getUInt8();
                case VariantType::kInt16: return getInt64() | other.getInt16();
                case VariantType::kUInt16: return getInt64() | other.getUInt16();
                case VariantType::kInt32: return getInt64() | other.getInt32();
                case VariantType::kUInt32: return getInt64() | other.getUInt32();
                case VariantType::kInt64: return getInt64() | other.getInt64();
                case VariantType::kUInt64: return getInt64() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() | other.getInt8();
                case VariantType::kUInt8: return getUInt64() | other.getUInt8();
                case VariantType::kInt16: return getUInt64() | other.getInt16();
                case VariantType::kUInt16: return getUInt64() | other.getUInt16();
                case VariantType::kInt32: return getUInt64() | other.getInt32();
                case VariantType::kUInt32: return getUInt64() | other.getUInt32();
                case VariantType::kInt64: return getUInt64() | other.getInt64();
                case VariantType::kUInt64: return getUInt64() | other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for | operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for | operation");
        }
    }
}

Variant Variant::operator&(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() & other.getInt8();
                case VariantType::kUInt8: return getInt8() & other.getUInt8();
                case VariantType::kInt16: return getInt8() & other.getInt16();
                case VariantType::kUInt16: return getInt8() & other.getUInt16();
                case VariantType::kInt32: return getInt8() & other.getInt32();
                case VariantType::kUInt32: return getInt8() & other.getUInt32();
                case VariantType::kInt64: return getInt8() & other.getInt64();
                case VariantType::kUInt64: return getInt8() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() & other.getInt8();
                case VariantType::kUInt8: return getUInt8() & other.getUInt8();
                case VariantType::kInt16: return getUInt8() & other.getInt16();
                case VariantType::kUInt16: return getUInt8() & other.getUInt16();
                case VariantType::kInt32: return getUInt8() & other.getInt32();
                case VariantType::kUInt32: return getUInt8() & other.getUInt32();
                case VariantType::kInt64: return getUInt8() & other.getInt64();
                case VariantType::kUInt64: return getUInt8() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() & other.getInt8();
                case VariantType::kUInt8: return getInt16() & other.getUInt8();
                case VariantType::kInt16: return getInt16() & other.getInt16();
                case VariantType::kUInt16: return getInt16() & other.getUInt16();
                case VariantType::kInt32: return getInt16() & other.getInt32();
                case VariantType::kUInt32: return getInt16() & other.getUInt32();
                case VariantType::kInt64: return getInt16() & other.getInt64();
                case VariantType::kUInt64: return getInt16() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() & other.getInt8();
                case VariantType::kUInt8: return getUInt16() & other.getUInt8();
                case VariantType::kInt16: return getUInt16() & other.getInt16();
                case VariantType::kUInt16: return getUInt16() & other.getUInt16();
                case VariantType::kInt32: return getUInt16() & other.getInt32();
                case VariantType::kUInt32: return getUInt16() & other.getUInt32();
                case VariantType::kInt64: return getUInt16() & other.getInt64();
                case VariantType::kUInt64: return getUInt16() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() & other.getInt8();
                case VariantType::kUInt8: return getInt32() & other.getUInt8();
                case VariantType::kInt16: return getInt32() & other.getInt16();
                case VariantType::kUInt16: return getInt32() & other.getUInt16();
                case VariantType::kInt32: return getInt32() & other.getInt32();
                case VariantType::kUInt32: return getInt32() & other.getUInt32();
                case VariantType::kInt64: return getInt32() & other.getInt64();
                case VariantType::kUInt64: return getInt32() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() & other.getInt8();
                case VariantType::kUInt8: return getUInt32() & other.getUInt8();
                case VariantType::kInt16: return getUInt32() & other.getInt16();
                case VariantType::kUInt16: return getUInt32() & other.getUInt16();
                case VariantType::kInt32: return getUInt32() & other.getInt32();
                case VariantType::kUInt32: return getUInt32() & other.getUInt32();
                case VariantType::kInt64: return getUInt32() & other.getInt64();
                case VariantType::kUInt64: return getUInt32() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() & other.getInt8();
                case VariantType::kUInt8: return getInt64() & other.getUInt8();
                case VariantType::kInt16: return getInt64() & other.getInt16();
                case VariantType::kUInt16: return getInt64() & other.getUInt16();
                case VariantType::kInt32: return getInt64() & other.getInt32();
                case VariantType::kUInt32: return getInt64() & other.getUInt32();
                case VariantType::kInt64: return getInt64() & other.getInt64();
                case VariantType::kUInt64: return getInt64() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() & other.getInt8();
                case VariantType::kUInt8: return getUInt64() & other.getUInt8();
                case VariantType::kInt16: return getUInt64() & other.getInt16();
                case VariantType::kUInt16: return getUInt64() & other.getUInt16();
                case VariantType::kInt32: return getUInt64() & other.getInt32();
                case VariantType::kUInt32: return getUInt64() & other.getUInt32();
                case VariantType::kInt64: return getUInt64() & other.getInt64();
                case VariantType::kUInt64: return getUInt64() & other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for & operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for & operation");
        }
    }
}

Variant Variant::operator^(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() ^ other.getInt8();
                case VariantType::kUInt8: return getInt8() ^ other.getUInt8();
                case VariantType::kInt16: return getInt8() ^ other.getInt16();
                case VariantType::kUInt16: return getInt8() ^ other.getUInt16();
                case VariantType::kInt32: return getInt8() ^ other.getInt32();
                case VariantType::kUInt32: return getInt8() ^ other.getUInt32();
                case VariantType::kInt64: return getInt8() ^ other.getInt64();
                case VariantType::kUInt64: return getInt8() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() ^ other.getInt8();
                case VariantType::kUInt8: return getUInt8() ^ other.getUInt8();
                case VariantType::kInt16: return getUInt8() ^ other.getInt16();
                case VariantType::kUInt16: return getUInt8() ^ other.getUInt16();
                case VariantType::kInt32: return getUInt8() ^ other.getInt32();
                case VariantType::kUInt32: return getUInt8() ^ other.getUInt32();
                case VariantType::kInt64: return getUInt8() ^ other.getInt64();
                case VariantType::kUInt64: return getUInt8() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() ^ other.getInt8();
                case VariantType::kUInt8: return getInt16() ^ other.getUInt8();
                case VariantType::kInt16: return getInt16() ^ other.getInt16();
                case VariantType::kUInt16: return getInt16() ^ other.getUInt16();
                case VariantType::kInt32: return getInt16() ^ other.getInt32();
                case VariantType::kUInt32: return getInt16() ^ other.getUInt32();
                case VariantType::kInt64: return getInt16() ^ other.getInt64();
                case VariantType::kUInt64: return getInt16() ^ other.getUInt64();

                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() ^ other.getInt8();
                case VariantType::kUInt8: return getUInt16() ^ other.getUInt8();
                case VariantType::kInt16: return getUInt16() ^ other.getInt16();
                case VariantType::kUInt16: return getUInt16() ^ other.getUInt16();
                case VariantType::kInt32: return getUInt16() ^ other.getInt32();
                case VariantType::kUInt32: return getUInt16() ^ other.getUInt32();
                case VariantType::kInt64: return getUInt16() ^ other.getInt64();
                case VariantType::kUInt64: return getUInt16() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() ^ other.getInt8();
                case VariantType::kUInt8: return getInt32() ^ other.getUInt8();
                case VariantType::kInt16: return getInt32() ^ other.getInt16();
                case VariantType::kUInt16: return getInt32() ^ other.getUInt16();
                case VariantType::kInt32: return getInt32() ^ other.getInt32();
                case VariantType::kUInt32: return getInt32() ^ other.getUInt32();
                case VariantType::kInt64: return getInt32() ^ other.getInt64();
                case VariantType::kUInt64: return getInt32() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() ^ other.getInt8();
                case VariantType::kUInt8: return getUInt32() ^ other.getUInt8();
                case VariantType::kInt16: return getUInt32() ^ other.getInt16();
                case VariantType::kUInt16: return getUInt32() ^ other.getUInt16();
                case VariantType::kInt32: return getUInt32() ^ other.getInt32();
                case VariantType::kUInt32: return getUInt32() ^ other.getUInt32();
                case VariantType::kInt64: return getUInt32() ^ other.getInt64();
                case VariantType::kUInt64: return getUInt32() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() ^ other.getInt8();
                case VariantType::kUInt8: return getInt64() ^ other.getUInt8();
                case VariantType::kInt16: return getInt64() ^ other.getInt16();
                case VariantType::kUInt16: return getInt64() ^ other.getUInt16();
                case VariantType::kInt32: return getInt64() ^ other.getInt32();
                case VariantType::kUInt32: return getInt64() ^ other.getUInt32();
                case VariantType::kInt64: return getInt64() ^ other.getInt64();
                case VariantType::kUInt64: return getInt64() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() ^ other.getInt8();
                case VariantType::kUInt8: return getUInt64() ^ other.getUInt8();
                case VariantType::kInt16: return getUInt64() ^ other.getInt16();
                case VariantType::kUInt16: return getUInt64() ^ other.getUInt16();
                case VariantType::kInt32: return getUInt64() ^ other.getInt32();
                case VariantType::kUInt32: return getUInt64() ^ other.getUInt32();
                case VariantType::kInt64: return getUInt64() ^ other.getInt64();
                case VariantType::kUInt64: return getUInt64() ^ other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for ^ operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for ^ operation");
        }
    }
}

Variant Variant::operator<<(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() << other.getInt8();
                case VariantType::kUInt8: return getInt8() << other.getUInt8();
                case VariantType::kInt16: return getInt8() << other.getInt16();
                case VariantType::kUInt16: return getInt8() << other.getUInt16();
                case VariantType::kInt32: return getInt8() << other.getInt32();
                case VariantType::kUInt32: return getInt8() << other.getUInt32();
                case VariantType::kInt64: return getInt8() << other.getInt64();
                case VariantType::kUInt64: return getInt8() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() << other.getInt8();
                case VariantType::kUInt8: return getUInt8() << other.getUInt8();
                case VariantType::kInt16: return getUInt8() << other.getInt16();
                case VariantType::kUInt16: return getUInt8() << other.getUInt16();
                case VariantType::kInt32: return getUInt8() << other.getInt32();
                case VariantType::kUInt32: return getUInt8() << other.getUInt32();
                case VariantType::kInt64: return getUInt8() << other.getInt64();
                case VariantType::kUInt64: return getUInt8() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() << other.getInt8();
                case VariantType::kUInt8: return getInt16() << other.getUInt8();
                case VariantType::kInt16: return getInt16() << other.getInt16();
                case VariantType::kUInt16: return getInt16() << other.getUInt16();
                case VariantType::kInt32: return getInt16() << other.getInt32();
                case VariantType::kUInt32: return getInt16() << other.getUInt32();
                case VariantType::kInt64: return getInt16() << other.getInt64();
                case VariantType::kUInt64: return getInt16() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() << other.getInt8();
                case VariantType::kUInt8: return getUInt16() << other.getUInt8();
                case VariantType::kInt16: return getUInt16() << other.getInt16();
                case VariantType::kUInt16: return getUInt16() << other.getUInt16();
                case VariantType::kInt32: return getUInt16() << other.getInt32();
                case VariantType::kUInt32: return getUInt16() << other.getUInt32();
                case VariantType::kInt64: return getUInt16() << other.getInt64();
                case VariantType::kUInt64: return getUInt16() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() << other.getInt8();
                case VariantType::kUInt8: return getInt32() << other.getUInt8();
                case VariantType::kInt16: return getInt32() << other.getInt16();
                case VariantType::kUInt16: return getInt32() << other.getUInt16();
                case VariantType::kInt32: return getInt32() << other.getInt32();
                case VariantType::kUInt32: return getInt32() << other.getUInt32();
                case VariantType::kInt64: return getInt32() << other.getInt64();
                case VariantType::kUInt64: return getInt32() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() << other.getInt8();
                case VariantType::kUInt8: return getUInt32() << other.getUInt8();
                case VariantType::kInt16: return getUInt32() << other.getInt16();
                case VariantType::kUInt16: return getUInt32() << other.getUInt16();
                case VariantType::kInt32: return getUInt32() << other.getInt32();
                case VariantType::kUInt32: return getUInt32() << other.getUInt32();
                case VariantType::kInt64: return getUInt32() << other.getInt64();
                case VariantType::kUInt64: return getUInt32() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() << other.getInt8();
                case VariantType::kUInt8: return getInt64() << other.getUInt8();
                case VariantType::kInt16: return getInt64() << other.getInt16();
                case VariantType::kUInt16: return getInt64() << other.getUInt16();
                case VariantType::kInt32: return getInt64() << other.getInt32();
                case VariantType::kUInt32: return getInt64() << other.getUInt32();
                case VariantType::kInt64: return getInt64() << other.getInt64();
                case VariantType::kUInt64: return getInt64() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() << other.getInt8();
                case VariantType::kUInt8: return getUInt64() << other.getUInt8();
                case VariantType::kInt16: return getUInt64() << other.getInt16();
                case VariantType::kUInt16: return getUInt64() << other.getUInt16();
                case VariantType::kInt32: return getUInt64() << other.getInt32();
                case VariantType::kUInt32: return getUInt64() << other.getUInt32();
                case VariantType::kInt64: return getUInt64() << other.getInt64();
                case VariantType::kUInt64: return getUInt64() << other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for << operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for << operation");
        }
    }
}

Variant Variant::operator>>(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() >> other.getInt8();
                case VariantType::kUInt8: return getInt8() >> other.getUInt8();
                case VariantType::kInt16: return getInt8() >> other.getInt16();
                case VariantType::kUInt16: return getInt8() >> other.getUInt16();
                case VariantType::kInt32: return getInt8() >> other.getInt32();
                case VariantType::kUInt32: return getInt8() >> other.getUInt32();
                case VariantType::kInt64: return getInt8() >> other.getInt64();
                case VariantType::kUInt64: return getInt8() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() >> other.getInt8();
                case VariantType::kUInt8: return getUInt8() >> other.getUInt8();
                case VariantType::kInt16: return getUInt8() >> other.getInt16();
                case VariantType::kUInt16: return getUInt8() >> other.getUInt16();
                case VariantType::kInt32: return getUInt8() >> other.getInt32();
                case VariantType::kUInt32: return getUInt8() >> other.getUInt32();
                case VariantType::kInt64: return getUInt8() >> other.getInt64();
                case VariantType::kUInt64: return getUInt8() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() >> other.getInt8();
                case VariantType::kUInt8: return getInt16() >> other.getUInt8();
                case VariantType::kInt16: return getInt16() >> other.getInt16();
                case VariantType::kUInt16: return getInt16() >> other.getUInt16();
                case VariantType::kInt32: return getInt16() >> other.getInt32();
                case VariantType::kUInt32: return getInt16() >> other.getUInt32();
                case VariantType::kInt64: return getInt16() >> other.getInt64();
                case VariantType::kUInt64: return getInt16() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() >> other.getInt8();
                case VariantType::kUInt8: return getUInt16() >> other.getUInt8();
                case VariantType::kInt16: return getUInt16() >> other.getInt16();
                case VariantType::kUInt16: return getUInt16() >> other.getUInt16();
                case VariantType::kInt32: return getUInt16() >> other.getInt32();
                case VariantType::kUInt32: return getUInt16() >> other.getUInt32();
                case VariantType::kInt64: return getUInt16() >> other.getInt64();
                case VariantType::kUInt64: return getUInt16() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() >> other.getInt8();
                case VariantType::kUInt8: return getInt32() >> other.getUInt8();
                case VariantType::kInt16: return getInt32() >> other.getInt16();
                case VariantType::kUInt16: return getInt32() >> other.getUInt16();
                case VariantType::kInt32: return getInt32() >> other.getInt32();
                case VariantType::kUInt32: return getInt32() >> other.getUInt32();
                case VariantType::kInt64: return getInt32() >> other.getInt64();
                case VariantType::kUInt64: return getInt32() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() >> other.getInt8();
                case VariantType::kUInt8: return getUInt32() >> other.getUInt8();
                case VariantType::kInt16: return getUInt32() >> other.getInt16();
                case VariantType::kUInt16: return getUInt32() >> other.getUInt16();
                case VariantType::kInt32: return getUInt32() >> other.getInt32();
                case VariantType::kUInt32: return getUInt32() >> other.getUInt32();
                case VariantType::kInt64: return getUInt32() >> other.getInt64();
                case VariantType::kUInt64: return getUInt32() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() >> other.getInt8();
                case VariantType::kUInt8: return getInt64() >> other.getUInt8();
                case VariantType::kInt16: return getInt64() >> other.getInt16();
                case VariantType::kUInt16: return getInt64() >> other.getUInt16();
                case VariantType::kInt32: return getInt64() >> other.getInt32();
                case VariantType::kUInt32: return getInt64() >> other.getUInt32();
                case VariantType::kInt64: return getInt64() >> other.getInt64();
                case VariantType::kUInt64: return getInt64() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() >> other.getInt8();
                case VariantType::kUInt8: return getUInt64() >> other.getUInt8();
                case VariantType::kInt16: return getUInt64() >> other.getInt16();
                case VariantType::kUInt16: return getUInt64() >> other.getUInt16();
                case VariantType::kInt32: return getUInt64() >> other.getInt32();
                case VariantType::kUInt32: return getUInt64() >> other.getUInt32();
                case VariantType::kInt64: return getUInt64() >> other.getInt64();
                case VariantType::kUInt64: return getUInt64() >> other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for >> operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for >> operation");
        }
    }
}

}  // namespace siodb::iomgr::dbengine
