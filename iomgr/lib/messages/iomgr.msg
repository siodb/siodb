# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Siodb IO Manager Message File

##########################################
# PUBLIC MESSAGES
##########################################

##########################################
# SPECIAL COMMON MESSAGES
##########################################
PMSG:1 Error MessageNotFound  Message not found: %1%
PMSG:2 Error SyntaxError  Syntax error: %1%
PMSG:3 Error IOError  I/O error, message ID %1%. Exact details are available in the Siodb logs.
PMSG:4 Error InternalError  \
    Internal error, message ID %1%. Exact details are available in the Siodb logs.
PMSG:5 Error MessageFormatError  Bad message format: %1%
PMSG:6 Error TooFewParameters    Too few parameters for the message #%1%: expected %2% read %3%
PMSG:7 Error TooManyParameters   Too many parameters for the message #%1%: expected %2% read %3%
PMSG:8 Error ParametersIndexOutOfRange  \
    Parameter index is out of range for the message #%1%: index is %2%, but range is (%3%, %4%)
PMSG:10 Error Feature is not implemented yet
PMSG:11 Error Authentication error: %1%
PMSG:12 Error REST request parse error: %1%

##########################################
# Severe malfunctions
##########################################
ID 1000
PMSG Error SystemDatabaseNotFound     System database not found
PMSG Error InstanceResourceExhausted  Instance has exhausted resource '%1%'
PMSG Error DatabaseResourceExhausted  Database '%1%' has exhausted resource '%1%'
PMSG Error UserTridExhausted          User TRID exhausted for the table '%1%'.'%2%'
PMSG Error SystemTridExhausted        System TRID exhausted for the table '%1%'.'%2%'

##########################################
# SQL Errors
##########################################
ID 2000

# OBJECT DOESN'T EXIST
PMSG Error InstanceDoesNotExist   Instance doesn't exist
PMSG Error DatabaseDoesNotExist   Database '%1%' doesn't exist
PMSG Error TableDoesNotExist      Table '%1%'.'%2%' doesn't exist
PMSG Error TableDoesNotExist2     Table '%1%'.[%2%] doesn't exist
PMSG Error ColumnDoesNotExist     Column '%1%'.'%2%'.'%3%' doesn't exist
PMSG Error ColumnDoesNotExist2    Column '%1%'.'%2%'.[%3%] doesn't exist
PMSG Error ColumnDoesNotExist3    Column '%1%'.[%2%] doesn't exist
PMSG Error ColumnSetDoesNotExist  Column set '%1%'.%2% doesn't exist
PMSG Error ColumnDefinitionDoesNotExist      Column definition '%1%'.'%2%'.'%3%'.%4% doesn't exist
PMSG Error ColumnDefinitionDoesNotExist2     Column definition '%1%'.[%2%] doesn't exist
PMSG Error ConstraintDefinitionDoesNotExist  Constraint definition '%1%'.[%2%] doesn't exist
PMSG Error ConstraintDoesNotExist2           Constraint '%1%'.[%2%] doesn't exist
PMSG Error IndexDoesNotExist   Index '%1%'.'%2%' doesn't exist
PMSG Error IndexDoesNotExist2  Index '%1%'.[%2%] doesn't exist
PMSG Error UserDoesNotExist    User '%1%' doesn't exist
PMSG Error UserIdDoesNotExist  User #%1% doesn't exist
PMSG Error UserAccessKeyDoesNotExist  User access key '%1%'.'%2%' doesn't exist
PMSG Error UserAccessKeyIdDoesNotExist  User access key #%1% doesn't exist
PMSG Error UserTokenDoesNotExist  User token '%1%'.'%2%' doesn't exist
PMSG Error UserTokenIdDoesNotExist  User token #%1% doesn't exist

# OBJECT ALREADY EXISTS
PMSG Error InstanceAlreadyExists  Instance already exists
PMSG Error DatabaseAlreadyExists  Database '%1%' already exists
PMSG Error TableAlreadyExists     Table '%1%'.'%2%' already exists
PMSG Error ColumnAlreadyExists    Column %1%'.'%2%'.'%3%' already exists
PMSG Error ColumnDataBlockAlreadyExists  Column data block '%1%'.'%2%'.'%3%'.'%4%' already exists
PMSG Error IndexAlreadyExists          Index '%1%'.'%2%' already exists
PMSG Error ConstraintAlreadyExists     Constraint '%1%'.'%2%' already exists
PMSG Error UserAlreadyExists           User '%1%' already exists
PMSG Error UserAccessKeyAlreadyExists  User access key '%1%'.'%2%' already exists
PMSG Error UserTokenAlreadyExists      User token '%1%'.'%2%' already exists

