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
MSG:1 Error MessageNotFound  Message not found: %1%
MSG:2 Error SyntaxError  Syntax error: %1%
MSG:3 Error IOError  I/O error, message ID %1%. Exact details are available in the Siodb logs.
MSG:4 Error InternalError  Internal error, message ID %1%. Exact details are available in the Siodb logs.
MSG:5 Error MessageFormatError  Bad message format: %1%
MSG:6 Error TooFewParameters    Too few parameters for the message #%1%: expected %2%, but got %3%
MSG:7 Error TooManyParameters   Too many parameters for the message #%1%: expected %2%, but got %3%
MSG:8 Error ParametersIndexOutOfRange   Parameter index is out of range for the message #%1%: index is %2%, but range is (%3%, %4%)
MSG:10 Error Feature is not implemented yet

##########################################
# Severe malfunctions
##########################################
ID 1000
MSG Error SystemDatabaseNotFound     System database not found
MSG Error InstanceResourceExhausted  Instance has exhausted resource '%1%'
MSG Error DatabaseResourceExhausted  Database '%1%' has exhausted resource '%1%'
MSG Error UserTridExhausted          User TRID exhausted for the table '%1%'.'%2%'
MSG Error SystemTridExhausted        System TRID exhausted for the table '%1%'.'%2%'

##########################################
# SQL Errors
##########################################
ID 2000

# OBJECT DOESN'T EXIST
MSG Error InstanceDoesNotExist   Instance doesn't exist
MSG Error DatabaseDoesNotExist   Database '%1%' doesn't exist
MSG Error TableDoesNotExist      Table '%1%'.'%2%' doesn't exist
MSG Error TableDoesNotExist2     Table '%1%'.[%2%] doesn't exist
MSG Error ColumnDoesNotExist     Column '%1%'.'%2%'.'%3%' doesn't exist
MSG Error ColumnDoesNotExist2    Column '%1%'.'%2%'.[%3%] doesn't exist
MSG Error ColumnDoesNotExist3    Column '%1%'.[%2%] doesn't exist
MSG Error ColumnSetDoesNotExist  Column set '%1%'.%2% doesn't exist
MSG Error ColumnDefinitionDoesNotExist      Column definition '%1%'.'%2%'.'%3%'.%4% doesn't exist
MSG Error ColumnDefinitionDoesNotExist2     Column definition '%1%'.[%2%] doesn't exist
MSG Error ConstraintDefinitionDoesNotExist  Constraint definition '%1%'.[%2%] doesn't exist
MSG Error ConstraintDoesNotExist2           Constraint '%1%'.[%2%] doesn't exist
MSG Error IndexDoesNotExist   Index '%1%'.'%2%' doesn't exist
MSG Error IndexDoesNotExist2  Index '%1%'.[%2%] doesn't exist
MSG Error UserDoesNotExist    User '%1%' doesn't exist
MSG Error UserIdDoesNotExist  User #%1% doesn't exist
MSG Error UserAccessKeyDoesNotExist  User access key '%1%'.'%2%' doesn't exist
MSG Error UserAccessKeyIdDoesNotExist  User access key #%1% doesn't exist
MSG Error UserTokenDoesNotExist  User token '%1%'.'%2%' doesn't exist
MSG Error UserTokenIdDoesNotExist  User token #%1% doesn't exist

# OBJECT ALREADY EXISTS
MSG Error InstanceAlreadyExists  Instance already exists
MSG Error DatabaseAlreadyExists  Database '%1%' already exists
MSG Error TableAlreadyExists     Table '%1%'.'%2%' already exists
MSG Error ColumnAlreadyExists    Column %1%'.'%2%'.'%3%' already exists
MSG Error ColumnDataBlockAlreadyExists  Column data block '%1%'.'%2%'.'%3%'.'%4%' already exists
MSG Error IndexAlreadyExists          Index '%1%'.'%2%' already exists
MSG Error ConstraintAlreadyExists     Constraint '%1%'.'%2%' already exists
MSG Error UserAlreadyExists           User '%1%' already exists
MSG Error UserAccessKeyAlreadyExists  User access key '%1%'.'%2%' already exists
MSG Error UserTokenAlreadyExists      User token '%1%'.'%2%' already exists

