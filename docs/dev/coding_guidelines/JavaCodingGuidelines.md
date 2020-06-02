# Siodb Java Coding Guidelines

## Introduction

This document lists requirements to Java code. Many of them are based on established standards
collected from a number of sources, individual experience, local requirements/needs.

Code readability must be taken as first priority. Given that, code efficincy must be achieved
mostly by using proper data structures, algorithms, standard functions and classes.

While a given development environment (IDE) can improve the readability of code by access
visibility, color coding, automatic formatting and so on, the programmer should never rely on such
features. Source code should always be considered larger than the IDE it is developed within and
should be written in a way that maximise its readability independent of any IDE.

## 1. General Rules

**Rule 1.1** All Java code must use Java 8 syntax. Avoid intentionally writing more
complicated code than mentioned above standard allows that is compatible with earlier
Java specifications. Java standard version may be eventually upgraded as long as Oracle
drops support for the mentioned Java version.

**Rule 1.2** Follow Google Java Style Guide (further "GJSG") with addions and exceptions
described in the rest of this document.

## 2. Differences from GJSG

**Rule 2.1** Each source code file must start with following "header" comment:

```Java
// Copyright (C) 2019-<current year> Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.
```

**Rule 2.2** Use 4 space indent instead of 2 space.

**Rule 2.3** Mark local variable `final` if it is not supposed to be modified
after initialization.

## Appendix A. Useful Links

1. [Google Java Style Guide](https://google.github.io/styleguide/javaguide.html)