# INVALID NAMES
PMSG Error InvalidDatabaseName  Invalid database name '%1%'
PMSG Error InvalidTableName     Invalid table name '%1%'
PMSG Error InvalidTableAlias    Invalid table alias '%1%'
PMSG Error InvalidColumnName    Invalid column name '%1%'
PMSG Error InvalidColumnNameInTableColumn      Invalid column name '%1%'.'%2%'.'%3%'
PMSG Error InvalidColumnDataTypeInTableColumn  \
    Invalid column data type %1% in the column '%2%'.'%3%'.'%4%'
PMSG Error InvalidColumnAlias     Invalid column '%1%' alias '%2%'
PMSG Error InvalidIndexName       Invalid index name '%1%'
PMSG Error InvalidConstraintName  Invalid constraint name '%1%'
PMSG Error InvalidConstraintNameInTable  Invalid constraint name '%1%'.'%2%'.'%3%'
PMSG Error InvalidUserName           Invalid user name '%1%'
PMSG Error InvalidUserAccessKeyName  Invalid user access key name '%1%'
PMSG Error InvalidUserTokenName  Invalid user token name '%1%'

# INVALID OBJECT TYPE
PMSG Error InvalidConstraintType    Constraint '%1%'.'%2%'.'%3%' has invalid constraint type
PMSG Error InvalidValueType        Invalid value type '%1%'
PMSG Error InvalidColumnType       Column '%1%'.'%2' type is invalid
PMSG Error InvalidTransactionType  Transaction type '%1%' is invalid

# DUPLICATES
PMSG Error SelectDuplicateColumnAlias      SELECT duplicate column alias '%1%'
PMSG Error SelectDuplicateColumnName       SELECT duplicate column name '%1%'
PMSG Error InsertDuplicateColumnName       INSERT duplicate column name '%1%'
PMSG Error UpdateDuplicateColumnName       UPDATE duplicate column name '%1%'
PMSG Error CreateTableDuplicateColumnName  CREATE TABLE duplicate column name '%1%'
PMSG Error CreateTableDuplicateConstraintName  CREATE TABLE duplicate constraint name '%1%'
PMSG Error CreateTableDuplicateColumnConstraintType  \
    CREATE TABLE duplicate constraint type '%1%' in the column '%2%'

# WHERE
PMSG Error LikePatternTypeIsWrong  LIKE pattern type '%1%' is not supported
PMSG Error LikeValueTypeIsWrong    LIKE value type '%1%' is not supported
PMSG Error InvalidWhereCondition   WHERE condition is invalid: %1%

# SELECT
PMSG Error CannotUseAllColumnsAlias  Column alias '%1%' to "*" is prohibited
PMSG Error UnexpectedNullValue  Unexpected NULL value in '%1%'.'%2%'.'%3%', TRID %4%
PMSG Error SelectWithoutTables  Select without tables is not supported yet
PMSG Error TableNotSpecified    SELECT: Table %1% is not specified
PMSG Error ColumnIsUnknown      SELECT: Column '%1%'.'%2%' is unknown
PMSG Error LimitValueTypeNotInteger LIMIT value type isn't integer
PMSG Error LimitValueIsNegative LIMIT value is negative
PMSG Error OffsetValueTypeNotInteger OFFSET value type isn't integer
PMSG Error OffsetValueIsNegative OFFSET value is negative

# INSERT
PMSG Error ValuesListNotMatchColumns  \
    Length of '%1%'.'%2%' VALUES list doesn't match to the number of columns \
    %3%, row %4% has %5% values
PMSG Error ValuesListLengthsNotSame  \
    '%1%'.'%2%' VALUES lists must all have the same length %3%, but row %4% has %5% values
PMSG Error TableColumnIndexOutOfRange  \
    Table column index '%3%' is out of range for the '%1%'.'%2%'
PMSG Error CannotInsertToSystemTable     Insert into system table '%1%'.'%2%' is not allowed
PMSG Error CannotInsertIntoMasterColumn  Can't insert value into TRID, it is assigned automatically
PMSG Error CannotInsertNullValue         Can't insert NULL value to column '%1%'.'%2%'.'%3%'
PMSG Error CannotInsertDuplicateTrid  \
    Can't insert duplicate value %4% into master column '%1%'.'%2%'.'%3%'

