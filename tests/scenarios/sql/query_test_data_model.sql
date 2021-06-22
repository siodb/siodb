
drop database
if exists db1;
create database db1;

create table db1.t1
(
    cint int,
    ctext text
);

insert into db1.t1
values(1, 'a1');
insert into db1.t1
values(2, 'b1');
insert into db1.t1
values(3, 'c1');
insert into db1.t1
values(4, 'd1');

create table db1.t2
(
    cint int,
    ctext text
);

insert into db1.t2
values(1, 'a2');
insert into db1.t2
values(2, 'b2');
insert into db1.t2
values(3, 'c2');
insert into db1.t2
values(4, 'd2');


create table db1.t3
(
    cint int,
    ctext text
);

insert into db1.t3
values(1, 'a2');
insert into db1.t3
values(2, 'b2');
insert into db1.t3
values(3, 'c2');
insert into db1.t3
values(4, 'd2');