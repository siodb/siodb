# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

ID 100
MSG Error XXX1 YYYYYYYY
PMSG Error XXX2 YYYYYYYY

ID 1
MSG Info BBB1 ZZZZ YYY XXXX
MSG:22 Debug ZZZ1 Some details here
PMSG Info BBB2 ZZZZ YYY XXXX
PMSG:123 Debug ZZZ2 Some details here

ID 1000

MSG Info QBF_1LE Quick brown fox %1 jumps over the lazy dog

MSG Info QBF_1L Quick brown fox jumps over the lazy dog

MSG Info QBF_2L Quick brown fox \
    jumps over the lazy dog

MSG Info QBF_3L Quick brown fox \
    jumps over \
    the lazy dog

MSG Info QBF_4L Quick brown\
    fox %1% \
    jumps over %3 \
    3 the lazy dog

MSG Info QBF_2L_EL  \
    Quick brown fox