# UPDATE
PMSG Error ValuesListIsEmpty             VALUES list type is empty 
PMSG Error ColumnsListIsEmpty            '%1%'.'%2%' columns list type is empty 
PMSG Error UpdateInvalidValueExpression  UPDATE SET expression is invalid: %1%
PMSG Error CannotUpdateSystemTable       Update system table '%1%'.'%2%' is not allowed
PMSG Error CannotUpdateMasterColumn      Can't update master column in the table '%1%'.'%2%'
PMSG Error ValueAndColumnTypeMismatch  \
    Column '%1%' data type '%2%' isn't compatible with value type '%3%'
PMSG Error UpdateValuesDoesNotFitToPositions  \
    '%1%'.'%2%' values count %3% not equal to positions count %4%
PMSG Error UpdateValuesCountGreaterThanAddresses  \
    '%1%'.'%2%' values count %3% greater than column data addresses %4%
PMSG Error UpdateTableIsNotEqualToColumnTable  \
    Table '%1%' is not equal to '%2%'.'%3%'

# DELETE
PMSG Error CannotDeleteFromSystemTable  Deleting from system table '%1%'.'%2%' is not allowed

# DROP USER
PMSG Error CannotDropSuperUser  Can't drop superuser

# ALTER USER
PMSG Error CannotChangeSuperUserState  Can't change superuser state
PMSG Error InvalidUserAccessKey  \
    Invalid key in attempt to create user access key '%1%'.'%2%'
PMSG Error UserAccessKeyIsTooLong  \
    User access key '%1%'.'%2%' size %3% is larger than maximum allowed size %4%
PMSG Error CannotDeleteLastSuperUserAccessKey      Can't delete last superuser access key '%1%'
PMSG Error CannotDeactivateLastSuperUserAccessKey  Can't deactivate last superuser access key '%1%'
PMSG Error AlterUserAccessKeyFailed  Alter user access key '%1%'.'%2%' failed
PMSG Error InvalidUserTokenValue2    Invalid user token
PMSG Error DuplicateUserToken        Duplicate user token

# CREATE DATABASE
PMSG Error CannotCreateTemporaryDatabase  Temporary databases are not supported yet
PMSG Error CipherIdTypeIsNotString        CIPHER_ID must be string
PMSG Error CipherKeySeedIsNotString       CIPHER_KEY_SEED must be string

# DROP DATABASE
PMSG Error CannotDropSystemDatabase   Can't drop system database
PMSG Error CannotDropUsedDatabase     Can't drop database '%1%' which is currently in use
PMSG Error CannotDropCurrentDatabase  Can't drop current database '%1%'

# RENAME DATABASE
PMSG Error CannotRenameSystemDatabase   Can't rename system database
PMSG Error CannotRenameUsedDatabase     Can't rename database '%1%' which is currently in use
PMSG Error CannotRenameCurrentDatabase  Can't rename current database '%1%'

# CREATE TABLE
PMSG Error TableTypeNotSupported  Table type %1% is not supported yet
PMSG Error CannotCreateUserTablesInSystemDatabase  Can't create user table in the system database

# ENCRYPTION
PMSG Error CipherUnknown     Cipher '%1%' is unknown
PMSG Error InvalidCipherKey  Invalid key for the cipher '%1%'

# AUTHENTICATION
PMSG Error UserAccessDenied        User '%1%' access denied
PMSG Error SessionDoesNotExist     Session '%1%' doesn't exist
PMSG Error UserTokenCheckFailed    User token '%1%'.'%2%' check failed
PMSG Error UserTokenMismatch       User '%1%' token mismatch
PMSG Error InvalidUserTokenValue   Invalid user token

# UNSUPPORTED FEATURES
PMSG Error ConstraintNotSupported  \
    Constraint type #%4% is not supported (constraint definition '%1%'.%2% (%3%.%2%)
PMSG Error ConstraintNotSupported2  Constraint type #%4% is not supported

# INVALID ATTRIBUTES
PMSG Error InvalidNextUserTrid Invalid next user TRID %3% for the table '%1%'.'%2%'

# OBJECT LIMITS
PMSG Error TooManyUsers      Too many user accounts
PMSG Error TooManyDatabases  Too many databases
PMSG Error TooManyTables     Too many tables for the database %1%

##########################################
# REST ERRORS
##########################################
ID 3000
PMSG Error JsonPayloadParsingError JSON payload parse error: %1%
PMSG Error TooManyValuesInPayload  \
    Too many values (%1%) in the row %2% of the JSON payload, expecting at most %3% while \
    posting data to the table '%4%'.'%5%'

