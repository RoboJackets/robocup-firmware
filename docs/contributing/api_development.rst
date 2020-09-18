.. _api-development:

API Development
################

This is a technical guide on how to approach adding new features and functionality
to the mJackets API.

Organization
************

The API components of mJackets are organized into two directories:

- `include` - This directory contains all of the public headers for the
  mJackets API functions. When adding a new API source file to mJackets, its
  corresponding header file MUST be included in this directory.

- `api` - This directory contains the source code for any API specific
  functions and classes.

Additionally, the `drivers` directory can be used to store the source code and
include headers for any peripheral drivers. For example, if you wanted to add
a driver for a Lidar Lite V3, you would add the source code, header files, and
`CMakeList.txt` file in `drivers/sensors/lidar_lite_v3/`.

Format
******

Define Guards
=============
All header files should have :code:`#define` guards to prevent multiple inclusion.
The format of the symbol name should be <PROJECT>_<PATH>_<FILE>_H_.

To guarantee uniqueness, they should be based on the full path in a project's source tree.
For example, the file `foo/src/bar/baz.h` in project `foo` should have the
following guard:

.. code-block:: c++

    #ifndef FOO_BAR_BAZ_H_
    #define FOO_BAR_BAZ_H_

    ...

    #endif  // FOO_BAR_BAZ_H_

Naming Conventions
******************

Naming for API classes, member functions, and parameters must adhere to the Google `C++ coding standards`_

.. _C++ Coding Standards:
   https://google.github.io/styleguide/cppguide.html

General Naming Rules
====================
- Optimize for readability using names that would be clear even to people on a different team.
- Use names that describe the purpose or intent of the object. Do not worry about saving horizontal
  space as it is far more important to make your code immediately understandable by a new reader.
  Minimize the use of abbreviations that would likely be unknown to someone outside your project
  (especially acronyms and initialisms). Do not abbreviate by deleting letters within a word. As
  a rule of thumb, an abbreviation is probably OK if it's listed in Wikipedia. Generally speaking,
  descriptiveness should be proportional to the name's scope of visibility. For example, n may be a
  fine name within a 5-line function, but within the scope of a class, it's likely too vague.

Class Naming
============
All class names should adhere to PascalCase convention.

- Use upper case letters as word separators, lower case for the rest of a word
- First character in a name is upper case
- No underbars ('_')

Example: :code:`AnalogIn`

Function Naming
===============
Functions, including class member functions, use the same rule as for class names.

Accessors and mutators (get and set functions) may be named like variables.
These often correspond to actual member variables, but this is not required.

Example:

.. code-block:: c++

    class NameOneTwo
    {
        public:
            int DoIt();
            void HandleError();
        private:
            int count()
            void set_count(int count)
    }

.. code-block:: c++

    AddTableEntry()
    DeleteUrl()
    OpenFileOrDie()

Variable Names
==============
The names of variables (including function parameters) and data members are all lowercase,
with underscores between words. Data members of classes (but not structs) additionally
have trailing underscores. For instance: :code:`a_local_variable`, :code:`a_struct_data_member`,
:code:`a_class_data_member_`.

Class Data Members
++++++++++++++++++
Data members of classes, both static and non-static, are named like ordinary nonmember variables,
but with a trailing underscore.

.. code-block:: c++

    class TableInfo {
        ...
        private:
            std::string table_name_;
            static Pool<TableInfo>* pool_;
    };

Struct Data Members
+++++++++++++++++++
Data members of structs, both static and non-static, are named like ordinary nonmember variables.
They do not have the trailing underscores that data members in classes have.

.. code-block:: c++

    struct UrlTableProperties {
        std::string name;
        int num_entries;
        static Pool<UrlTableProperties>* pool;
    };

Constant Names
==============
Variables declared constexpr or const, and whose value is fixed for the duration of the program, are
named with a leading "k" followed by mixed case. Underscores can be used as separators in the rare
cases where capitalization cannot be used for separation. For example:

.. code-block:: c++

    const int kDaysInAWeek = 7;
    const int kAndroid8_0_0 = 24;  // Android 8.0.0

Type Names
==========
When possible for types based on native types make a typedef.
Typedef names should use the same naming policy as for a class with the word Type appended.
Example:

.. code-block:: c++

    typedef uint16  ModuleType;
    typedef uint32  SystemType;

Enumerated Type Names
=====================
Enumerators (for both scoped and unscoped enums) should be named like constants, not like macros.
That is, use kEnumName not ENUM_NAME.

Example:

.. code-block:: c++

    enum class UrlTableError {
        kOk = 0,
        kOutOfMemory,
        kMalformedInput,
    };

Comments
********
Comments are absolutely vital to keeping our code readable. The following rules describe
what you should comment and where. But remember: while comments are very important, the
best code is self-documenting. Giving sensible names to types and variables is much better
than using obscure names that you must then explain through comments.