# INVALID NAMES
MSG Error InvalidDatabaseName  Invalid database name '%1%'
MSG Error InvalidTableName     Invalid table name '%1%'
MSG Error InvalidTableAlias    Invalid table alias '%1%'
MSG Error InvalidColumnName    Invalid column name '%1%'
MSG Error InvalidColumnNameInTableColumn      Invalid column name '%1%'.'%2%'.'%3%'
MSG Error InvalidColumnDataTypeInTableColumn  Invalid column data type %1% in the column '%2%'.'%3%'.'%4%'
MSG Error InvalidColumnAlias     Invalid column '%1%' alias '%2%'
MSG Error InvalidIndexName       Invalid index name '%1%'
MSG Error InvalidConstraintName  Invalid constraint name '%1%'
MSG Error InvalidConstraintNameInTable  Invalid constraint name '%1%'.'%2%'.'%3%'
MSG Error InvalidUserName           Invalid user name '%1%'
MSG Error InvalidUserAccessKeyName  Invalid user access key name '%1%'
MSG Error InvalidUserTokenName  Invalid user token name '%1%'

# INVALID OBJECT TYPE
MSG Error InvalidConstraintType    Constraint '%1%'.'%2%'.'%3%' has invalid constraint type
MSG Error InvalidValueType        Invalid value type '%1%'
MSG Error InvalidColumnType       Column '%1%'.'%2' type is invalid
MSG Error InvalidTransactionType  Transaction type '%1%' is invalid

# DUPLICATES
MSG Error SelectDuplicateColumnAlias      SELECT duplicate column alias '%1%'
MSG Error SelectDuplicateColumnName       SELECT duplicate column name '%1%'
MSG Error InsertDuplicateColumnName       INSERT duplicate column name '%1%'
MSG Error UpdateDuplicateColumnName       UPDATE duplicate column name '%1%'
MSG Error CreateTableDuplicateColumnName  CREATE TABLE duplicate column name '%1%'
MSG Error CreateTableDuplicateConstraintName  CREATE TABLE duplicate constraint name '%1%'
MSG Error CreateTableDuplicateColumnConstraintType  CREATE TABLE duplicate constraint type '%1%' in the column '%2%'

# WHERE
MSG Error LikePatternTypeIsWrong  LIKE pattern type '%1%' is not supported
MSG Error LikeValueTypeIsWrong    LIKE value type '%1%' is not supported
MSG Error InvalidWhereCondition   WHERE condition is invalid: %1%

# SELECT
MSG Error CannotUseAllColumnsAlias  Column alias '%1%' to "*" is prohibited
MSG Error UnexpectedNullValue  Unexpected NULL value in '%1%'.'%2%'.'%3%', TRID %4%
MSG Error SelectWithoutTables  Select without tables is not supported yet
MSG Error TableNotSpecified    SELECT: Table %1% is not specified
MSG Error ColumnIsUnknown      SELECT: Column '%1%'.'%2%' is unknown
MSG Error LimitValueTypeNotInteger LIMIT value type isn't integer
MSG Error LimitValueIsNegative LIMIT value is negative
MSG Error OffsetValueTypeNotInteger OFFSET value type isn't integer
MSG Error OffsetValueIsNegative OFFSET value is negative

# INSERT
MSG Error ValuesListNotMatchColumns     Length of '%1%'.'%2%' VALUES list doesn't match to the number of columns %3%, row %4% has %5% values
MSG Error ValuesListLengthsNotSame      '%1%'.'%2%' VALUES lists must all have the same length %3%, but row %4% has %5% values
MSG Error TableColumnIndexOutOfRange    Table column index '%3%' is out of range for the '%1%'.'%2%'
MSG Error CannotInsertToSystemTable     Insert into system table '%1%'.'%2%' is not allowed
MSG Error CannotInsertIntoMasterColumn  Cannot insert value into TRID, it is assigned automatically
MSG Error CannotInsertNullValue         Can't insert NULL value to column '%1%'.'%2%'.'%3%'