##########################################
# INTERNAL MESSAGES
##########################################

##########################################
# I/O Errors
##########################################

ID -2
STEP -1

MSG Fatal InstanceNameMismatch  \
    Instance name %1% in the instance data folder doesn't match to the current instance name '%2%'
MSG Fatal CannotOpenInstanceMetadata     Can't open instance metadata file '%1%': (%2%) %3%
MSG Fatal CannotLoadInstanceMetadata     Can't load instance metadata: (%1%) %2%
MSG Error CannotSaveInstanceMetadata     Can't save instance metadata: (%1%) %2%
MSG Fatal CannotOpenMasterEncryptionKey  Can't open master encryption key file '%1%': (%2%) %3%
MSG Fatal CannotStatMasterEncryptionKey  Can't stat master encryption key file '%1%': (%2%) %3%
MSG Fatal InvalidMasterEncryptionKey  \
    Master encryption key file '%1%' is invalid. Expected size %2%, but actual size is %3%
MSG Fatal CannotReadMasterEncryptionKey  \
    Can't read master encryption key file '%1%': (%2%) %3% (length %4% read %5%)
MSG Fatal CannotStatSuperUserKey         Can't stat super user access key file '%1%'
MSG Fatal CannotOpenSuperUserKey         Can't open super user access key file '%1%'
MSG Fatal SuperUserAccessKeyIsTooLong    \
    Super user initial access key length %1% is larger than maximum %2%

MSG Warning CannotRemoveDatabaseDataDirectory  \
    Can't remove data directory for the database '%1%' (%2%): (%3%) %4%

MSG Error CannotCreateDatabaseMetadataFile  \
    Can't create metadata file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotOpenDatabaseMetadataFile  \
    Can't open metadata file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotReadDatabaseMetadataFile  \
    Can't read from metadata file for the database '%1%' (%2%): (%3%) %4% (length %5% read %6%)
MSG Error CannotWriteDatabaseMetadataFile  \
    Can't write to metadata file for the database '%1%' (%2%): (%3%) %4% (length %5% written %6%)
MSG Error DatabaseMetadataFileCorrupted  \
    Metadata file '%1%' for the database '%2%' (%3%) is corrupted: %4%

MSG Error CannotCreateDatabaseCipherKeyFile  \
    Can't create cipher key file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotOpenDatabaseCipherKeyFile  \
    Can't open cipher key file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotReadDatabaseCipherKeyFile  \
    Can't read from cipher key file for the database '%1%' (%2%): (%3%) %4% (length %5% read %6%)
MSG Error CannotWriteDatabaseCipherKeyFile  \
    Can't write to cipher key file for the database '%1%' (%2%): (%3%) %4% (length %5% written %6%)
MSG Error DatabaseCipherKeyFileCorrupted  \
    Cipher key file '%1%' for the database '%2%' (%3%) is corrupted: %4%

MSG Error CannotCreateInstanceDataDir   Can't create instance data directory '%1%': (%2%) %3%
MSG Error CannotClearInstanceDataDir    Can't clear instance directory '%1%': (%2%) %3%
MSG Error InstanceDataDirIsNotDir       Instance directory path '%1%' is not a directory
MSG Error CannotCreateDatabaseDataDir \
    Can't create data directory '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotCreateTableDataDir  \
    Can't create data directory '%1%' for the table '%2%'.'%3%' (%4%.%5%): (%6%) %7%
MSG Error CannotCreateColumnDataDir \
    Can't create data directory '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotCreateIndexDataDir \
    Can't create data directory '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%

MSG Fatal CannotCreateInstanceInitializationFlagFile  \
    Can't create instance initialization success data file '%1%': %2%
MSG Fatal CannotOpenInstanceInitializationFlagFile  \
    Can't open instance initialization success data file '%1%': %2%
MSG Error CannotCreateDatabaseInitializationFlagFile  \
    Can't create initialization success data file '%1%' for the database %2% (%3%): %4%
MSG Error CannotCreateTableInitializationFlagFile  \
    Can't create initialization success data file '%1%' for the table '%2%'.'%3%' (%4%.%5%): %6%
MSG Error CannotCreateColumnInitializationFlagFile  \
    Can't create initialization success data file '%1%' for the column '%2%'.'%3%'.'%4%' \
    (%5%.%6%.%7%): %8%
MSG Error CannotCreateIndexInitializationFlagFile  \
    Can't create initialization success data file '%1%' for the column '%2%'.'%3%'.'%4%' \
    (%5%.%6%.%7%): %8%

