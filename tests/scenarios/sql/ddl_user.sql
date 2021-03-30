-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

create user nicolas;
create user buzz
with state = inactive;
select *
from sys_users;

alter user nicolas
set description
= 'I am Nicolas';
alter user buzz
set real_name
= 'Buzzzzzz', description = 'I am Buzzzzzz';
select *
from sys_users;

alter user nicolas
add access key mykey 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDD2eNrBjOHQCG6mv6GVy+HKCghldD+F0dm4S3kUmQ/PRJZyTVRpNCxCazXn44hG1faDh1MoUTXPwz1Uz68BZcbsrCMMUZ/poSauV8U1p+z+MwPQRsX5VmiTEnrsMm3J5CxExW5VW1+urT1XKeLcmNYoNG0DAIoz9WEnwmdRjKtSqaBA1ERGR0xiA6o85j0tgBXxLtECXBAY9g1Pj0I3tnsqHilFg+DyslrdUNiPdFX/rq+xCaPlwDjDFfJ5ckRJfdHcPuE7HqoNTTQrE3aAAsjjn34IhkcPof13rWH2bNLTcueIY3A3bqKWDlIb3wWqUyx4yxJyqYd6EXbkEYU6eTn';
select *
from sys_user_access_keys;

alter user nicolas
alter access key mykey
set state
= inactive;
select *
from sys_user_access_keys;

alter user nicolas
alter access key mykey
set state
= active;
select *
from sys_user_access_keys;

alter user nicolas
alter access key mykey
set description
= 'my key';
select *
from sys_user_access_keys;

alter user nicolas
alter access key mykey
set description
= null;
select *
from sys_user_access_keys;

alter user nicolas
drop access key mykey;
select *
from sys_user_access_keys;

alter user nicolas
add token mytoken1
with expiration_timestamp='2023-01-01 01:01:01';
select *
from sys_user_tokens;

alter user nicolas
add token mytoken2 x'0123456789abcdef0123456789abcdef';
select *
from sys_user_tokens;

alter user nicolas
alter token mytoken1
set expiration_timestamp
='2021-01-01 01:01:01';
select *
from sys_user_tokens;

alter user nicolas
alter token mytoken2
set expiration_timestamp
='2022-01-01 01:01:01';
select *
from sys_user_tokens;

alter user nicolas
alter token mytoken1
set description
='my token #1';
alter user nicolas
alter token mytoken2
set description
='my token #2';
select *
from sys_user_tokens;

alter user nicolas
alter token mytoken2
set description
=null;
select *
from sys_user_tokens;

alter user nicolas
alter token mytoken1
set expiration_timestamp
=null;
select *
from sys_user_tokens;

alter user nicolas
drop token mytoken1;
select *
from sys_user_tokens;

alter user nicolas
add token mytoken3 x'0123456789abcdef0123456789abcdef01';
check token nicolas.mytoken3 x'0123456789abcdef0123456789abcdef01';

drop user nicolas;
select *
from sys_users;
select *
from sys_user_access_keys;
select *
from sys_user_tokens;