# UPDATE
MSG Error ValuesListIsEmpty             VALUES list type is empty 
MSG Error ColumnsListIsEmpty            '%1%'.'%2%' columns list type is empty 
MSG Error UpdateInvalidValueExpression  UPDATE SET expression is invalid: %1%
MSG Error CannotUpdateSystemTable       Update system table '%1%'.'%2%' is not allowed
MSG Error CannotUpdateMasterColumn      Cannot update master column
MSG Error ValueAndColumnTypeMismatch    Column '%1%' data type '%2%' isn't compatible with value type '%3%'
MSG Error UpdateValuesDoesNotFitToPositions      '%1%'.'%2%' values count %3% not equal to positions count %4%
MSG Error UpdateValuesCountGreaterThanAddresses  '%1%'.'%2%' values count %3% greater than column data addresses %4%
MSG Error UpdateTableIsNotEqualToColumnTable     Table '%1%' is not equal to '%2%'.'%3%'

# DELETE
MSG Error CannotDeleteFromSystemTable  Deleting from system table '%1%'.'%2%' is not allowed

# DROP USER
MSG Error CannotDropSuperUser  Can't drop superuser

# ALTER USER
MSG Error CannotChangeSuperUserState              Can't change superuser state
MSG Error InvalidUserAccessKey                    Invalid key in attempt to create user access key '%1%'.'%2%'
MSG Error UserAccessKeyIsTooLong                  User access key '%1%'.'%2%' size %3% is larger than maximum allowed size %4%
MSG Error CannotDeleteLastSuperUserAccessKey      Can't delete last superuser access key '%1%'
MSG Error CannotDeactivateLastSuperUserAccessKey  Can't deactivate last superuser access key '%1%'
MSG Error AlterUserAccessKeyFailed                Alter user access key '%1%'.'%2%' failed
MSG Error InvalidUserToken                        Invalid token value in attempt to create user token '%1%'.'%2%'

# CREATE DATABASE
MSG Error CannotCreateTemporaryDatabase  Temporary databases are not supported yet
MSG Error CipherIdTypeIsNotString        CIPHER_ID must be string
MSG Error CipherKeySeedIsNotString       CIPHER_KEY_SEED must be string

# DROP DATABASE
MSG Error CannotDropSystemDatabase   Can't drop system database
MSG Error CannotDropUsedDatabase     Can't drop database '%1%' which is currently in use
MSG Error CannotDropCurrentDatabase  Can't drop current database '%1%'

# RENAME DATABASE
MSG Error CannotRenameSystemDatabase   Can't rename system database
MSG Error CannotRenameUsedDatabase     Can't rename database '%1%' which is currently in use
MSG Error CannotRenameCurrentDatabase  Can't rename current database '%1%'

# CREATE TABLE
MSG Error TableTypeNotSupported  Table type %1% is not supported yet
MSG Error CannotCreateUserTablesInSystemDatabase  Can't create user table in the system database

# ENCRYPTION
MSG Error CipherUnknown     Cipher '%1%' is unknown
MSG Error InvalidCipherKey  Invalid key for the cipher '%1%'

# AUTHENTICATION
MSG Error UserAccessDenied     User '%1%' access denied
MSG Error SessionDoesNotExist  Session '%1%' doesn't exist

# UNSUPPORTED FEATURES
MSG Error ConstraintNotSupported   Constraint type #%4% is not supported (constraint definition '%1%'.%2% (%3%.%2%)
MSG Error ConstraintNotSupported2  Constraint type #%4% is not supported

# INVALID ATTRIBUTES
MSG Error InvalidNextUserTrid Invalid next user TRID %3% for the table '%1%'.'%2%'