MSG Error CannotCreateColumnDataBlockFile  \
    Can't create new data block file '%8%' for the column '%1%'.'%2%'.'%3%'.%4% \
    (%5%.%6%.%7%.%4%): %9%: (%10%) %11%
MSG Error CannotOpenColumnDataBlockFile  \
    Can't open data block file '%8%' for the column '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%): \
    (%10%) %11%
MSG Error CannotReadColumnDataBlockFile  Can't read data block file '%1%'.'%2%'.'%3%'.%4% \
    (%5%.%6%.%7%.%4%) offset %8% length %9%: (%10%) %11% (read %12%)
MSG Error CannotWriteColumnDataBlockFile  Can't write data block file '%1%'.'%2%'.'%3%'.%4% \
    (%5%.%6%.%7%.%4%) offset %8% length %9%: (%10%) %11% (written %12%)
MSG Error InvalidPositionForColumnDataBlock  \
    Attempt to set invalid position for the data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%): \
    offset %8%, while max allowed offset is %9%

MSG Error InvalidDataFileHeaderSize  \
    Invalid block data file header size '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error InvalidDataFileHeader  \
    Invalid block data header '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%), ver '%8%'

MSG Error ColumnDataBlockNotAvailable  \
    Column data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) is not available
MSG Error ColumnDataBlockConsistencyMismatch  \
    Column data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) consistency error: %8%
MSG Error ColumnDataBlockDoesNotExist  \
    Column data block '%1%'.'%2%'.'%3%'.%4% ('%5%'.'%6%'.'%7%'.%4%) doesn't exist

MSG Error CannotCreateBlockListDataFile  \
    Can't create block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): \
    (%8%) %9%
MSG Error CannotOpenBlockListDataFile   \
    Can't open block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): \
    (%8%) %9%
MSG Error CannotGetBlockListDataFileSize  \
    Can't get block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error InvalidBlockListDataFileSize  \
    Invalid block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %7%
MSG Error CannotReadBlockListDataFile  \
    %1%: Can't read block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %8% length %9%: (%10%) %11% (read %12%)
MSG Error CannotWriteBlockListDataFile  \
    %1%: Can't write block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %8% length %9%: (%10%) %11% (written %12%)

MSG Error CannotCreateNextBlockListDataFile  \
    Can't create next block list data file '%1%' for the column '%2%'.'%3%'.'%4%' \
    (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenNextBlockListDataFile  \
    Can't open next block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): \
    (%8%) %9%
MSG Error CannotGetNextBlockListDataFileSize  \
    Can't get next block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    (%7%) %8%
MSG Error InvalidNextBlockListDataFileSize  \
    Invalid next block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %7%
MSG Error CannotReadNextBlockListDataFile  \
    %1%: Can't read next block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %9% length %9%: (%10%) %11% (read %12%)
MSG Error CannotWriteNextBlockListDataFile  \
    %1%: Can't write next block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %8% length %9%: (%10%) %11% (written %12%)

MSG Error CannotCreateIndexFile  \
    Can't create data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotLinkIndexFile  \
    Can't link to filesystem data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): \
    (%8%) %9%
MSG Error CannotRenameIndexFile  \
    Can't rename data file '%1%' into '%2%' for the index '%3%'.'%4%'.'%5%' (%6%.%7%.%8%): \
    (%9%) %10%
MSG Error CannotOpenIndexFile  \
    Can't open data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotStatIndexFile  \
    Can't stat data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadIndexFile  \
    Can't read from data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %8% length %9%: (%10%) %11% (read %12%)
MSG Error CannotWriteIndexFile  \
    Can't write to data file '%1%' for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) \
    offset %8% length %9%: (%10%) %11% (written %12%)
MSG Error IndexFileCorrupted  Index file '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) corrupted: %7%
MSG Error CannotFindIndexRoot  Can't find root node for the index '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error CannotFindIndexNode  \
    Can't find root node for the index '%1%'.'%2%'.'%3%'.'%4%' (%5%.%6%.%7%.%4%)
MSG Error IndexNodeCorrupted  Index node '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) corrupted

MSG Error MasterColumnRecordIndexCorrupted  \
    Master column record index on table '%1%'.'%2%' (%3%.%4%) corrupted. Defect code %5%

MSG Error CannotCreateMainIndexIdFile  \
    Can't create main index ID file '%1%' for column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenMainIndexIdFile  \
    Can't open main index ID file '%1%' for column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadMainIndexIdFile  \
    Can't read main index ID file for column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    (%7%) %8% (length %9% read %10%)
