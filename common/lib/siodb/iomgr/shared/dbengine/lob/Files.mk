# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	dbengine/lob/BinaryValueBlobStream.cpp  \
	dbengine/lob/BlobStream.cpp  \
	dbengine/lob/BlobWrapperClobStream.cpp  \
	dbengine/lob/ClobStream.cpp  \
	dbengine/lob/ClobWrapperBlobStream.cpp  \
	dbengine/lob/StringClobStream.cpp

CXX_HDR+= \
	dbengine/lob/BinaryValueBlobStream.h  \
	dbengine/lob/BlobStream.h  \
	dbengine/lob/BlobWrapperClobStream.h  \
	dbengine/lob/ClobStream.h  \
	dbengine/lob/ClobWrapperBlobStream.h  \
	dbengine/lob/LobStream.h  \
	dbengine/lob/StringClobStream.h
