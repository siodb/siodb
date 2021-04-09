-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

-- DB1
create database db1;
use database db1;
create table tablealldatatypes
(
    ctinyintmin TINYINT,
    ctinyintmax TINYINT,
    ctinyuint TINYUINT,
    csmallintmin SMALLINT,
    csmallintmax SMALLINT,
    csmalluint SMALLUINT,
    cintmin INT,
    cintmax INT,
    cuint UINT,
    cbigintmin BIGINT,
    cbigintmax BIGINT,
    cbiguint BIGUINT,
    cfloatmin FLOAT,
    cfloatmax FLOAT,
    cdoublemin   DOUBLE,
    cdoublemax   DOUBLE,
    ctext        TEXT,
    cts          TIMESTAMP
);

insert into tablealldatatypes
values
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    )
;

-- DB2
create database db2;
use database db2;

create table tablealldatatypes
(
    ctinyintmin TINYINT,
    ctinyintmax TINYINT,
    ctinyuint TINYUINT,
    csmallintmin SMALLINT,
    csmallintmax SMALLINT,
    csmalluint SMALLUINT,
    cintmin INT,
    cintmax INT,
    cuint UINT,
    cbigintmin BIGINT,
    cbigintmax BIGINT,
    cbiguint BIGUINT,
    cfloatmin FLOAT,
    cfloatmax FLOAT,
    cdoublemin   DOUBLE,
    cdoublemax   DOUBLE,
    ctext        TEXT,
    cts          TIMESTAMP
);

insert into tablealldatatypes
values
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    )
;

-- DB3
create database db3;
use database db3;
create table tablealldatatypes
(
    ctinyintmin TINYINT,
    ctinyintmax TINYINT,
    ctinyuint TINYUINT,
    csmallintmin SMALLINT,
    csmallintmax SMALLINT,
    csmalluint SMALLUINT,
    cintmin INT,
    cintmax INT,
    cuint UINT,
    cbigintmin BIGINT,
    cbigintmax BIGINT,
    cbiguint BIGUINT,
    cfloatmin FLOAT,
    cfloatmax FLOAT,
    cdoublemin   DOUBLE,
    cdoublemax   DOUBLE,
    ctext        TEXT,
    cts          TIMESTAMP
);

insert into tablealldatatypes
values
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '汉字',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        '-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAuDsSjBPsqO6mH18o/4Pc1jouoeD+GWGPmpgb/K6UxjTYS6ni
WBK0hPmN1aqBkHyj2i7SipiWcSr+lXGX71QOYLJV36nX5oy2WgAFO81ZKbLIOlam
oZKelWRp99BpTjHItZtM2Y/9I5pgFK3b97KQamysMT7hy3MrCI7377GReDqw4q99
+W9jPFKJSEWU2+/aCDl3UkkI11Ab/cKlT6g7WmtaBF0+vT2hk/89FPF/dtpwpYD7
lngHOSNKSbhn2qjuuGGgSP+GcTcCuwElfzSMb7JBoX8mfL74ZrVL5KhbFKKr+LeO
0G8vinaZwe6Ksyd6mIREzboKtcEXFw/96tnOAQIDAQABAoIBAALq39EZQc02MAoZ
NNgoyJQJimZyroh0rShZPEB56+oIgUQ/usF6/JoKXQu9JDmPYT2D0j6K0n9U21Cu
2o1yG/sN2O56iTxV1i+OPnb5Mk4+1iUXHSL79EQksalIlwnTj9B5LtgnS66bH8ZV
D2GslaUTTu3l11/cRVI4qCJPAjkUgyopTbm2gwuiaEJyyqYdSbj1avvKgJTM4igZ
B0GGRpcIO/O5hY+Pv26Bb/wwLa4gOpUyi4MVApSI63Dfn4/2Gmv4v3h/8olSXcZj
5BcUH+pdl+9tQ/jPALN1BTr1JBb/j09KLv8Fwrn8UMJixX0D9lhVEAMe0Ih1fSxu
epaUx6ECgYEAvWQ37/gcRYkem9GV7hnhm0IfrtVRmHQaKpYHbN9z7C9W47cHE8jd
5njgpsX5hjDPY6mVbRASuxpLbQL9pRyMTTenbrOKVs5SfPa4ICPOuv2JWZwoMD1C
vQAqoMwaHtDYkw7YgdO9vHM/0eMHP6dIGcjsAE5cT9mcbc3E/cW4DGECgYEA+QY1
yOCDe8RH2U2yPZIeMgJ9yhIeJJwbSmbsLlm8DA+gud4gu55fn+kGVtKlxi9hTxfW
wR0uBwUe5P8+EGnhjXNVryEZI+JXKNOHo+SpA0Ra2+IMQUkKx3ByN7lCShMVF6Xg
zKI6HC4+Y9wm2SO79FYqcI8qI5goi13t01pQJaECgYEApZRF924SwZR1B0PAcg98
l/HCo2bq1H/FFLBgQ4ZE4hwtOh8dd+WoY0QRHJ1/XxuzZW2xL03bImuFwAPaYA1K
eIQMxRMBAo2Vvp0xMyA7MG5TM937oNkeTQElQ7nNqF1sy30yOqc1fdnA1S5IexU0
Sx7HfikEOeeGBNXewQOoumECgYBd33Js+/10rYQsLXbQcQGC9p92iifkwxgijvPf
cSJLJaUAC/Uo5MXFYTFrj1LAh+HVz/W7rIVKTircRj+eLlvBV8XoE8EHXu5eTIco
SC1SNvVNSEQ4ZBF1JzVXPjX3+ION+5DncwwWzXPlbvsSBb93lve+oKlQ6631A36A
mt31oQKBgHolJf4H7rp//MSJDojVtMThHxNHBLwL0cCUIVaVvI0G4+UEW8pCyK29
RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----',
        CURRENT_TIMESTAMP
    ),
    (
        -128,
        127,
        255,
        -32768,
        32767,
        65535,
        -2147483648,
        2147483647,
        4294967295,
        -9223372036854775808,
        9223372036854775807,
        18446744073709551615,
        222.222,
        222.222,
        222.222,
        222.222,
        'RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----RqIBteLI5l3qQRIr7Y3PbYqVThvwBGyO/LeOTrCOxba6PyjXgLY4HtFpYpS6p/Rj
PrNYxF4pzNmSyyojtONpfHuobo4pt+4MUjRwAMLnbdF95vXKoAR9
-----END RSA PRIVATE KEY-----',
        CURRENT_TIMESTAMP
    )
;