MSG Error CannotWriteMainIndexIdFile  \
    Can't write main index ID file for column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    (%7%) %8% (length %9% written %10%)

MSG Error LobReadFailed  LOB read error in column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)

MSG Error BlockRegistryDirNotExists  \
    Block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) doesn't exist
MSG Error BlockRegistryDirAlreadyExists  \
    Block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) already exists
MSG Error BlockRegistryDirIsNotDir  \
    Path of the block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)\
    is not a directory
MSG Error CannotCreateBlockRegistryDir  \
    Failed to create block registry directory for the column '%1%'.'%2%'.'%3%' \
    (%4%.%5%.%6%): (%7%) %8%

MSG Error CannotCreateSystemObjectsFile  \
    Can't create system objects file for the database '%1%' (%2%): %3%
MSG Error CannotOpenSystemObjectsFile  \
    Can't open system objects file for the database '%1%' (%2%): %3%
MSG Error CannotWriteSystemObjectsFile  \
    Can't write to the system objects file for the database '%1%' (%2%): %3%
MSG Error CannotReadSystemObjectsFile  \
    Can't read from the system objects file for the database '%1%' (%2%): %3% (length %4% read %5%)
MSG Error SystemObjectsFileCorrupted  \
    System objects file for the database '%1%' (%2%) is corrupted: %3%
MSG Error CannotMoveSystemObjectsFile  \
    Can't move system objects file for the database '%1%' (%2%): %3%
MSG Error CannotSerializeSystemObject  \
    Can't serialize system object for the database '%1%' (%2%): %3%
MSG Error CannotDeserializeSystemObject  \
    Can't deserialize system object for the database '%1%' (%2%): %3%

MSG Error BptiFlushNodeCacheFailed  \
    There were errors while flushing BPTI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) node cache to disk: %7%

MSG Error CannotCreateTridCountersFile  \
    Can't create TRID counter file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenTridCounterFile  \
    Can't open TRID counter file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadTridCounterFile  \
    Can't read from TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    (%7%) %8% (length %9% read %10%)
MSG Error CannotWriteTridCounterFile \
    Can't write to TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    (%7%) %8% (length %9% written %10%)
MSG Error CannotRenameTridCounterFile  \
    Can't rename TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error TridCounterFileCorrupted  \
    TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is corrupted

MSG Error OnlyConstantExpressionsAreAllowed  Only constant expressions are allowed in this context
MSG Error TableDoesNotExistInContext  Table '%1%'.'%2%' doesn't exist in request context
MSG Error ColumnDoesNotExistInContext  Column '%1%'.'%2%'.'%3%' doesn't exist in request context
MSG Error DatasetHasNoTables  Database '%1%' dataset has no tables

# OBJECT INITIALIZATION FILE DOESN'T EXIST
MSG Error DatabaseInitFileDoesNotExist  Database '%1%' initilaization file '%2%' doesn't exist
MSG Error TableInitFileDoesNotExist     Table '%1%'.'%2%' initilaization file '%3%' doesn't exist
MSG Error ColumnInitFileDoesNotExist  \
    Column '%1%'.'%2%'.'%3%' initilaization file '%4%' doesn't exist
MSG Error IndexInitFileDoesNotExist  \
    Index %1%'.'%2%'.'%3%' initilaization file '%4%' doesn't exist

# OBJECT FOLDER DOESN'T EXIST
MSG Error DatabaseDataFolderDoesNotExist  Database '%1%' data folder '%2%' doesn't exist
MSG Error TableDataFolderDoesNotExist     Table '%1%'.'%2%' data folder '%3%' doesn't exist
MSG Error ColumnDataFolderDoesNotExist    Column '%1%'.'%2%'.'%3%' data folder '%4%' doesn't exist
MSG Error IndexDataFolderDoesNotExist     Index %1%'.'%2%'.'%3%' data folder '%4%' doesn't exist

MSG Error kErrorDefaultValueDeserializationFailed  \
    Failed to deserialize default value for the constraint '%1%'.'%2%'.'%3%'.'%4%' \
    (%5%.%6%.%7%.%8%)

##########################################
# Internal Errors
##########################################

ID -1000
STEP -1

MSG Error UserCacheFull      User cache is full
MSG Error DatabaseCacheFull  Database cache is full
MSG Error TableCacheFull       Table cache of the database '%1%' is full
MSG Error ConstraintCacheFull  Constraint cache of the database '%1%' is full
MSG Error ConstraintDefinitionCacheFull  Constraint definition cache of the database '%1%' is full

