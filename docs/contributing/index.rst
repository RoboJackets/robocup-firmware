.. _contribution_guide:

Contribution Guide
##################

.. toctree::
   :maxdepth: 1

   api_development
   commit_guidelines
   workflow

This guide targets new members looking for a guide on contributing to the
project. This guide will focus on the overall picture of effective coding and
contributions. It is not intended to be a technical guide.

Initial Notes
*************
Contributing to open source presents a unique set of challenges, especially when
dealing with large projects. This should not be discouraging, but rather informative. Most initial
contributions will be small; it takes time to learn the project structure and any
languages and skills each particular project requires. Contributions of any size
will be greatly appreciated, and you should have realistic expectations of what
a first contribution might be (if you dont believe me, you can look at my first
contribution `here <https://github.com/RoboJackets/robocup-software/commit/6ce98fc0f8d88b6d145700779e126c0f1b99bb92#diff-71a3477f37bd5b20744e292eda2e3fbc>`_ .
It's two lines of 'code'). This guide will help you learn to navigate our code base, and
work toward an initial contribution.

Licensing
*********

Licensing is very important to open source projects. It helps ensure the
software continues to be available under the terms that the author desired.

.. _Apache 2.0 license:
   https://github.com/RoboJackets/mjackets-api/blob/master/LICENSE

.. _GitHub repo: https://github.com/RoboJackets/mjackets-api

mJackets uses the `Apache 2.0 license`_ (as found in the LICENSE file in
the project's `GitHub repo`_) to strike a balance between open
contribution and allowing you to use the software however you would like
to.  The Apache 2.0 license is a permissive open source license that
allows you to freely use, modify, distribute and sell your own products
that include Apache 2.0 licensed software.  (For more information about
this, check out articles such as `Why choose Apache 2.0 licensing`_ and
`Top 10 Apache License Questions Answered`_).

.. _Why choose Apache 2.0 licensing:
   https://www.zephyrproject.org/faqs/#1571346989065-9216c551-f523

.. _Top 10 Apache License Questions Answered:
   https://www.whitesourcesoftware.com/whitesource-blog/top-10-apache-license-questions-answered/

A license tells you what rights you have as a developer, as provided by the
copyright holder. It is important that the contributor fully understands the
licensing rights and agrees to them. Sometimes the copyright holder isn't the
contributor, such as when the contributor is doing work on behalf of a
company.

Components using other Licenses
===============================

There are some imported or reused components of mJackets that
use other licensing, as described in :ref:`mJackets_Licensing`.

Importing code into mJackets from other projects that use a license
other than the Apache 2.0 license needs to be fully understood in
context and approved my the mJackets project maintainers.

Prerequisites
*************

As a contributor, you'll want to be familiar with the mJackets project, how to
configure, install, and use it, and how to set up your development environment
as introduced in the mJackets `Getting Started Guide <../getting_started/index.rst>`.

You should be familiar with common developer tools such as Git and CMake, and
platforms such as GitHub.

If you haven't already done so, you'll need to create a (free) GitHub account
on https://github.com and have Git tools available on your development system.

Pull Request Requirements
*************************
With large projects, organization and structure can break down fairly quickly
resulting in some bad spaghetti code. We want to avoid this as much as
possible. For this reason, every pull request will be reviewed by a more senior
member of the team. If your request isn't accepted
right away, don't take it personally. Often your code may work fine, but there
are things you can add or refine. This helps keep our repository clean and will
give you valuable experience participating in a code review process.

Continuous Integration
**********************
Continuous Integration (CI) is a tool to help auto-detect problems before they
are merged into the main respository and have a chance to cause problems. Every
time you submit a Pull Request (PR), the CI tool is run and it will assign a passing
or failing mark to the request. If the CI fails, you will need to fix the error
in your code before the code review. If the reason for the failure isn't
obvious or it's a problem in the CI check itself, seek some help. The CI system
verifies several aspects of the PR:

* Git commit formatting
* Coding Style
* Sanity Check builds for multiple architectures and boards
* Documentation build to verify any doc changes

The current status of the CI run can always be found at the bottom of the
GitHub PR page, below the review status. Depending on the success or failure
of the run you will see:

* "All checks have passed"
* "All checks have failed"

Content
*******
It's important that the content of a pull request be kept clean and small. Pull
requests should be less than 1-2k lines of code. The code changes should
reflect one and only one topic (e.g do not include two bug fixes in one pull
request). Content should generally be kept to code and documentation, binary
content, such as images, may be uploaded elsewhere.

Documentation
*************
Code should be documented thoroughly. Generally speaking, you won't be here for
more than 4(ish) years. Many students will come behind you and will need to use
the code you've written.

Each class or file you create should be documented as to what it contains and
what purpose it serves.

Each function should have documentation containing it's purpose, what
parameters is takes, and what values it returns. Error handling should be
described as well: what errors will it produce, and what assumptions it makes
regarding the validation the caller performs ahead of time. If relevant, state
if the function may block for extended periods of time. If applicable, state if
the function is reentrant or accquires and releases locks.

If the overall set of code is complex and new, consider editing or adding to
the wiki.

C/C++/Python is documented using doxygen. You can view the guide for writing doxygen
comments and documentation `here <http://www.stack.nl/~dimitri/doxygen/manual/docblocks.html>`_ .

Text file types that are not supported by doxygen should still be documented
using what ever commenting style that format supports.

Style and Formatting
********************
In order to keep the code more readable, code should be formatted and styled
uniformly according to the `Google C++ Style Guide`_ . This would be difficult
to coordinate across multiple users, so we have a program that automatically
restyles the code for you. If you submit a pull request before restyling the
code, it will likely fail the CI style check. You can auto-format the code
by running `make pretty`. If you have a lot of code, you may have to run this
a few times. You can check if the style is passing by running `make checkstyle`.
If there are no errors, then you are good to go.

.. _Google C++ Style Guide:
   https://google.github.io/styleguide/cppguide.html
