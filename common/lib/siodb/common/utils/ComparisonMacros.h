// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Based on idea from here:
// https://stackoverflow.com/a/55210969/1540501

#define CMP_LT2(a, b) ((a) < (b) ? (a) : (b))
#define CMP_GT2(a, b) ((a) > (b) ? (a) : (b))
#define CMP_LTE2(a, b) ((a) <= (b) ? (a) : (b))
#define CMP_GTE2(a, b) ((a) >= (b) ? (a) : (b))
#define CMP_EQ2(a, b) ((a) == (b))
#define CMP_NEQ2(a, b) ((a) != (b))
#define CMP_LT3(a, b, c) (CMP_EQ2(a, b) ? (c) : CMP_LT2(a, b))
#define CMP_GT3(a, b, c) (CMP_EQ2(a, b) ? (c) : CMP_GT2(a, b))
#define CMP_LTE3(a, b, c) (CMP_EQ2(a, b) ? (c) : CMP_LT2(a, b))
#define CMP_GTE3(a, b, c) (CMP_EQ2(a, b) ? (c) : CMP_GT2(a, b))
#define CMP_EQ3(a, b, c) ((a) == (b) ? (c) : false)
#define CMP_NEQ3(a, b, c) ((a) != (b) ? true : (c))