MSG Error InvalidMasterColumnDataType  \
    Invalid master column data type for the table \'%1%'.'%2%' (%3%.%3%)
MSG Error InvalidMasterColumnRecordSize  \
    Invalid master column record size %6% in the table '%1%'.'%2%' column '%3%' (%4%.%5%.%6%) \
    at address %7%.%8%: %9%
MSG Error InvalidMasterColumnRecordColumnCount  \
    Invalid column count in master column record in the table '%1%'.'%2%' (%3%.%4%) \
    at address %5%.%6%: expecting %7%, but actually %8%

MSG Error NotMasterColumn  Column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is not a master column
MSG Error TooManyColumns   Too many columns in the table '%1%'.'%2%' (%3%.%4%)
MSG Error IncompatibleDataType1  \
    Can't store value of incompatible data type into column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    destination data type is %7%, source value type is %8%
MSG Error IncompatibleDataType2  \
    Can't store value of incompatible data type into column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): \
    column data type is %7%, source value type is %8%

MSG Error InvalidDataBlockPosition  \
    Invalid position %8% in the data block: '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error InvalidRollbackDataBlockPosition  \
    Invalid position %8% in the data block for rollback: '%1%'.'%2%'.'%3%'.%4% \
    (%5%.%6%.%7%.%4%), while current block is %9%
MSG Error InvalidCurrentDataBlock  \
    Invalid current data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error UnreachableRollbackDataBlockPosition  \
    Rollback data block: '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) is unreachable

MSG Error MissingTableColumns  Missing columns information for the table '%1%'.'%2%' (%3%.%4%)

MSG Error UliMissingLessValueWhenExpected  \
    ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) missing less value when it is expected to exist
MSG Error UliMissingGreaterValueWhenExpected  \
    ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) missing greater value when it is expected to exist
MSG Error UliMissingFileWhenExpected  \
    ULI '%1%'.'%2%'.'%3%' (%5%.%6%.%7%) missing file #%4% when it is expected to exist
MSG Error UliCorrupted  \
    ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) corrupted at key %7%
MSG Error UliInvalidIndexFileName  \
    ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) invalid index file name %7%

MSG Error InvalidLobChunkHeader  \
    Invalid LOB header at address '%7%'.'%8%' in the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %9%
MSG Error InvalidLobChunkHeaderAddress  \
    Invalid LOB header address '%7%'.'%8%' in the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %9%

MSG Error CannotGetCachedTable      Can't obtain cached table '%1%'.'%2%' (%3%.%4%)
MSG Error CannotGetSystemTableInfo  \
    Can't obtain information about system table '%1%'.'%2%' (%3%.%4%)
MSG Error CannotFindAvailableBlockRecord  \
    Can't find available block record for the block '%1%'.'%2%'.'%3%'.%4% (%4%.%5%.%6%.%4%)

MSG Error TableColumnRegistryCorrupted    Column registry corrupted in table '%1%'.'%2%'
MSG Error NumberOfValuesMistatchOnInsert  \
    Insert to the table '%1%'.'%2%': Number of values (%3%) doesn't match number of columns (%4%)
MSG Error TooManyColumnsToInsert  \
    Insert to the table '%1%'.'%2%': Too many columns provided (%3%), while maximum possible \
    number of columns is %4%. 
MSG Error AdjustedNumberOfValuesMistatchOnInsert  \
    Insert to the table '%1%'.'%2%': Adjusted number of values (%3%) doesn't match \
        number of columns (%4%)
MSG Error NumberOfNextBlocksMistatchOnRollback  \
    Rollback last insert in the table '%1%'.'%2%': Number of next block IDs (%3%) \
    doesn't match number of columns (%4%)
MSG Error TooManyColumnsToRollback  \
    Rollback last insert in the table '%1%'.'%2%': Too many columns provided (%3%), \
    while maximum possible number of columns is %4%. 

MSG Error InvalidColumnSetTable  \
    Column set #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidColumnTable  \
    Column #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidIndexTable  \
    Index #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidIndexColumnIndex  \
    Index column #%1% of index #%2% doesn't belong to the index '%3%'.'%4%'.'%5%' (%6%.%7%.%8%)
MSG Error InvalidColumnSetColumnColumnSet  \
    Column set column #%1% of column set #%2% doesn't belong to the column set \
    '%2%'.'%3%'.%4% (%5%.%6%.%4%)