##########################################
# INTERNAL MESSAGES
##########################################

##########################################
# I/O Errors
##########################################

ID -2
STEP -1

MSG Fatal InstanceNameMismatch  Instance name in the instance data files '%1%' doesn't match to the current instance name '%2%'

MSG Fatal CannotOpenInstanceMetadata  Can't open instance metadata file %1%: (%2%) %3%
MSG Fatal CannotLoadInstanceMetadata  Can't load instance metadata: (%1%) %2%
MSG Error CannotSaveInstanceMetadata  Can't save instance metadata: (%1%) %2%
MSG Fatal CannotOpenSystemDatabaseEncryptionKey  Can't open system database encryption key file %1%: (%2%) %3%
MSG Fatal CannotStatSystemDatabaseEncryptionKey  Can't stat system database encryption key file %1%: (%2%) %3%
MSG Fatal InvalidSystemDatabaseEncryptionKey  System database encryption key file %1% is invalid. Expected size %2%, but actual size is %3%
MSG Fatal CannotReadSystemDatabaseEncryptionKey  Can't read system database encryption key file %1%: (%2%) %3%
MSG Fatal CannotStatSuperUserKey                 Can't stat super user access key file %1%
MSG Fatal CannotOpenSuperUserKey                 Can't open super user access key file %1%
MSG Fatal SuperUserAccessKeyIsTooLong      Super user initial access key length %1% is larger than maximum %2%

MSG Error CannotCreateDatabaseMetadataFile  Can't create metadata file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotOpenDatabaseMetadataFile    Can't open metadata file '%1%' for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotReadDatabaseMetadataFile    Can't read from metadata file for the database '%1%' (%2%): (%3%) %4%
MSG Error CannotWriteDatabaseMetadataFile   Can't write to metadata for the database '%1%' (%2%): (%3%) %4%
MSG Warning CannotRemoveDatabaseDataDirectory  Can't remove data directory for the database '%1%' (%2%): (%3%) %4%
MSG Error DatabaseMetadataFileCorrupted     Metadata file '%1%' for the database '%2%' (%3%) is corrupted: %4%

MSG Error CannotCreateInstanceDataDir   Can't create instance data directory %1%: (%2%) %3%
MSG Error CannotClearInstanceDataDir    Can't clear instance directory %1%: (%2%) %3%
MSG Error InstanceDataDirIsNotDir       Instance directory path %1% is not a directory
MSG Error CannotCreateDatabaseDataDir   Can't create data directory %1% for the database '%2%' (%3%): (%4%) %5%
MSG Error CannotCreateTableDataDir      Can't create data directory %1% for the table '%2%'.'%3%' (%4%.%5%): (%6%) %7%
MSG Error CannotCreateColumnDataDir     Can't create data directory %1% for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotCreateIndexDataDir      Can't create data directory %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%

MSG Fatal CannotCreateInstanceInitializationFlagFile  Can't create instance initialization success data file %1%: %2%
MSG Fatal CannotOpenInstanceInitializationFlagFile    Can't open instance initialization success data file %1%: %2%
MSG Error CannotCreateDatabaseInitializationFlagFile  Can't create initialization success data file %1% for the database %2% (%3%): %4%
MSG Error CannotCreateTableInitializationFlagFile  Can't create initialization success data file %1% for the table '%2%'.'%3%' (%4%.%5%): %6%
MSG Error CannotCreateColumnInitializationFlagFile  Can't create initialization success data file %1% for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): %8%
MSG Error CannotCreateIndexInitializationFlagFile  Can't create initialization success data file %1% for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): %8%

