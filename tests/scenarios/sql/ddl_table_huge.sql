-- Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

drop database if exists db_default;
create database db_default;

use database db_default;

show tables;

drop table if exists table_100_cols_all_datatypes_0;
create table table_100_cols_all_datatypes_0 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
drop table if exists table_100_cols_all_datatypes_1;
create table table_100_cols_all_datatypes_1 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
create table table_100_cols_all_datatypes_3 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
create table table_100_cols_all_datatypes_4 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
create table table_100_cols_all_datatypes_5 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
create table table_100_cols_all_datatypes_6 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
create table table_100_cols_all_datatypes_7 (
    col_1_integer integer,
    col_2_int int,
    col_3_uint uint,
    col_4_tinyint tinyint,
    col_5_tinyuint tinyuint,
    col_6_smallint smallint,
    col_7_smalluint smalluint,
    col_8_bigint bigint,
    col_9_biguint biguint,
    col_10_smallreal smallreal,
    col_11_real real,
    col_12_float float,
    col_13_double double,
    col_14_text text,
    col_15_char char,
    col_16_varchar varchar,
    col_17_blob blob,
    col_18_timestamp timestamp,
    col_19_integer integer,
    col_20_int int,
    col_21_uint uint,
    col_22_tinyint tinyint,
    col_23_tinyuint tinyuint,
    col_24_smallint smallint,
    col_25_smalluint smalluint,
    col_26_bigint bigint,
    col_27_biguint biguint,
    col_28_smallreal smallreal,
    col_29_real real,
    col_30_float float,
    col_31_double double,
    col_32_text text,
    col_33_char char,
    col_34_varchar varchar,
    col_35_blob blob,
    col_36_timestamp timestamp,
    col_37_integer integer,
    col_38_int int,
    col_39_uint uint,
    col_40_tinyint tinyint,
    col_41_tinyuint tinyuint,
    col_42_smallint smallint,
    col_43_smalluint smalluint,
    col_44_bigint bigint,
    col_45_biguint biguint,
    col_46_smallreal smallreal,
    col_47_real real,
    col_48_float float,
    col_49_double double,
    col_50_text text,
    col_51_char char,
    col_52_varchar varchar,
    col_53_blob blob,
    col_54_timestamp timestamp,
    col_55_integer integer,
    col_56_int int,
    col_57_uint uint,
    col_58_tinyint tinyint,
    col_59_tinyuint tinyuint,
    col_60_smallint smallint,
    col_61_smalluint smalluint,
    col_62_bigint bigint,
    col_63_biguint biguint,
    col_64_smallreal smallreal,
    col_65_real real,
    col_66_float float,
    col_67_double double,
    col_68_text text,
    col_69_char char,
    col_70_varchar varchar,
    col_71_blob blob,
    col_72_timestamp timestamp,
    col_73_integer integer,
    col_74_int int,
    col_75_uint uint,
    col_76_tinyint tinyint,
    col_77_tinyuint tinyuint,
    col_78_smallint smallint,
    col_79_smalluint smalluint,
    col_80_bigint bigint,
    col_81_biguint biguint,
    col_82_smallreal smallreal,
    col_83_real real,
    col_84_float float,
    col_85_double double,
    col_86_text text,
    col_87_char char,
    col_88_varchar varchar,
    col_89_blob blob,
    col_90_timestamp timestamp,
    col_91_integer integer,
    col_92_int int,
    col_93_uint uint,
    col_94_tinyint tinyint,
    col_95_tinyuint tinyuint,
    col_96_smallint smallint,
    col_97_smalluint smalluint,
    col_98_bigint bigint,
    col_99_biguint biguint,
    col_100_smallreal smallreal
);
desc table table_100_cols_all_datatypes_1;
show tables;

-- Select dictionary from db_defaulty
-- (Added aliases to find it in the log faster)
select st.* from sys_tables st;
select sc.* from sys_columns sc;

-- Cleanup
use database sys;
drop database db_default;