MSG Error InvalidColumnDefinitionColumn \
    Column definition #%1% of column #%2% doesn't belong to the column '%2%'.'%3%'.'%4%' \
    (%5%.%6%.%7%)
MSG Error InvalidTableColumnDefinition  \
    Column definition #%1% of column #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidColumnDefinitionConstraintColumnDefinition  \
    Column definition constraint #%1% of column definition #%2% does not belong to the \
    column definition '%3%'.'%4%'.'%5%'.%6% (%7%.%8%.%9%.%6%)

MSG Error CannotModifyClosedColumnSet  \
    Can't modify closed column set '%1%'.'%2%'.%3% (%4%.%5%.%3%)
MSG Error CannotModifyClosedColumnDefinition  \
    Can't modify closed column set '%1%'.'%2%'.'%3%'.%4% (%6%.%7%.%4%)
MSG Error CannotGenerateSystemTridUsingNonMasterColumn  \
    Can't generate system TRID using non-master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error CannotGenerateUserTridUsingNonMasterColumn    \
    Can't generate user TRID using non-master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error ExplicitConstraintsForMasterColumnProhibited  \
    Explicit spcification of constraints for the master column '%1%'.'%2%'.'%3%' \
    (%4%.%5%.%6%) is prohibited

### Internal integrity checks ###

# Database
ID -100000
MSG Error CannotReleaseUnusedDatabase Can't release database '%1%' (%2%) which is marked as unused
MSG Error MissingSystemTable Missing system table '%1%'.'%2%' (%3%.'%4%')

# Table
ID -101000
MSG Error TableDoesNotBelongToDatabase  \
    %1%: table '%2%'.'%3%' (%4%.%5%) doesn't belong to the database '%6%' (%7%)
MSG Error TableConstraintTypeDoesNotMatch  \
    Constraint type #%1% doesn't match to required type #%2% in the table constraint \
    '%3%'.'%4%'.'%5%' ('%6%'.'%7%'.'%8%') (constraint definition #%9%)
MSG Error TableMissingColumnSet  Table '%1%'.'%2%' ('%3%'.'%4%') missing column set

# Column set
ID -102000
MSG Error ColumnSetStillOpenForModification  \
    Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) still open for modification
MSG Error ColumnSetAlreadyClosedForModification  \
    Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) is already closed for modification
MSG Error ColumnSetMissingColumns  Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) missing columns

# Column
ID -103000
MSG Error ColumnDoesNotBelongToTable  \
    %1%: column '%2%'.'%3%'.'%4%' (%4%.%5%.%6%) doesn't belong to the table '%6%'.'%7%' (%8%.%9%)
MSG Error ColumnConstraintTypeDoesNotMatch  \
    Constraint type #%1% doesn't match to required type #%2% in the column \
    constraint '%3%'.'%4%'.'%5%'.'%6%' ('%7%'.'%8%'.'%9%'.'%10%') (constraint definition #%11%)
MSG Error ColumnDoesNotBelongToColumnSet  \
    Column #%6% doesn't belong to the column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%)
MSG Error MasterColumnMainIndexAlreadyExists  \
    Main index of the master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) already exist

# Column definition 
ID -104000
MSG Error MissingColumnDefinitionsForColumn \
    Missing column definitions for the column [%1%].[%2%].[%3%]

# Constraint
ID -105000
MSG Error ColumnNotSpecifiedForColumnConstraint  \
    Column is not specified when creating column constraint '%1%' of type #%2% \
    in the table '%3%'.'%4%'(%5%.%6%)
MSG Error InvalidConstraintTable  Constraint '%2%' doesn't belong to the table '%2%'.'%3%'

# Index
ID -106000
MSG Error InvalidIndexColumns  Index '%1%'.'%2%'.'%3%' has empty column list

# User
ID -107000

# User access key
ID -108000
MSG Error InvalidUserForUserAccessKey Invalid user #%1% for the user access key #%2%
MSG Error DuplicateUserAccessKeyId    Duplicate user access key id %2% for the user '%1%'

# User token
ID -109000
MSG Error InvalidUserForUserToken      Invalid user #%1% for the user token #%2%
MSG Error CannotGenerateUserToken      Can't generate new token: %1%
MSG Error DuplicateUserTokenId         Duplicate user token id %2% for the user '%1%'
MSG Error InvalidUserTokenHashedValue  Invalid user token hashed value for the token '%1%'

# Invalid attributes
# ID -110000
MSG Error InvalidLastUserTrid Invalid last user TRID %5% for the table '%1%'.'%2%' (%3%.%4%)

# END