MSG Error CannotCreateNewColumnDataBlockFile  Can't create new data block file %1% for the column '%2%'.'%3%'.'%4%'.%5% (%6%.%7%.%8%.%5%) : %9%: (%10%) %11%
MSG Error CannotOpenColumnDataBlockFile       Can't open data block file '%1%' for '%2%'.'%3%'.'%4%'.%5% (%6%.%7%.%8%.%5%)
MSG Error CannotReadColumnDataBlockFile       Can't read data block file '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) offset %8% length %9%: (%10%) %11%
MSG Error CannotWriteColumnDataBlockFile      Can't write data block file '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) offset %8% %9%: (%10%) %11%

MSG Error InvalidDataFileHeaderSize Invalid block data file header size '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error InvalidDataFileHeader     Invalid block data header '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%), ver '%8%'

MSG Error ColumnDataBlockNotAvailable         Column data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) is not available
MSG Error ColumnDataBlockConsistencyMismatch  Column data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) consistency error: %8%
MSG Error ColumnDataBlockDoesNotExist         Column data block '%1%'.'%2%'.'%3%'.%4% ('%5%'.'%6%'.'%7%'.%4%) doesn't exist

MSG Error CannotCreateBlockListDataFile   Can't create block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenBlockListDataFile     Can't open block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotGetBlockListDataFileSize  Can't get block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error InvalidBlockListDataFileSize    Invalid block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %7%
MSG Error CannotReadBlockListDataFile     %1%: Can't read block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %8% length %9%: (%10%) %11%
MSG Error CannotWriteBlockListDataFile    %1%: Can't write block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %8% length %9%: (%10%) %11%

MSG Error CannotCreateNextBlockListDataFile   Can't create next block list data file %1% for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenNextBlockListDataFile     Can't open next block list data file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotGetNextBlockListDataFileSize  Can't get next block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error InvalidNextBlockListDataFileSize    Invalid next block list data file size for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %7%
MSG Error CannotReadNextBlockListDataFile     %1%: Can't read next block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %9% length %9%: (%10%) %11%
MSG Error CannotWriteNextBlockListDataFile    %1%: Can't write next block list data file for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %8% length %9%: (%10%) %11%

MSG Error CannotCreateIndexFile  Can't create data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotLinkIndexFile    Can't link to filesystem data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotRenameIndexFile  Can't rename data file %1% into %2% for the index '%3%'.'%4%'.'%5%' (%6%.%7%.%8%): (%9%) %10%
MSG Error CannotOpenIndexFile    Can't open data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotStatIndexFile    Can't stat data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadIndexFile    Can't read from data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %8% length %9%: (%10%) %11%
MSG Error CannotWriteIndexFile   Can't write to data file %1% for the index '%2%'.'%3%'.'%4%' (%5%.%6%.%7%) offset %8% length %9%: (%10%) %11%
MSG Error IndexFileCorrupted     Index file '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) corrupted: %7%
MSG Error CannotFindIndexRoot    Can't find root node for the index '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error CannotFindIndexNode    Can't find root node for the index '%1%'.'%2%'.'%3%'.'%4%' (%5%.%6%.%7%.%4%)
MSG Error IndexNodeCorrupted     Index node '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) corrupted

MSG Error MasterColumnRecordIndexCorrupted Master column record index on table '%1%'.'%2%' (%3%.%4%) corrupted. Defect code %5%

MSG Error CannotCreateMainIndexIdFile  Can't create main index ID file %1 for column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenMainIndexIdFile    Can't open main index ID file %1 for column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadMainIndexIdFile    Can't read main index ID file for column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error CannotWriteMainIndexIdFile   Can't write main index ID file for column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%

MSG Error LobReadFailed  LOB read error in column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)

MSG Error BlockRegistryDirNotExists      Block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) doesn't exist
MSG Error BlockRegistryDirAlreadyExists  Block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) already exists
MSG Error BlockRegistryDirIsNotDir       Path of the block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is not a directory
MSG Error CannotCreateBlockRegistryDir   Failed to create block registry directory for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%