When writing your comments, write for your audience: the next contributor who will need
to understand your code. Be generous — the next one may be you!

For automatically generating api documentation, it is important
that Doxygen Javadoc-style formatting is used when writing comments for functions and classes.

File Comments
=============
Start each file with license and project boilerplate. A template is provided `here
<https://github.com/RoboJackets/mjackets-api/tree/master/docs/contributing/boilerplate_template.txt>`_.

File comments describe the contents of a file. If a file declares, implements, or tests
exactly one abstraction that is documented by a comment at the point of declaration, file
comments are not required. All other files must have file comments.

If a .h declares multiple abstractions, the file-level comment should broadly describe
the contents of the file, and how the abstractions are related. A 1 or 2 sentence
file-level comment may be sufficient. The detailed documentation about individual
abstractions belongs with those abstractions, not at the file level.

Do not duplicate comments in both the .h and the .cpp. Duplicated comments diverge.

Class Comments
==============

All classes should be commented with the following at a minimum:

- Class description
- Member function comments with description, paramaters, and return values
- Public enums, typedefs, and structs comments with description

Example:

.. code-block:: c++

    /**
    *  A test class. A more elaborate class description.
    */

    class Javadoc_Test
    {
    public:

        /**
        * An enum.
        * More detailed enum description.
        */

        enum TEnum {
            TVal1, /**< enum value TVal1. */
            TVal2, /**< enum value TVal2. */
            TVal3  /**< enum value TVal3. */
            }
        *enumPtr, /**< enum pointer. Details. */
        enumVar;  /**< enum variable. Details. */

        /**
        * A constructor.
        * A more elaborate description of the constructor.
        */
        Javadoc_Test();

        /**
        * A destructor.
        * A more elaborate description of the destructor.
        */
        ~Javadoc_Test();

        /**
        * a normal member taking two arguments and returning an integer value.
        * @param a an integer argument.
        * @param s a constant character pointer.
        * @see Javadoc_Test()
        * @see ~Javadoc_Test()
        * @see testMeToo()
        * @see publicVar()
        * @return The test results
        */
        int testMe(int a,const char *s);

        /**
        * A pure virtual member.
        * @see testMe()
        * @param c1 the first argument.
        * @param c2 the second argument.
        */
        virtual void testMeToo(char c1,char c2) = 0;

        /**
        * a public variable.
        * Details.
        */
        int publicVar;

        /**
        * a function variable.
        * Details.
        */
        int (*handler)(int a,int b);
    };

Doxygen Comment Structure
=========================

.. code-block:: c++

    /**
    * A brief history of JavaDoc-style (C-style) comments.
    *
    * This is the typical JavaDoc-style C-style comment. It starts with two
    * asterisks.
    *
    * @param theory Even if there is only one possible unified theory. it is just a
    *               set of rules and equations.
    */
    void cstyle( int theory );

    /*******************************************************************************
    * A brief history of JavaDoc-style (C-style) banner comments.
    *
    * This is the typical JavaDoc-style C-style "banner" comment. It starts with
    * a forward slash followed by some number, n, of asterisks, where n > 2. It's
    * written this way to be more "visible" to developers who are reading the
    * source code.
    *
    * Often, developers are unaware that this is not (by default) a valid Doxygen
    * comment block!
    *
    * However, as long as JAVADOC_BLOCK = YES is added to the Doxyfile, it will
    * work as expected.
    *
    * This style of commenting behaves well with clang-format.
    *
    * @param theory Even if there is only one possible unified theory. it is just a
    *               set of rules and equations.
    ******************************************************************************/
    void javadocBanner( int theory );

JavaDoc Tags
============

+---------------------------------+-------------------------------------------------------------+---------------------------------------+
| Tag & Parameter	              | Usage	                                                    | Applies to                            |
+=================================+=============================================================+=======================================+
| *@author* John Smith            | Describes an author.	                                    | Class, Interface, Enum                |
+---------------------------------+-------------------------------------------------------------+---------------------------------------+
| *@param* name description       |	Describes a method parameter.                               | Method	                            |
+---------------------------------+-------------------------------------------------------------+---------------------------------------+
| *@return* description	          | Describes the return value.	                                | Method	                            |
+---------------------------------+-------------------------------------------------------------+---------------------------------------+
| *@throws* classname description | Describes an exception that may be thrown from this method. | Method	                            |
+---------------------------------+-------------------------------------------------------------+---------------------------------------+
| *@deprecated* description       | Describes an outdated method.                               | Class, Interface, Enum, Field, Method |
+---------------------------------+-------------------------------------------------------------+---------------------------------------+

Example:

