-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

SELECT trid, type, name, first_user_trid, current_column_set_id, description
FROM sys.sys_tables;
