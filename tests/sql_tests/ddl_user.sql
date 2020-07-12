-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

create user nicolas with state = active;

alter user nicolas
add access key mykey 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDD2eNrBjOHQCG6mv6GVy+HKCghldD+F0dm4S3kUmQ/PRJZyTVRpNCxCazXn44hG1faDh1MoUTXPwz1Uz68BZcbsrCMMUZ/poSauV8U1p+z+MwPQRsX5VmiTEnrsMm3J5CxExW5VW1+urT1XKeLcmNYoNG0DAIoz9WEnwmdRjKtSqaBA1ERGR0xiA6o85j0tgBXxLtECXBAY9g1Pj0I3tnsqHilFg+DyslrdUNiPdFX/rq+xCaPlwDjDFfJ5ckRJfdHcPuE7HqoNTTQrE3aAAsjjn34IhkcPof13rWH2bNLTcueIY3A3bqKWDlIb3wWqUyx4yxJyqYd6EXbkEYU6eTn';

select * from sys_user_access_keys;

alter user nicolas alter access key mykey set state = inactive;

alter user nicolas alter access key mykey set state = active;

alter user nicolas drop access key mykey;

drop user nicolas;