MSG Error CannotCreateSystemObjectsFile  Can't create system objects file for the database '%1%' (%2%): %3%
MSG Error CannotOpenSystemObjectsFile    Can't open system objects file for the database '%1%' (%2%): %3%
MSG Error CannotWriteSystemObjectsFile   Can't write to the system objects file for the database '%1%' (%2%): %3%
MSG Error CannotReadSystemObjectsFile    Can't read from the system objects file for the database '%1%' (%2%): %3%
MSG Error SystemObjectsFileCorrupted     System objects file for the database '%1%' (%2%) is corrupted: %3%
MSG Error CannotMoveSystemObjectsFile    Can't move system objects file for the database '%1%' (%2%): %3%
MSG Error CannotSerializeSystemObject    Can't serialize system object for the database '%1%' (%2%): %3%
MSG Error CannotDeserializeSystemObject  Can't deserialize system object for the database '%1%' (%2%): %3%

MSG Error UliFlushNodeCacheFailed  There were errors while flushing ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) node cache for the file #%7%: %8%

MSG Error BptiFlushNodeCacheFailed  There were errors while flushing BPTI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) node cache to disk: %7%

MSG Error CannotDeleteTridGreaterThanMax  Can't delete TRID %1% larger than '%2%'.'%3%' table max TRID %4%

MSG Error CannotCreateTridCountersFile  Can't create TRID counter file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotOpenTridCounterFile     Can't open TRID counter file '%1%' for the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%): (%8%) %9%
MSG Error CannotReadTridCounterFile     Can't read from TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error CannotWriteTridCounterFile    Can't write to TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error CannotRenameTridCounterFile   Can't rename TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): (%7%) %8%
MSG Error TridCounterFileCorrupted      TRID counter file for the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is corrupted

MSG Error OnlyConstantExpressionsAreAllowed  Only constant expressions are allowed in this context;
MSG Error TableDoesNotExistInContext         Table '%1%'.'%2%' doesn't exist in request context
MSG Error ColumnDoesNotExistInContext        Column '%1%'.'%2%'.'%3%' doesn't exist in request context
MSG Error DatasetHasNoTables                 Database '%1%' dataset has no tables

# OBJECT INITIALIZATION FILE DOESN'T EXIST
MSG Error DatabaseInitFileDoesNotExist  Database '%1%' initilaization file '%2%' doesn't exist
MSG Error TableInitFileDoesNotExist     Table '%1%'.'%2%' initilaization file '%3%' doesn't exist
MSG Error ColumnInitFileDoesNotExist    Column '%1%'.'%2%'.'%3%' initilaization file '%4%' doesn't exist
MSG Error IndexInitFileDoesNotExist     Index %1%'.'%2%'.'%3%' initilaization file '%4%' doesn't exist

# OBJECT FOLDER DOESN'T EXIST
MSG Error DatabaseDataFolderDoesNotExist    Database '%1%' data folder '%2%' doesn't exist
MSG Error TableDataFolderDoesNotExist       Table '%1%'.'%2%' data folder '%3%' doesn't exist
MSG Error ColumnDataFolderDoesNotExist      Column '%1%'.'%2%'.'%3%' data folder '%4%' doesn't exist
MSG Error IndexDataFolderDoesNotExist       Index %1%'.'%2%'.'%3%' data folder '%4%' doesn't exist

MSG Error kErrorDefaultValueDeserializationFailed  Failed deserialize default value for the constraint '%1%'.'%2%'.'%3%'.'%4%' (%5%.%6%.%7%.%8%)

##########################################
# Internal Errors
##########################################

ID -1000
STEP -1

MSG Error UserCacheFull      User cache is full
MSG Error DatabaseCacheFull  Database cache is full
MSG Error TableCacheFull       Table cache of the database '%1%' is full
MSG Error ConstraintCacheFull  Constraint cache of the database '%1%' is full
MSG Error ConstraintDefinitionCacheFull  Constraint definition cache of the table '%1%'.'%2%' is full