.. code-block:: c++

    /*******************************************************************************
    * Validates a chess move.
    *
    * @param fromFile file from which a piece is being moved
    * @param fromRank rank from which a piece is being moved
    * @param toFile   file to which a piece is being moved
    * @param toRank   rank to which a piece is being moved
    * @return            true if the move is valid, otherwise false
    *******************************************************************************/
    boolean isValidMove(int fromFile, int fromRank, int toFile, int toRank) {
        // ...body
    }

    /**
    * Moves a chess piece.
    */
    void doMove(int fromFile, int fromRank, int toFile, int toRank)  {
        // ...body
    }

Online Documentation
********************

After making sure that your code is thouroughly documented according to the above
specifications, you will need to add the corresponding API reference page for
the `online documentation`_. The source files for the API documentation, written in the 
reStructured Text format, are located in the :file:`docs/api_reference` directory.

Make sure to name the documentation page the same as the source file or class you are 
documenting, and add a link to the new documentation page in the toc-tree of the 
:file:`doc/api_reference/index.rst` file. 

At a minimum, you need to add the code documentation and example usage to your
documentation page.

.. _online documentation: https://mjackets-api.readthedocs.io/en/latest

Code Documentation
==================

This is the part where all of your hard work adding comments to your code pays off!
By adding various `Breathe Directives`_, you can automatically pull in a complete
reference of your code for the end user to use. 

Below is an example for pulling in documentation for an API Class:

.. code-block:: reST

    .. doxygenclass:: <YOUR_CLASS_NAME>
        :project: mjackets-api
        :members:
 
.. _Breathe Directives: https://breathe.readthedocs.io/en/latest/directives.html

Including HAL Components
************************

You can directly use HAL components from your API source code without needing include
statements. The `mJackets.hpp` file automatically includes the appropriate HAL
drivers into the project, which are selected by the target device definition in the
build system as well as the HAL configuration file.

You will also need to make sure that the appropriate HAL driver libraries are linked with
the API to ensure that the driver source files, linker and compiler flags, and header files
are imported into the project. This can all be accomplished by adding a single line to the
`CMakeLists.txt` file in the top level mjacket-api directory. For each driver library
you want to link, add a line with the format
:code:`HAL::STM32::${FAMILY}::<your_driver_name>` in the :code:`target_link_libraries`
function. The :code:`${FAMILY}` variable will automatically be populated by the build
system, and ensure the appropriate driver is included for the target device. An example
is shown below for implementation of a few HAL drivers including the ADC, DAC, and GPIO
drivers.

.. code-block:: cmake

    target_link_libraries(API
        HAL::STM32::${FAMILY}
        HAL::STM32::${FAMILY}::ADC
        HAL::STM32::${FAMILY}::CORTEX
        HAL::STM32::${FAMILY}::DAC
        HAL::STM32::${FAMILY}::GPIO
        HAL::STM32::${FAMILY}::RCC
        CMSIS::STM32::${DEVICE}
        STM32::NoSys
    )

The below libraries are required as a bare minimum:

- :code:`HAL::STM32::${FAMILY}`
- :code:`HAL::STM32::${FAMILY}::CORTEX`
- :code:`HAL::STM32::${FAMILY}::RCC`
- :code:`CMSIS::STM32::${DEVICE}`
- :code:`STM32::NoSys`

Peripheral Drivers
******************

As mentioned in the Organization section above, peripheral drivers
should be stored in an appropriate category with the folder structure
being `drivers/<category>/<driver_name>/`. Under that directory you
will need three files at a minimum.

- C++ source code file
- C++ header file
- CMakeList.txt build system file

The build system will generate a library for each driver. A driver can
be included in an API function or application code by including the
driver header file and linking with the library in the appropriate
API or application CMakeLists.txt file. For example, if you want to use
a Lidar Lite V3 driver in your application code, your application
CMakeList.txt file would need to include the following:

.. code-block:: c++

    find_package(DRIVERS COMPONENTS SENSORS REQUIRED)

    ...

    target_link_libraries(myProject
        DRIVERS::SENSORS::LIDAR_LITE_V3
    )

The CMakeLists.txt file in the driver directory will need to have the
following components to ensure compatability with the build system:

.. code-block:: cmake

    add_library(DRIVERS::<CATEGORY>::<DRIVER_NAME> INTERFACE IMPORTED
        <LIST_SOURCE_FILES_HERE>
    )

    target_include_directories(DRIVERS::<CATEGORY>::<DRIVER_NAME> INTERFACE
        .
    )

Additionally, if any HAL drivers or API functions are used, they will need
to be linked with the peripheral driver using:

.. code-block:: cmake

    target_link_libraries(DRIVERS::<CATEGORY>::<DRIVER_NAME> INTERFACE
        <Libraries_To_Link>
    )
