# Siodb C++ Coding Guidelines

## Introduction

This document lists requirements to C++ code. Many of them are based on established standards
collected from a number of sources, individual experience, local requirements/needs.

Code readability must be taken as first priority. Given that, code efficincy must be achieved
mostly by using proper data structures, algorithms, standard functions and classes.

While a given development environment (IDE) can improve the readability of code by access
visibility, color coding, automatic formatting and so on, the programmer should never rely on such
features. Source code should always be considered larger than the IDE it is developed within and
should be written in a way that maximise its readability independent of any IDE.

Most rules listed here are applicable to the C code as well. In some cases, rules say explicitly,
what are differences between C and C++ code regarding the certain rule.

The key words “MUST”, “MUST NOT”, “REQUIRED”, “SHALL”, “SHALL NOT”, “SHOULD”, “SHOULD NOT”,
“RECOMMENDED”, “MAY”, and “OPTIONAL” in this document are to be interpreted as described
in the [RFC 2119](https://www.ietf.org/rfc/rfc2119.txt).

## 1. General Rules

**Rule 1.1** All C++ code must use C++17 standard. Avoid intentionally writing more
complicated code than mentioned above standard allows, which is compatible with earlier
C++ standards. Current standard version in use will be aggressively updated over time.

**Rule 1.2** All C code must use C99 standard. Avoid intentionally writing more
complicated code than mentioned above standard allows, which is compatible with earlier C standards.
Standard version may be updated over time, but we don't tend to do it since there is
very small amount of C code in the project and C99 still fit very good to our purposes.

## 2. Naming Conventions

**Rule 2.1** All names should be written in English.

```C++
// Correct
fileName


// INCORRECT
filNavn
imjePliku
```

**Rule 2.2** Names representing types must be in the mixed case starting with upper case letter.

```C++
// Correct
class Database {

};

// Correct
class FileBuffer {

};

// INCORRECT
class database {

};

// INCORRECT
class file_buffer {

};
```

**Exceptions:** When writing STL-like common classes located in stl_ext, C++ STL naming conventions
is used, which in all-lowercase letters with underscores as delimiters between words. For example:

```C++
// Common STL extension class
class concurrent_queue {

};
```

**Rule 2.3** Variable names must be in mixed case starting with lower case letter.

```C++
// Correct
const auto columnCount = getColumnCount();

// INCORRECT
auto columncount = getColumnCount();
auto ColumnCount = getColumnCount();
auto column_count = getColumnCount();
```

**Exceptions:** When writing STL-like common classes located in stl_ext, C++ STL naming conventions
is used, which in all-lowercase letters with underscores as delimiters between words. For example:

```C++
// Varibale in the common STL extension class method
const auto new_size = old_size + delta_size;
```

**Rule 2.4** Named constants (including enumeration values) must start with lowercase letter "k"
and continue in the mixed case starting with upper case letter.

```C++
// Correct
const std::size_t kSize = 10;

// Correct
enum class TableType {
    kMemory,
    kDisk,
    kMax
};


// INCORRECT
const std::size_t size = 10;
const std::size_t SIZE = 10;
const std::size_t ksize = 10;
const std::size_t kSIZE = 10;

enum class TableType {
    // INCORRECT
    MEMORY,
    DISK,
    MAX
};
```

**Rule 2.5** Names representing methods or functions must be verbs and written in mixed case
starting with a lower case letter.

```C++
class SomeClass {
    // Correct
    void sayHello();

    // INCORRECT
    void say_hello();

    // Inсorrect
    void SayHello();
};
```

**Rule 2.6** Names representing namespaces should be all lowercase and use underscore as delimiter
between words.

```C++
// Correct
namespace siodb {

}

// Correct
namespace something_big {

}


// INCORRECT
namespace Siodb {

}

// INCORRECT
namespace SomethingBig {

}

// INCORRECT
namespace AIRCRAFT {

}
```

**Rule 2.7** Names representing template types should be named same way as other type names,
but also can be a single uppercase letter.

```C++
// Correct
template<class T>
class Something {

};

// Correct
template<class Element>
class MyContainer {

};


// INCORRECT, x is lowercase
template<class x>
class MyContainer {

};

// INCORRECT, "element" does not match type naming scheme
template<class element>
class MyContainer {

};
```

**Rule 2.8** Abbreviations and acronyms must have only leading uppercase letter when used as name
or its part.

```C++
// Correct
exportHtmlSource();
openDvdPlayer();

// Correct
class Html {

};


// INCORRECT
exportHTMLSource();
openDVDPlayer();

// INCORRECT
class HTML {

};
```

**Rule 2.9** Global variables must be named with prefix `g_`.

```C++

// Correct
int g_argc;

// INCORRECT
int ARGC;
int argc_;
int __argc;
```

**Rule 2.10** Both constant and non-constant Non-static member variables must be named with prefix
`m_`. This applies to STL-like classes too.

```C++
class MyClass {

private:
    // Correct
    int m_something;

    // Correct
    const int m_somethingElse;


    // INCORRECT
    int something;

    // INCORRECT
    int somethingElse;

    // INCORRECT
    int something_;

    // INCORRECT
    const int somethingElse_;

};
```

**Rule 2.11** Static non-constant member variables must be named with prefix `s_`.
This applies to STL-like classes too.

```C++
class MyClass {

private:
    // Correct
    static int s_something;


    // INCORRECT
    static int m_something;

    // INCORRECT
    static int something;

    // INCORRECT
    static int something_;

};
```

**Rule 2.12** Static constant (`const`) member variables must be named with prefix `s_`.
This applies to STL-like classes too.

```C++
class MyClass {

private:
    // Correct
    static const int s_something;


    // INCORRECT
    static const int m_something;

    // INCORRECT
    static const int something;

    // INCORRECT
    static const int something_;

};
```

**Rule 2.13** Static constant experession (`constexpr`) member variables must follow constant
naming rules. STL-like classes must prefix use `k_` and usual for them further naming convention.

```C++
class MyClass {

private:
    // Correct
    static constexpr int kSomething = 10;


    // INCORRECT
    static constexpr int m_something = 10;

    // INCORRECT
    static constexpr int something = 10;

    // INCORRECT
    static constexpr int something_ = 10;

};
```

**Rule 2.14** Generic variables should have the same name as their type.

```C++
// Correct
void setTopic(const Topic& topic);
void connect(Database& database;


// Inorrect
void setTopic(const Topic& value);
void setTopic(Topic* aTopic);
void setTopic(Topic* t);

// INCORRECT
void connect(Database& db);
void connect(Database& oracleDB);
```

**Rule 2.15** Variables with a large scope should have long names, and variables with a small scope
can have short names. For example, scratch variables used for temporary storage or indices are best
kept short. Common scratch variables:

- For integers: `i`, `j`, `k`, `m`, `n`
- For characters: `c`, `ch`, `d`
- For constainer elements: `e`, `v`.

```C++
// Correct
for (int i = 0; i < 10; ++i) {
    std::cout << i << "^2 = " << (i * i) << std::endl;
}

// Correct
std::unordered_map<std::string, std::size_t> wordCounts;
for (const auto& e: wordCounts) {
    std::cout << "Word '" << e.first << "' happens " << e.second << " times." << std::endl;
}

// Correct
// NOTE: C code
while ((char ch = fgetc(stdin))) {
    switch (ch) {
        // ....
    }
}


// INCORRECT
for (int theIndexIWantToPrint = 0; theIndexIWantToPrint < 10; ++theIndexIWantToPrint) {
    std::cout << theIndexIWantToPrint << "^2 = " << (theIndexIWantToPrint * theIndexIWantToPrint)
              << std::endl;
}
```

**Rule 2.16** The name of the object is implicit, and must be avoided in a method name.

```C++
// Correct
line.getLength()
database.getId()


// INCORRECT
line.getLineLength()
database.getDatabaseId()
```

**Rule 2.17** The terms `get`/`set` must be used where a member variable is accessed directly.

```C++
// Correct
employee.getName();
employee.setName(name);
matrix.getElement(2, 4);
matrix.setElement(2, 4, value);
```

**Exception:** STL-like classes should follow STL naming convention, where member functions normally
that would be named as `getSomething()` or `setSomething()` should be named just `something()`,
getter is const, setter is not const.

**Rule 2.18** The term `compute` can be used in methods where something is computed.

```C++
// Correct
valueSet->computeAverage();
matrix->computeInverse();


// INCORRECT
valueSet->average();
matrix->inverse();
```

**Exception:** STL-like classes should follow STL naming convention, where member function that
normally would be named as `computeSomething()` should be named just `something()`.

**Rule 2.19** The term `find` can be used in methods where something is looked up.

```C++
// Correct
table.findColumn(columnName)
```

**Rule 2.20** The term initialize can be used where an object or a concept is established.
The american `initialize` should be used instead of the English `initialise`.
Abbreviation `init` must be avoided.

```C++
printer.initializeFontSet();
```

**Rule 2.21** Variables representing GUI components should be suffixed by the component type name.

```C++
mainWindow, propertiesDialog, widthScale, loginText,
leftScrollbar, mainForm, fileMenu, minLabel, exitButton, yesToggle etc.
```

**Rule 2.22** Plural form should be used on names representing a collection of objects.

```C++
// Correct
std::vector<Point> points;


// INCORRECT
std::vector<Point> point;
```

**Rule 2.23** The suffix `Count` should be used for variables representing a number of objects.

```C++
// Correct
const auto objectCount = objects.size();

// INCORRECT
const auto nObjects = objects.size();
const auto n_objects = objects.size();
const auto objectNumber = objects.size();
```

**Rule 2.24** Short-lived variables representing a number of objects must be named `n`, `m`.

```C++
std::vector<Variant> values;

std::vector<std::size_t> nullValueIndices;

// Correct
for (std::size_t i = 0, n = values.size(); i != n; ++i) {
    if (values[i].isNull()) nullValueIndices.push_back(i);
}
```

**Rule 2.25** The suffix `Id` should be used for variables representing an entity number.

```C++
// Correct
objectId

// INCORRECT
object
objectNo
objectNr
```

**Rule 2.26** Integer iterator variables must called like `i`, `j`, `k`, but also can be
called `index`. Avoid shortening `idx`.

```C++
// Correct
for (int i = 0; i < n; ++i) {
  std::cout << i << std::endl;
}

// Correct
std::vector<std::size_t> nullValueIndices;
for (std::size_t index = 0, valueCount = values.size(); index != valueCount; ++index) {
    if (values[index].isNull()) nullValueIndices.push_back(i);
}

// INCORRECT
std::vector<std::size_t> nullValueIndices;
for (std::size_t idx = 0, n = values.size(); idx != n; ++idx) {
    if (values[idx].isNull()) nullValueIndices.push_back(i);
}
```

**Rule 2.27** Iterator variables ot container iterator type must be called like `it`, `itSomething`.
Reverse iterators must be called `rit`, `ritSomething`.

```C++
// Correct
std::vector<int> numbers;
for (auto it = numbers.begin(); it != number.end(); ++it) {
    std::cout << *it << std::endl;
}

// Correct
std::vector<int> numbers;
for (auto rit = numbers.rbegin(); rit != number.rend(); ++rit) {
    std::cout << *rit << std::endl;
}


// INCORRECT - reverse iterator must be "rit".
std::vector<int> numbers;
for (auto it = numbers.rbegin(); it != number.rend(); ++it) {
    std::cout << *it << std::endl;
}
```

**Rule 2.28** The prefix `is` must be used for the general methods returning boolean.

```C++
// Correct
bool isNull() const noexcept;


// INCORRECT
bool getIsNull() const noexcept;
```

**Exception:** STL-like classes follow STL naming convention. For example, `empty()`
instead of `isEmpty()`.

**Rule 2.29** The prefix `has` must be used for the methods returning boolean which indicate that
object has valid value for some property.

```C++
// Correct
bool hasDeadline() const noexcept;


// INCORRECT
bool getHasDeadline() const noexcept;
```

**Exception:** STL-like classes must use prefix `has_`.

**Rule 2.30** Complement names must be used for complement operations. Reduce complexity
by symmetry.

```C++
get/set, add/remove, create/destroy, start/stop, insert/erase,
increment/decrement, old/new, begin/end, first/last, up/down, min/max,
next/previous, open/close, show/hide, suspend/resume, etc.
```

**Note:** To be more close to SQL terms, we also allow pair `create/drop`.

**Rule 2.31** Abbreviations in names must be avoided.

```C++
// Correct
computeAverage();


// INCORRECT
compAvg();
```

There are two types of words to consider. First are the common words listed in a language
dictionary. These must never be abbreviated. Never write:

```text
cmd   instead of   command

cp    instead of   copy

pt    instead of   point

comp  instead of   compute

init  instead of   initialize

etc.
```

Then there are domain specific phrases that are more naturally known through their
abbreviations/acronym. These phrases should be kept abbreviated. Never write:

```text
HypertextMarkupLanguage  instead of   html

CentralProcessingUnit    instead of   cpu

PriceEarningRatio        instead of   pe

etc.
```

**Rule 2.32** Naming pointers and references specifically must be avoided.

```C++
// Correct
Line* line
Line& line


// INCORRECT
Line* pLine
Line& rLine
```

**Rule 2.33** Negated boolean variable names must be avoided.

```C++
// Correct
bool isError;
bool isFound;

// INCORRECT
bool isNoError;
bool isNotFound;
```

**Rule 2.34** Exception classes should be suffixed with `Error` or `Exception`.
Prefer suffix `Exception` for exception base classes, and `Error` for final classes that indicate
concrete error situation.

```C++
// Correct
class SomethingNotFoundError: public std::runtime_error {

};

// Correct
class MySubsystemException: public std::runtime_error {

};

class MySubsystemNotReadyError : public MySubsystemException {

};


// Inorrect
class SomethingNotFound: public std::runtime_error {

};
```

**Rule 2.35** Member functions returning something should be named after what they return
and procedures (member functions with void return type) after what they do.

```C++
// Correct
int getX();
void rotateLeft();
```

**Rule 2.36** Preprocessor macros must be named in all capital letters with undescore separator
between words.

```C++
// Correct
#define LOG_WARNING(x) std::cout << "Warning: " << x << std::endl


// INCORRECT
#define logWarning(x) std::cout << "Warning: " << x << std::endl
```

## 3. Files and Directories

**Rule 3.1** Directories in the source code tree are named with lowecase characters only
and use underscores as separator between words.

**Rule 3.2** Makefile names start with capital letter. Makefile name is always `Makefile`.
Includes parts of makefiles start their names with capital letter and must have extension `.mk`.

**Rule 3.3** All source code files must use UTF-8 encoding.

**Rule 3.4** C++ header files must have the extension `.h`.
C++ source files must have the extension `.cpp`.

**Rule 3.5** C header files must have the extension `.h`.
C source files must have the extension `.c`.

**Rule 3.6** All C and C++ source and header files **MUST** be formatted with clang-format using
provided clang-format configuration before committing to source code repository. This automatically
ensures a lot of (but not all) code formatting rules.

**Rule 3.7** File content must be kept within 100 columns.

**Rule 3.8** Each source code file must end with a newline charatcer. There must not be last
line in the file without newline character in the end.

**Rule 3.9** All source code files (even those are not C or C++) must use 4-space indent.
Special characters like "tab" and "page break" must be avoided.

**Exceptions:**

1. Tabs are allowed and must be used in the makefiles, since this is what
   GNU make requires.
2. Tabs are allowed and must be used in the TSV (tab-separated value) data files.

**Rule 3.10** Each header file must be self-contained, i.e. include all dependent headers that
allow it to compile without additional includes from user. Use `make check-headers` and
`make check-headers-release` to ensure that.

```C++
// This source file must compile as is.

////////// Begin of HeaderTest.cpp //////////////////
#include "AHeaderFile.h"
/////////// End of HeaderTest.cpp ///////////////////
```

**Rule 3.11** Each source code file must start with following "header" comment:

```C++
// Copyright (C) 2019-<current year> Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.
```

**Rule 3.12** Header files must not contain traditinal macro-based include guard, unless absolutely
required by the some third-party reasons. Instead, they must contain `#pragma once`
as first directive.

**Rule 3.13** Includes must be groupped in the listed below groups that appear strictly in the order
specified below and start with a group name comment. Groups that are not applicable must be skipped.
Headers inside groups must be sorted alphabetically. (Hint: clang-format does sorting in the group
automatically).

```C++
// Internal headers
// Contains headers not exposed for public use, typically in the subdirectory "detail".

// Project headers
// Contains headers from the current project

// Common project headers
// Contians headers from the "common" Siodb libraries

// CRT headers
// C runtime library headers (only those specified by C or C++ standard)

// STL headers
// C++ standard library headers (only those specified by C or C++ standard)

// System headers
// OS-specific headers, that are not part of C standard
// but may be part of OS-specific API,
// for example: unistd.h dlfcn.h Windows.h

// <third-party library name> headers
// e.g: Boost headers
// Headers belonging to the certain thord-party library.
// Third-party library header group must go in the alphabetical order of library names
```

**Rule 3.14** The incompleteness of split lines must be made obvious.

```C++
totalSum = a + b + c +
           d + e;

function(param1, param2,
          param3);

setText("Long line split"
         "into two parts.");

std::cout << "Something veeeeeerrrrrryyyyyy veeeeeeeerrrrrrrrrrryyyyyyyyyyyyy loooooooooooooooooong"
          << std::endl;

for (std::size_t index = 0; index < tableCount;
     i += step) {
  ...
}
```

## 4. Statements

### 4.1 Namespaces

**Rule 4.1.1** Scope delimiter (`::`) must be used for declaring nested namespaces.

```C++
// Correct
namespace siodb::iomgr::dbengine {

}  // namespace siodb::iomgr::dbengine


// INCORRECT
namespace siodb {
namespace iomgr {
namespace dbengine {

}  // namespace dbengine
}  // namespace iomgr
}  // namespace siodb
```

**Rule 4.1.2** Namespace must end with comment placed after closing bracket. Comment must consist
of word `namespace` and namespace name, including nested namespace name. Anonymous namespace ends
with comment containing only word `namespace`.

```C++
// Correct
namespace siodb::iomgr::dbengine {

}  // namespace siodb::iomgr::dbengine

// Correct
namespace {

}  // namespace


// INCORRECT
namespace siodb {

}

// INCORRECT
namespace {

}
```

**Rule 4.1.3** Use of `using namespace some-namespace-name;` is completely prohibited.
You can use local namespace alias instead, if you have to deal with long but often used
nested namespace (i.e. something like `boost::something::yet_another_something`).

**Rule 4.1.4** Global function, incluing OS API functions and system calls, must be referred to
using leading scope resolution (`::`) operator, when calling them, taking address, etc.

```C++
// Correct
auto pid = ::fork();


// INCORRECT, "fork" is global function, system call
auto pid = fork();
```

**Rule 4.1.4** Functions, types, classes in the namespaces `std` and `stdext` must be always
referred using respective namespace name prefix.

### 4.2 Types

**Rule 4.2.1** A class should be declared in a header file and defined in a source file where
the name of the files match the name of the class.

```C++
// Table.h
class Table {

    std::shared_ptr<Column> findColumn(const std::string& name);

};

// Table.cpp
std::shared_ptr<Column> Table::findColumn(const std::string& name)
{
    // ....
}
```

**Rule 4.2.2** Order of declaration in the class must be as described below.
Not applicable parts and sections must be skipped.

```C++
template<class T1, ..., class TN>
class ClassName : public PublicBaseClass1, ..., public PublicBaseClassN,
    protected ProtectedBaseClass1, ..., protected ProtectedBaseClassN,
    private PrivateBaseClass1, ..., private PrivateBaseClassN {
public:
    // Public type defintions

public:
    // Public static constants

public:
    // Constructors

    // Destrcutor

    // Operators

    // Public getters and getter-like const member functions

    // Public setter and setter-like member functions

    // Other public member functions

protected:
    // Protected type defintions

protected:
    // Protected getters and getter-like const member functions

    // Protected setter and setter-like member functions

    // Protected member functions

private:
    // Private type defintions

private:
    // Private getters and getter-like const member functions

    // Private setter and setter-like member functions

    // Private member functions

public:
    // Public supplementary type definitions

public:
    // Public member variables except public static constants

protected:
    // Protected supplementary type definitions

protected:
    // Protected member variables except public static constants

protected:
    // Protected static constants

private:
    // Private supplementary type definitions

private:
    // Private member variables except public static constants

private:
    // Private static constants
};
```

**Rule 4.2.3** Order of member function defintions in the source file must be the same as order
of their declarations in the respective header file.

**Rule 4.2.4** Small member function definitions, like on-line getters, must reside in the header
file, in the class declaration.

```C++
class Table {

    auto getColumnCount() const noexcept
    {
        return m_columns.size();
    }
}
```

**Rule 4.2.5** Virtual and overridden function definitions, no matter of function code size,
must reside in the source file.

**Rule 4.2.6** Other member function definitions (i.e. not small ones) must reside
in the source file.

### 4.3 Variables

**Rule 4.3.1** Variables must be initialized where they are declared. This applies to the both
C++ and C code (C99 allows C++-style variable declarations).

```C++
// Correct
int i = argc - 1;

// Correct, because std::string is class with default constructor, and it is called.
std::string s;


// INCORRECT
int i;
i = argc - 1;
```

**Rule 4.3.2** Variables should be declares as `auto` or `const auto`, unless explicit type
specification is required because type cannot be deduced or differs from initial value
expression type.

**Exceptions:** Variables can use explicit type declaration if type name is shorter or equal length
to keyword `auto` (4 characters). From standard type it is acceptable to use `int`, `bool`, `char`.

```C++
// Correct
auto x = 0.0;
auto y = 1;
auto f = 0.0f;
auto it = m.find(x);
int z = 1; // "int" is shorter than "auto"
bool b = isGreen(); // "bool" is 4 chars, and "auto" too
short z = 0; // Correct, because 0 is int and there is not modifier to tell it is short.

// INCORRECT
vector<int>::iterator it = v.begin();
float f = 0.0f;
double x = 0; // write auto x = 0.0; instead
```

**Rule 4.3.3** Variable which is not supposed to be changed after it is initialized, must be
declared as `const`.

```C++
std::shared_ptr<Object> findObject(const std::string& name) noexcept
{
    // Correct
    const auto it = m_objects.find(name);
    return it != m_objects.end() ? *it : nullptr;
}


std::shared_ptr<Object> findObject(const std::string& name) noexcept
{
    // INCORRECT, because "it" will not be modified after declaration and initialization.
    auto it = m_objects.find(name);
    return it != m_objects.end() ? *it : nullptr;
}
```

**Rule 4.3.4** Variables must never have dual meaning. Ensure that all concepts
are represented uniquely.

**Rule 4.3.5** Use of global variables should be minimized.

**Rule 4.3.6** Member variables of a class should never be declared public.

**Rule 4.3.7** C++ pointers and references should have their reference symbol next to the type
rather than to the name.

```C++
// Correct
int* p;
long& v;

// INCORRECT
int *p;
long &v;
```

**Rule 4.3.8** Implicit test for `false` should not be used other than for boolean variables
and pointers.

```C++
bool b;
void* x;
std::shared_ptr<Something> p;

int y;
double z;

// Correct
while (!b) {
    // ...
}

// Correct
while (b) {
    // ...
}

// Correct
if (!x) {
    // ...
}

// Correct
if (x) {
    // ...
}

// Correct
if (!p) {
    // ...
}

// Correct
if (p) {
    // ...
}


// Correct
if (y == 0) {

}

// Correct
if (z == 0.0) {

}


// INCORRECT
if (!y) {

}

// INCORRECT
if (z) {

}
```

**Rule 4.3.9** Variables should be declared in the smallest scope possible.

### 4.4 Functions

**Rule 4.4.1** All C and C++ source code must use Linux kernel like braces scheme:

- Structure and class declarations, all statememts inside functions use K&R braces.

```C++
// Correct
if (a > 1) {
    b = 10;
    sendNotification();
}


// INCORRECT
if (a > 1)
{
    b = 10;
    sendNotification();
}

// INCORRECT
if (a > 1)
  {
      b = 10;
      sendNotification();
  }
```

- Function definitions have Allman opening and closing braces.

```C++
// Correct
void MyClass:swap(MyClass& other) noexcept
{
    if (this != other) {
        std::swap(m_a, other.m_a);
        std::swap(m_b, other.m_b);
    }
}


// INCORRECT: opening and closing braces of the function declaration must be Allman-style.
void MyClass:swap(MyClass& other) noexcept {
    if (this != other) {
        std::swap(m_a, other.m_a);
        std::swap(m_b, other.m_b);
    }
}

// INCORRECT: braces inside function must be K&R
void MyClass:swap(MyClass& other) noexcept
{
    if (this != other)
    {
        std::swap(m_a, other.m_a);
        std::swap(m_b, other.m_b);
    }
}
```

**Rule 4.4.2** If non-static member function is not supposed to modify class state,
it must be declared `const`.

```C++
class Something {
public:
    // Correct
    auto getCount() const noexcept
    {
        return m_count;
    }


    // Inocrrect, must be const
    const std::string& getCode()
    {
        return m_code;
    }

private:
    std::size_t m_count;
    std::string m_code;
};
```

**Rule 4.4.3** If some function is not supposed to throw exceptions itself
or in any function called by it, you must declare it noexcept.

```C++
class Something {
public:
    // Correct
    auto getCount() const noexcept
    {
        return m_count;
    }

private:
    std::size_t m_count;
    std::string m_code;
};

// Correct
inline void swap(MyClass& a, MyClass& b) noexcept
{
    a.swap(b);
}


// INCORRECT - assuming MyClass::swap() is noexcept, this function must be noexcept too.
inline void swap(MyClass& a, MyClass& b)
{
    a.swap(b);
}

class Something {
public:

    // Inocrrect, must be noexcept
    const std::string& getCode() const
    {
        return m_code;
    }

private:
    std::size_t m_count;
    std::string m_code;
};

```

**Rule 4.4.4** Member functions that override virtual function in the base class must be
marked with `override` keyword. They also must not use `virtual` keyword.

```C++
class Base {
public:
    virtual ~Base() = default;

    virtual void f() = 0;
};

// Correct
class Derived: public Base {
public:
    void f() override;
};


// INCORRECT - missing "override"
class Derived: public Base {
public:
    void f();
};

// INCORRECT - unnecessary "virtual" keyword
class Derived: public Base {
public:
    virtual void f() override;
};
```

### 4.5 Loops

**Rule 4.5.1** `for` loop with single-line body must have following form:

```C++
// Correct
for (int a = 0; i < 10; ++i)
    doSomething();

// INCORRECT - extra braces are not needed
for (int a = 0; i < 10; ++i) {
    doSomething();
}
```

**Rule 4.5.2** `for` loop with multi-line body must have following form:

```C++
// Correct
for (int a = 0; i < 10; ++i) {
    doSomething();
    doSomethingElse()
}

// Correct
for (int a = 0; i < 10; ++i) {
    makeSomeProgress(getProcessingParameter1(), getProcessingParameter1(),
        getProcessingParameter3(), getProcessingParameter4());
}

// INCORRECT - multiline body, even single-statement, requires braces.
for (int a = 0; i < 10; ++i)
    makeSomeProgress(getProcessingParameter1(), getProcessingParameter1(),
        getProcessingParameter3(), getProcessingParameter4());
```

**Rule 4.5.3** Prefer using for-each loop instead of normal for loop where possible. Use for loop
only when you need index on each iteration or operands do not conform to the for-each loop.

```C++
std::vector<Variant> values;

// Correct
for (const auto value: values)
    std::cout << value << std::endl;

// INCORRECT
for (std::size_t i = 0; i != values.size(); ++i)
    std::cout << values[i] << std::endl;

// Correct - we need to collect indices.
std::vector<std::size_t> nullValueIndices;
for (std::size_t i = 0, n = values.size(); i != n; ++i) {
    if (values[i].isNull()) nullValueIndices.push_back(i);
}
```

**Rule 4.5.4** In the `for-each` loop, a loop variable must be declared as:

- `const auto` if iterable object contains values of the fundamental type and such value is not
  supposed to be modified inside the loop or `auto` if value can be modifed.
- `const auto&` if iterable object contains values of the non-fundamental type and such value
  is not supposed to be modified inside the loop or `auto&` if value can be modifed.

**Rule 4.5.5** Always use prefix increment and decrement operators in the 3rd operand of the for
loop. This is especially important when using container iterators.

```C++
// Correct
for (int i = 0; i < 10; ++i)
    std::cout << i << std::endl;

// Correct
for (auto it = v.rbegin(); it != v.rend(); ++it)
    std::cout << *it << std::endl;


// INCORRECT
for (int i = 0; i < 10; i++)
    std::cout << i << std::endl;

// INCORRECT
for (auto it = v.rbegin(); it != v.rend(); it++)
    std::cout << *it << std::endl;
```

**Rule 4.5.6** Only loop control statements must be included in the `for()` construction.

```C++
// Correct
int sum = 0;
for (int i = 1; i <= 10; ++i)
    sum += i;


// INCORRECT
int sum;
for (int i = 1, sum = 0; i <= 10; ++i)
    sum += i;
```

***Rule 4.5.7** Single-line `while` loop body must be places without block on the next line.

```C++
// Correct
while (!done) // Single statement
    done = doSomeProgress();

// Inorrect
while (!done) { // Not required
    done = doSomeProgress();
}
```

***Rule 4.5.8** Multi-line `while` loop body must be places with block.

```C++
// Correct
while (!done) { // Multiple statements
  doSomething();
  done = moreToDo();
}

// Correct
while (shouldRun()) {
    makeSomeProgress(getProcessingParameter1(), getProcessingParameter1(),
        getProcessingParameter3(), getProcessingParameter4());
}

// INCORRECT - multi-line statement, even single one, required block
while (shouldRun())
    makeSomeProgress(getProcessingParameter1(), getProcessingParameter1(),
        getProcessingParameter3(), getProcessingParameter4());
```

**Rule 4.5.9** The form `while (true)` must be used for infinite loops in the C++ code
and `while (1)` in the C code.

```C++
// Correct
while (true) {
  // do something
}


// INCORRECT
for (;;) {
  // do something
}

// INCORRECT in C++, but correct in the C code
while (1) {
  // do something
}
```

**Rule 4.5.10** A `do-while` loop must have the following form (statements are always in the block):

```C++
do {
  statements;
} while (condition);
```

**Rule 4.5.11** Loop variables should be initialized immediately before the loop.

```C++
// Correct
bool done = false;
while(!done) {
    // do something
}


// INCORRECT
bool done = false;

... some statements here ...

while(!done) {
    // do something
}
```

### 4.6 Conditionals and Branching

**Rule 4.6.1** Complex conditional expressions must be avoided. Introduce temporary boolean
variables instead.

```C++
// Correct
const bool isFinished = (elementNo < 0) || (elementNo > maxElement);
const bool isRepeatedEntry = elementNo == lastElement;
if (isFinished || isRepeatedEntry) {
  // ...
}

// INCORRECT
if ((elementNo < 0) || (elementNo > maxElement) || elementNo == lastElement) {
  // ...
}
```

**Rule 4.6.2** The nominal case should be put in the if-part and the exception in the else-part
of an if statement.

```C++
bool isOk = readFile(fileName);
if (isOk) {
  // ...
} else {
  // ...
}
```

**Rule 4.6.3** Short nominal flow statement can be put on the same line as conditional if
there is no alternative flow (`else`).

```C++
// Correct
if (!loadData()) return;
```

**Rule 4.6.4** Single line nominal and alternative flow statement (`else`) should be put
without block, multi-line statements should be put into block.

```C++
// Correct
int errorCode = loadData();
if (errorCode == 0)
    std::cout << "Data successfully loaded" << std::endl;
else {
    std::cerr << "Error: Data could not be loaded. Error code: " << errorCode << ", error message: "
              << getErrorMessage(errorCode) << std::endl;
}

// INCORRECT
int errorCode = loadData();
if (errorCode == 0)
    std::cout << "Data successfully loaded" << std::endl;
else // Multi-line statement must be put into block.
    std::cerr << "Error: Data could not be loaded. Error code: " << errorCode << ", error message: "
              << getErrorMessage(errorCode) << std::endl;
```

**Rule 4.6.5** Executable statements in conditionals must be avoided.

```C++
// Correct
auto file = open(fileName, "w");
if (!file) {
  // ....
}

// INCORRECT
if (!(file = open(fileName, "w"))) {
  // ....
}
```

**Exeception:** C++17 style local variable declaration with initialization is allowed.

```C++
// Correct
if (auto file = open(filename)) {
    // ...
}

// INCORRECT, extra operation "!"
if (!(auto file = open(filename))) {
    // ...
}

// INCORRECT, variable is not declared in the if, only initialized.
File* file;
// ...
if (file = open(filename)) {
    // ...
}
```

**Rule 4.6.6** `case` labels and a `default` clause in the `switch` statement must be idented
with a single indent.

```C++
switch (condition) {
    case ABC: {
      statement1;
      statement2;
      break;
    }

    case DEF: {
      statement1;
      statement1;
      break;
    }

    default: {
      statement1;
      statement1;
      break;
    }
}
```

**Rule 4.6.7** Single-line `case` and `default` bodies must be put without block, multi-line bodies,
even single-statement, must be put into block.

```C++
// Correct
switch (condition) {
    case ABC: return 1;

    default: {
      statement1;
      statement1;
      break;
    }
}


// INCORRECT
switch (condition) {
    case ABC: {
        return 1; // Single line body goes w/o block
    }

    default: {
      statement1;
      statement1;
      break;
    }
}

// INCORRECT
switch (condition) {
    case ABC: return 1;

    default: // multi-line bodies must always be in a block
      statement1;
      statement1;
      break;
}
```

**Rule 4.6.8** C++17 attribute `[[fallthrough]]` must be used to incicate fallthroughs
in the `switch` statement. In the C code comment `// fallthrough` must be put.

```C++
// Correct C++ code
switch (condition) {
    case ABC: doSomething();
    [[fallthrough]]

    case DEF: {
        statement1;
        statement1;
        break;
    }

    default: break;
}

// Correct C code
switch (condition) {
    case ABC: doSomething();
    // fallthrough

    case DEF: {
        statement1;
        statement1;
        break;
    }

    default: break;
}
```

**Rule 4.6.9** `switch` statement must either cover all possible argument values with `case` labels
or have `default` clause.

**Rule 4.6.10** The `default` clause must be placed in the end of the `switch` statement.

### 4.7 Other Statements

**Rule 4.7.1** `goto` must not be used in the C++ code. However, it is allowed in the C code.

**Rule 4.7.2** A `try-catch` statement must have the following form:

```C++
try {
  // ...
} catch (Exception& exception) {
  // ...
}
```

**Rule 4.7.3** In the `try-catch` statement, more handlers for the more specialized exceptions
according to class hierarchy must be placed before more general.

```C++
class MyException: public std::runtime_error {
    // ...
};

try {
  // ...
} catch (MyException& exception) { // More specialized exception according to class hierarchy
  // ...
} catch (std::exception& exception) { // More general exceptiion
  // ...
}
```

### 4.8 Other Language Constructs

**Rule 4.8.1** Use only C++ type cast operators (`static_cast`, `dynamic_cast`, `const_cast`,
`reinterpret_cast`) in the C++ code.

```C++
// Correct
auto a = static_cast<int>(x);

// INCORRECT
auto a = (int)x;
```

**Rule 4.8.2** `#endif` preprocessor directive must be followed by the comment with symbol name
or condition which is used in the last `#if`/`#ifdef`/`#ifndef`/`#elif` directive.

```C++
// Correct
#ifdef __cpulsplus >= 201703L
// ...
#endif  // __cpulsplus >= 201703L


// INCORRECT
// No required comment after the #endif
#ifdef __cpulsplus >= 201703L
// ...
#endif
```

**Rule 4.8.3** Do not define and/or use any overloaded literals (including ones provided
by the C++ standard library).

**Exception:** Literals provided by the C++ standard library and 3rd party libraries can be used
in the unit tests.

### 4.9 Other Rules

**Rule 4.9.1** The use of "magic numbers" in the code must be avoided. Numbers other than 0 and 1
should be considered declared as named constants instead.

```C++
// Correct
constexpr auto kPi = 3.14159265359;
inline double circleSquare(double r) noexcept
{
    return kPi * r * r;
}

// INCORRECT
inline double circleSquare(double r) noexcept
{
    return 3.14159265359 * r * r;
}
```

**Rule 4.9.2** Do not define your own constants if standard library already provides them.

**Rule 4.9.3** Use `std::numeric_limits` to obtain value limits instead of C-style constants
declared in the `<climits>`. Use constants from `limits.h` only in the C code.

**Rule 4.9.4** Floating point constants must be written with decimal point and at least
one decimal digit on the both sides of it.

```C++
// Correct
double total = 0.0;
double speed = 3.0e8;

// Correct
double sum;
// ...
sum = (a + b) * 10.0;


// INCORRECT
double total = 1.;
double total = .1;
double total = 0;
double speed = 3e8;
```

**Rule 4.9.5** Floating point constants must be written with a digit before the decimal point.

```C++
// Correct
double total = 0.5;


// INCORRECT
double total = .5;
```

**Rule 4.9.6**  In C++ code, `nullptr` must be be used instead of "NULL".
"NULL" must used only in C code.

**Rule 4.9.7** Use `constexpr` instead of `const` whenever possible.

```C++
// Correct
inline constexpr auto kPi = 3.14159265359;


// INCORRECT, can be constexpr
const auto kPi = 3.14159265359;
```

**Rule 4.9.8** Use `enum class` instead of `enum`, unless absolutely needed.

```C++
// Correct
enum class TableType {
    kDisk,
    kMemory,
    kMax
};


// INCORRECT
enum TableType {
    TableType_Disk,
    TableType_Memory,
    TableType_Max
};
```

### 4.10 Whitespaces

**Rule 4.10.1** Conventional operators must be surrounded by a space character.

```C++
// Correct
a = (b + c) * d;

// INCORRECT
a=(b+c)*d
```

**Rule 4.10.2** C and C++ reserved words, except those which have function-call-like parametrs,
must be followed by a white space.

```C++
// Correct
while (true) {
    // ...
}

template <class T>
class MyClass {
public:
    // Correct: no whitespace after "noexcept" because it looks like function call
    explicit MyClass(const T& src) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : m_value(src)
    {
    }

private:
    T m_value;
};


// INCORRECT - no whitespace after "while"
while(true) {
    // ...
}
```

**Rule 4.10.3** Commas must be followed by a white space.

```C++
// Correct
doSomething(a, b, c, d);

// INCORRECT
doSomething(a,b,c,d);
```

**Rule 4.10.4** Colons must be surrounded by white space, except in the `case` labels and `default`
clause of the `switch` operator.

```C++
// Correct
a = b ? f1() : f2();

// Correct
case 100:
default:

// INCORRECT
a = b ? f1():f2();

// INCORRECT
case 100 :
default :
```

**Rule 4.10.5** Question mark `?` in the ternary operator must be surrounded by white space.

```C++
// Correct
a = b ? f1() : f2();

// INCORRECT
a = b?f1() : f2();
```

**Rule 4.10.6** Semicolons in for statments should be followed by a space character.

```C++
for (i = 0; i < 10; i++) {
    // ...
}

// INCORRECT
for(i=0;i<10;i++) {
    // ...
}
```

**Rule 4.10.7** Functional calls must not be followed by a whitespace.

```C++
// Correct
doSomething(parameter1, parameter2);

// INCORRECT
doSomething (parameter1, parameter2);
```

**Rule 4.10.8** Trailing whitespaces must be eliminated.

**Rule 4.10.9** Logical units within a block should be separated by one blank line.

```C++
Matrix4x4 matrix = new Matrix4x4();

double cosAngle = Math.cos(angle);
double sinAngle = Math.sin(angle);

matrix.setElement(1, 1,  cosAngle);
matrix.setElement(1, 2,  sinAngle);
matrix.setElement(2, 1, -sinAngle);
matrix.setElement(2, 2,  cosAngle);

multiply(matrix);
```

**Rule 4.10.10** Methods should be separated by a single blank line.

**Rule 4.10.11** Alignment is not required. Do not waste your time for alignment,
because it will be disacarded by automatic code formatting with clang-format.

```C++
// DO NOT WASTE YOUR TIME FOR THIS
if      (a == lowValue)    compueSomething();
else if (a == mediumValue) computeSomethingElse();
else if (a == highValue)   computeSomethingElseYet();
```

### 4.11 Comments

**Rule 4.11.1** All comments should be written in English.

**Rule 4.11.2** Each type declaration must have Javadoc-style documentation comment.

**Rule 4.11.3** Each template type declaration Javadoc-like comment must have `@tparam`
section for the each template parameter.

**Rule 4.11.4** Each function declaration must have Javadoc-style documentation comment,
containing following items strictly in the listed below order:

- First line: short one-sentence description.
- (optional) More description lines: more detailed description.
- `@tparam` for the each template parameter (aplicable only if this is template function)
- `@param` or `@param[out]` for the each function parameter (input/output repectively).
- `@return` the return value description (applicable only if function return type is not `void`).

**Rule 4.11.5** Each global and class constant declaration must have Javadoc-style
documentation comment.

**Rule 4.11.6** Each global variable declaration must have Javadoc-style documentation comment.

**Rule 4.11.7** All documentation comments must be written in Javadoc style and be compatible
with Doxygen.

**Rule 4.11.8** Documentation comments that fit into single line, must use single line.

```C++
// Correct
/** PI value */
constexpr double kPi = 3.14159265359;

// INCORRECT - whole documntation comment can fit into a single line.
/**
 * PI value
 */
constexpr double kPi = 3.14159265359;
```

**Rule 4.11.9** Documentation comments that fit into multiple lines, must start on the second line.

```C++
// Correct
/**
 * Retuns table ID.
 * @return Table ID.
 */
auto getTableId() const noexcept
{
    return m_id;
}


// INCORRECT
/** Retuns table ID <<-- must go to the next line.
 * @return Table ID.
 */
auto getTableId() const noexcept
{
    return m_id;
}
```

**Rule 4.11.10** Tricky code should not be commented but rewritten!

**Rule 4.11.11** Use only `//` style comments in the function bodies, including multi-line comments.
This applies to the both C and C++ code. Note that C99 standartizes `//` comments.

**Rule 4.11.12** Comments should be included relative to their position in the code.

```C++
// Correct
while (true) {
    // Do something
    something();
}


// INCORRECT
while (true) {
// Do something
    something();
}
```

**Rule 4.11.13** Use TODO comments for code that is temporary, a short-term solution,
or good-enough but not perfect.

## 5. Coding Practices

### 5.1 Classes

**Rule 5.1.1** Use a `struct` only for passive objects that carry data; everything else must be a `class`.

**Note:** Structs can be compared, hashed, serialized and deserialized. But they should not do
anything more than this. If not sure, prefer to use `class`.

**Rule 5.1.2** Prefer to use a `struct` instead of a `std::pair` or `std::tuple` whenever the elements
can have meaningful names.

**Rule 5.1.3** Composition is often more appropriate than inheritance. When using inheritance,
make it public.

**Rule 5.1.4** Make classes' data members `private`, unless they are constants that has to be used
outside of the class. Data member that must be accessed in the dervied class, must be declared it
`protected`. If constant has to be used only inside the class, make such constant `private`.
If a constant must be accessed only in this and derived classes, make it `protected`.

**Rule 5.1.5** Introduce explicit destructor only if it has to do something non-default,
or the class is base class and destructor has to be virtual because class contains other
virtual member functions.

```C++
// Correct
class MyClass {
public:
    // OK
    ~MyClass()
    {
        closeConnection();
    }
};

// Correct
class MyBaseClass {
protected:
    MyBaseClass() = default;

public:
    // OK
    virtual MyBaseClass() = default;

    virtual void f() = 0;
}


// INCORRECT
class MyClass {
public:
    // Don't do this - destructor does not make any nonm-default action.
    ~MyClass()
    {
    }
};
```

**Rule 5.1.6** All constructors of base class which is not supposed to be instantiated itself,
must be declared protected.

```C++
// Correct
class Base {
protected:
    // Must be protected
    Base(int a, double b)
        : m_a(a)
        , m_b(b)
    {
    }

public:
    virtual ~Base();

    // This one surely prevents class to be instatiated
    virtual void f() = 0;
};


// INCORRECT
class Base {
public:
    // Must be protected, but it is public
    Base(int a, double b)
        : m_a(a)
        , m_b(b)
    {
    }

    virtual ~Base();

    virtual void f() = 0;
};
```

**Rule 5.1.7** Empty destructor must be marked default instead of using empty body.

```C++
// Correct
class Base {
public:
    virtual ~Base() = default;

    virtual void f() = 0;
};

// INCORRECT
class Base {
public:
    virtual ~Base()
    {
    }

    virtual void f() = 0;
};

```

**Rule 5.1.8** Empty default constructor must be marked default instead of using empty body.

```C++
// Correct
class MyClass {
public:
    MyClass() = default;
};

// INCORRECT
class MyClass {
public:
    MyClass()
    {
    }
};
```

**Rule 5.1.9** Do not allow implict type conversion via constructor. Mark one-argument constrcutors
and constructor with only first non-default arguments as `explicit`.

**Exception:** Constructors of the often used custom primitive types can allow implicit
conversion. However, this is really rare case. One notable example of that in the Siodb codebase
is the class `Variant`.

**Rule 5.1.10** Constructor initializer list must be used to initialize all class member variables
that require non-default initialization. Avoid initialization in the constructor body unless
there is abolutely no other way to do it.

**Rule 5.1.11** Member varianles of the fundamental types and types that do not have default
constuctor must be explicitly initialized in the constructor initializer list or constructor body if
initialization in the constructor initializer list is absolutely not possible.

**Rule 5.1.12** Do not call virtual methods in constructors.

**Rule 5.1.13** Avoid in constructors initialization that can fail if you can't signal
an error with exception.

**Rule 5.1.14** A class which performs internally dymanic memory allocation, and stores its result in
the member variable, must explicitly implement copy constructor and copy assignment operator
or mark them as deleted, if they are not applicable.

**Rule 5.1.15** A class which performs internally dymanic memory allocation, and stores its result in
the member variable, must explicitly implement move constructor and move assignment operator,
or mark them as deleted, if they are not applicable.

**Exception** If there is alrady destructor and/or deleted copy constructor and/or copy assignment
operator, explicit deletion of move assignment constructor and move assignment operator is optional.

**Rule 5.1.16** Define overloaded operators only if their meaning is obvious, unsurprising, and consistent
with the corresponding built-in operators. For example, use | as a bitwise- or logical-or,
not as a shell-style pipe.

**Rule 5.1.17** Define overloaded operators only on your own types. Do not define overaloded operators
for the classes from the standrd C++ library and 3rd party libraries.

**Rule 5.1.18** Do not overload &&, ||, , (comma), or unary &. Do not overload operator"", i.e.,
do not introduce user-defined literals.

**Rule 5.1.19** Don't go your own way to avoid defining operator overloads. For example, define
operators `==`, `=`, and `<<`, rather than functions like `equals()`, `copyFrom()`, and `printTo()`.

**Rule 5.1.20** Don't define operator overloads just because other libraries expect them.
For example, if your type doesn't have a natural ordering by design, but you need to store it in a `std::set`, use a custom comparator rather than overloading `operator <`.

### 5.2 Functions

**Rule 5.2.1** Prefer small and focused functions.

**Rule 5.2.2** Define functions inline only when they are small, say, 10 lines or fewer.

**Rule 5.2.3** Default arguments are allowed on non-virtual functions when the default is guaranteed
to always have the same value.

**Rule 5.2.4** Use `auto` return type for the small inline functions with 1-3 LOC in the body,
like getters and setters and simple overloaded operators. Don't use it elsewhere.

**Rule 5.2.5** Function return type must be placed on the same line as function name,
unless they both doesn't fit line length limit.

```C++
// Correct
void myFunction(int x, int y)
{
    // ...
}

// INCORRECT
void
myFunction(int x, int y)
{
    // ...
}
```

**Rule 5.2.6** Use trailing return types only where using the ordinary syntax (leading return types)
is impractical or much less readable. This should be rare case.

### 5.3 Other Language Features

**Rule 5.3.1** Use rvalue references only in the following cases:

- To define move constructors and move assignment operators.
- To define &&-qualified methods that logically "consume" `*this`, leaving it in an
  unusable or empty state. Note that this applies only to method qualifiers (which come after the closing
  parenthesis of the function signature); if you want to "consume" an ordinary function parameter,
  prefer to pass it by value.
- To support perfect forwarding, in conjunction with `std::forward`.
- To define pairs of overloads, such as one taking `Foo&&` and the other taking const `Foo&.`
  Usually the preferred solution is just to pass by value, but an overloaded pair of functions
  sometimes yields better performance and it is sometimes necessary in the generic code that needs
  to support a wide variety of types. But if you're writing more complicated code for the
  sake of performance, make sure you have evidence that it actually helps.

**Rule 5.3.2** Avoid using Run Time Type Information (RTTI) unless absolutely needed. For example,
going into type-dependent decision tree clearly indicates wrong track:

```C++
if (typeid(*data) == typeid(D1)) {
    // ...
} else if (typeid(*data) == typeid(D2)) {
    // ...
} else if (typeid(*data) == typeid(D3)) {
    // ...
```

**Rule 5.3.3** A `dynamic_cast` can be used only when the logic of a program guarantees
with high probability that a given instance of a base class is in fact an instance of a particular
derived class.

**Rule 5.3.4** Do not hand-implement an RTTI-like workarounds.

**Rule 5.3.5** Use streams where appropriate, and stick to "simple" usages. Overload `operator <<`
and `operator >>` for streaming only for types representing values, and write only the user-visible
value, not any implementation details.

**Rule 5.3.6** Always use std::size_t to express size or length of somethings. If however you are
dealing with 3rd part library which uses something else, cast to and from 3rd party library
data type.

**Rule 5.3.7** Use explicit length integer data types like `std::uint32_t` and `std::uint64_t`
declared in the header file `<cstdint>` (or `<stdint.h>` in C) instead of C++ built-in types.

**Rule 5.3.8** Use `sizeof(variable)` instead of `sizeof(Type)` whenever possible.

**Rule 5.3.9** (Future C++20) Use designated initializers only in their C++20-compliant form.

```C++
struct Point {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
};

// Correct
Point p = {
    .x = 1.0,
    .y = 2.0,
    // z will be 0.0
};
```

**Rule 5.3.10** Use lambda expressions where appropriate. Prefer explicit captures when the lambda
will escape the current scope.

**Rule 5.3.11** Mark lambda functions as `noexcept` when they are ensured to be non-throwing.
For example, this often applies to the typical case like this:

```C++
class Item {
public:
    // ...

    auto getWeight() const noexcept
    {
        return m_weight;
    }

private:
    double m_weight;
}

std::vector<Item> items;

// Sort items by weight, note the "noexcept" on the comparison lambda function.
std::sort(items.begin(), items.end(), [](const auto& left, const auto& right) noexcept {
    return left.getWeight() < right.getWeight();
});
```

**Rule 5.3.12** Avoid complicated template programming.

**Rule 5.3.13** `std::hash` can be specialized for the your own custom types.

**Rule 5.3.14** Use N3876 `hash_combine` proposal functions when implementing specialization
of the `std::hash` for your own custom type.

**Rule 5.3.15** Avoid using global aliases for namespaces and types. Well-document them
when they are used.

**Exceptions:**

- Siodb uses global namespace alias `fs` to utilize `boost::filesystem` and switch to
  `std::filesystem` without code refactoring when Siodb migrates to C++ 20.

**Rule 5.3.16** Do not use nonstandard extensions. You may use portability wrappers that
are implemented using nonstandard extensions, so long as those wrappers are provided by
a designated project-wide portability header.

## 6 Third Party Libraries

**Rule 6.1** Use only libraries approved by project core team. 

Here is list of the libraries approved so far:

- All header-only Boost libraries
- boost::log and its dependencies
- boost::asio and its dependencies
- Google Protobuf
- ANTLR 4 Runtime
- utf8cpp
- xxHash
- Google Test
- date (by Howard Hinnant, to be replaced with new `std::chrono` stuff when switching to C++20)

## 7 Other Rules

**Rule 7.1** Existing non-confirmant Siodb code should be eventually refactored to match
these guidelines, however this is low priority activity, unless such non-conformance
generates defects.

**Rule 7.2** Existing non-confirmant third-party code which is adopted into Siodb codebase,
must be refactored to match these guidelines.

## Appendix A. Useful Links

1. [RFC 2119 Key words for use in RFCs to Indicate Requirement Levels](https://www.ietf.org/rfc/rfc2119.txt)
2. [GeoSoft C++ Programming Style Guidelines](https://geosoft.no/development/cpppractice.html)
3. [GeoSoft C++ Programming Practice Guidelines](https://geosoft.no/development/cpppractice.html)
4. [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuideline)
5. [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
6. Criticism of the Google C++ Style Guide: [Google C++ Style Guide is No Good](
    https://eyakubovich.github.io/2018-11-27-google-cpp-style-guide-is-no-good/)
7. [Windows Coding Style Convention](
    https://docs.microsoft.com/en-us/windows/win32/stg/coding-style-conventions)
8. [Linux Kernel Coding Style](https://www.kernel.org/doc/html/v4.10/process/coding-style.html)
9. [Doxygen documentation system](https://www.doxygen.nl/index.html)
10. [ClangFormat code formatting utility](https://clang.llvm.org/docs/ClangFormat.html)