MSG Error InvalidMasterColumnDataType    Invalid master column data type for the table '%1%'.'%2%' (%3%.%3%)
MSG Error InvalidMasterColumnRecordSize  Invalid master column record size %6% in the table '%1%'.'%2%' column '%3%' (%4%.%5%.%6%) at address %7%.%8%: %9%
MSG Error InvalidMasterColumnRecordColumnCount  Invalid column count in master column record in the table '%1%'.'%2%' (%3%.%4%) at address %5%.%6%: expecting %7%, but actually %8%

MSG Error NotMasterColumn  Column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is not a master column
MSG Error TooManyColumns   Too many columns in the table '%1%'.'%2%' (%3%.%4%)
MSG Error IncompatibleDataType   Can't write value of incompatible data type into column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): required value type %7%, source value type %8%
MSG Error IncompatibleDataType2  Can't write value of incompatible data type into column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): required column data type %7%, source value type %8%

MSG Error InvalidDataBlockPosition          Invalid position %8% in the data block: '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error InvalidRollbackDataBlockPosition  Invalid position %8% in the data block for rollback: '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%), while current block is %9%
MSG Error InvalidCurrentDataBlock           Invalid current data block '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%)
MSG Error UnreachableRollbackDataBlockPosition  Rollback data block: '%1%'.'%2%'.'%3%'.%4% (%5%.%6%.%7%.%4%) is unreachable

MSG Error MissingTableColumns  Missing columns information for the table '%1%'.'%2%' (%3%.%4%)

MSG Error UliMissingLessValueWhenExpected     ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) missing less value when it is expected to exist
MSG Error UliMissingGreaterValueWhenExpected  ULI '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) missing greater value when it is expected to exist
MSG Error UliMissingNodeWhenExpected          ULI '%1%'.'%2%'.'%3%' (%5%.%6%.%7%) missing node %4% when it is expected to exist

MSG Error InvalidLobChunkHeader         Invalid LOB header at address '%7%'.'%8%' in the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %9%
MSG Error InvalidLobChunkHeaderAddress  Invalid LOB header address '%7%'.'%8%' in the column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%): %9%

MSG Error CannotGetCachedTable      Can't obtain cached table '%1%'.'%2%' (%3%.%4%)
MSG Error CannotGetSystemTableInfo  Can't obtain information about system table '%1%'.'%2%' (%3%.%4%)
MSG Error CannotFindAvailableBlockRecord  Can't find available block record for the block '%1%'.'%2%'.'%3%'.%4% (%4%.%5%.%6%.%4%)

MSG Error TableColumnRegistryCorrupted    Column registry corrupted in table '%1%'.'%2%'
MSG Error NumberOfValuesMistatchOnInsert  Insert to the table '%1%'.'%2%': Number of values (%3%) doesn't match number of columns (%4%)
MSG Error TooManyColumnsToInsert          Insert to the table '%1%'.'%2%': Too many columns provided (%3%), while maximum possible number of columns is %4%. 
MSG Error AdjustedNumberOfValuesMistatchOnInsert  Insert to the table '%1%'.'%2%': Adjusted number of values (%3%) doesn't match number of columns (%4%)
MSG Error NumberOfNextBlocksMistatchOnRollback    Rollback last insert in the table '%1%'.'%2%': Number of next block IDs (%3%) doesn't match number of columns (%4%)
MSG Error TooManyColumnsToRollback                Rollback last insert in the table '%1%'.'%2%': Too many columns provided (%3%), while maximum possible number of columns is %4%. 

MSG Error InvalidColumnSetTable    Column set #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidColumnTable       Column #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidIndexTable        Index #%1% of table #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidIndexColumnIndex  Index column #%1% of index #%2% doesn't belong to the index '%3%'.'%4%'.'%5%' (%6%.%7%.%8%)
MSG Error InvalidColumnSetColumnColumnSet  Column set column #%1% of column set #%2% doesn't belong to the column set '%2%'.'%3%'.%4% (%5%.%6%.%4%)
MSG Error InvalidColumnDefinitionColumn    Column definition #%1% of column #%2% doesn't belong to the column '%2%'.'%3%'.'%4%' (%5%.%6%.%7%)
MSG Error InvalidTableColumnDefinition     Column definition #%1% of column #%2% doesn't belong to the table '%3%'.'%4%' (%5%.%6%)
MSG Error InvalidColumnDefinitionConstraintColumnDefinition  Column definition constraint #%1% of column definition #%2% does not belong to the column definition '%3%'.'%4%'.'%5%'.%6% (%7%.%8%.%9%.%6%)

