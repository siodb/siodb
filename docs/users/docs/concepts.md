# Concepts

## Instance

Siodb starts an instance to access a set of databases.
One instance can serve multiple databases.
An instance has a unique name per server.

## Database

A database is a set of files belonging to one database and organized in a certain way.
Siodb uses the instance to produce read and write operations into data files.
Data files contain database objects like tables or indexes.
They also contain business data.

## Column storage

First of all, Siodb stores its data per columns as opposed to row storage
(discover the difference here). Then, each column from each table has its own
set of data files. Furthermore, Siodb indexes each column by design according
to their type.

##  Data auto-sizing

Siodb has an automatic data type size for text and blob. Thanks to this feature,
 Siodb takes care of the data size without having to declare column size nor
 impacting performance. For instance, you just have to declare text and blob
 column like this:

```sql
siocli> create table table_test (
             post text,
             video blob
        ) ;
```

## Block partitioning

Siodb writes data from a column in data files called partitions. All partitions
 have the same size. Siodb fills the partition from the beginning to the end
 with the data from clients.

This storage architecture enables high throughput by leveraging the OS multi-threading
capacity. Indeed, for any kind of operation, multiples threads work in parallel to
 go through each partition of requested columns.

## Database versioning

Siodb converts each DML transaction into records. Then, Siodb appends those
records in the columns’ data files. Therefore, Siodb has always a consistent
view of data at any time natively.

This mechanism eliminates the need for a logging system and for an undo space.
Thus, it reduces I/Os considerably at the expense of more disk space consumption.
We choose that mechanism for the convenience it provides while nowadays disk space is cheap.

Therefore, Siodb provides natively versioning of both, the data model and the data.
That makes possible to create tags, flashback the database at any time, and query
the past without any additional configuration nor mechanism.

## The Cell Address Set

To rapidly identify any cells at any time, each cell has a unique address. And a
group of addresses from the cells of the same row is stored into a Cell Address Set.
In other words, Siodb maintains a set of cell addresses (Cell Address Set) per row.
The Cell Address Set is stored in the Master Column.

## The Master Column

The Master Column is a hidden column that Siodb automatically updates over time.
When Siodb produces a Cell Address Set, it stores this set into the Master Column.
Hence, Siodb always knows where the cells of a row are physically stored and can
quickly access to them with a minimum amount of I/Os. And to identify each row,
Siodb also stores a Table Row Id per row in the Master Column.

## Table Row Id

A Table Row Id is a unique numerical identifier (unsigned 64-bit integer). Siodb
increments this Table Row Id per table at each insert transaction. Thus, all newly
create rows in a table are uniquely identified with this Table Row Id. This provides
a virtual representation in rows on top of the physical column storage.
For instance, when you run this kind of query:

```sql
select * from table_1 where column_8 = 'user.mail@siodb.io' ;
```

Siodb will seek first the cells from column_8 of table_1 which match the filter
`user.mail@siodb.io`. For each cell, Siodb gets access to the Cell Address Set
in the master column from where Siodb gets the addresses of remaining cells in the
current row.

## Data changes tracking

Siodb also maintains multiple metadata for each row in the Master Column. For instance,
the date of creation or modification and the user who did the DML transactions on
that version of the row.

You can then query those metadata with standard SQL as you would for regular
column in a table. Thus you always know what’s happens to your data. Only Siodb
writes that metadata and it is not possible to modify them by design.

## Data TTL and data privacy

One of elements of the metadata that Siodb maintains is an expiration timestamp that can be
used to define a TTL (Time to Live) on rows. When the TTL has expired, Siodb
destroys the data physically for that row on the disk. The destruction is
then effortlessly propagated into backups at the next backup synchronization time.

This is a convenient and unique way to comply with the data destruction requested
by end-user using their rights from the data privacy regulations.