MSG Error CannotModifyClosedColumnSet         Can't modify closed column set '%1%'.'%2%'.%3% (%4%.%5%.%3%)
MSG Error CannotModifyClosedColumnDefinition  Can't modify closed column set '%1%'.'%2%'.'%3%'.%4% (%6%.%7%.%4%)
MSG Error CannotGenerateSystemTridUsingNonMasterColumn  Can't generate system TRID using non-master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error CannotGenerateUserTridUsingNonMasterColumn    Can't generate user TRID using non-master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%)
MSG Error ExplicitConstraintsForMasterColumnProhibited  Explicit spcification of constraints for the master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) is prohibited

### Internal integrity checks ###

# Database
ID -100000
MSG Error CannotReleaseUnusedDatabase Cannot release database '%1%' (%2%) which is marked as unused
MSG Error MissingSystemTable Missing system table '%1%'.'%2%' (%3%.'%4%')

# Table
ID -101000
MSG Error TableDoesNotBelongToDatabase  %1%: table '%2%'.'%3%' (%4%.%5%) doesn't belong to the database '%6%' (%7%)
MSG Error TableConstraintTypeDoesNotMatch  Constraint type #%1% doesn't match to required type #%2% in the table constraint '%3%'.'%4%'.'%5%' ('%6%'.'%7%'.'%8%') (constraint definition #%9%)
MSG Error TableMissingColumnSet  Table '%1%'.'%2%' ('%3%'.'%4%') missing column set

# Column set
ID -102000
MSG Error ColumnSetStillOpenForModification  Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) still open for modification
MSG Error ColumnSetAlreadyClosedForModification  Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) is already closed for modification
MSG Error ColumnSetMissingColumns  Column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%) missing columns

# Column
ID -103000
MSG Error ColumnDoesNotBelongToTable  %1%: column '%2%'.'%3%'.'%4%' (%4%.%5%.%6%) doesn't belong to the table '%6%'.'%7%' (%8%.%9%)
MSG Error ColumnConstraintTypeDoesNotMatch    Constraint type #%1% doesn't match to required type #%2% in the column constraint '%3%'.'%4%'.'%5%'.'%6%' ('%7%'.'%8%'.'%9%'.'%10%') (constraint definition #%11%)
MSG Error ColumnDoesNotBelongToColumnSet      Column #%6% doesn't belong to the column set '%1%'.'%2%'.[%3%] (%4%.%5%.%3%)
MSG Error MasterColumnMainIndexAlreadyExists  Main index of the master column '%1%'.'%2%'.'%3%' (%4%.%5%.%6%) already exist

# Column definition 
ID -104000
MSG Error MissingColumnDefinitionsForColumn Missing column definitions for the column [%1%].[%2%].[%3%]

# Constraint
ID -105000
MSG Error ColumnNotSpecifiedForColumnConstraint  Column is not specified when creating column constraint '%1%' of type #%2% in the table '%3%'.'%4%'(%5%.%6%)
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
MSG Error InvalidUserForUserToken  Invalid user #%1% for the user token #%2%
MSG Error CannotGenerateUserToken  Can't generate new token: %1%
MSG Error DuplicateUserTokenId     Duplicate user token id %2% for the user '%1%'
MSG Error DuplicateUserTokenValue  Duplicate user token value for the user token '%1%'.'%2%'
 
# Invalid attributes
# ID -110000
MSG Error InvalidLastUserTrid Invalid last user TRID %5% for the table '%1%'.'%2%' (%3%.%4%)

# END
