Commit Guidelines
*****************

Changes are submitted as Git commits. Each commit message must contain:

* A short and descriptive subject line that is less than 72 characters,
  followed by a blank line. The subject line must include a prefix that
  identifies the subsystem being changed, followed by a colon, and a short
  title, for example:  ``doc: update wiki references to new site``.
  (If you're updating an existing file, you can use
  ``git log <filename>`` to see what developers used as the prefix for
  previous patches of this file.)

* A change description with your logic or reasoning for the changes, followed
  by a blank line.

* A Signed-off-by line, ``Signed-off-by: <name> <email>`` typically added
  automatically by using ``git commit -s``

* If the change addresses an issue, include a line of the form::

      Fixes #<issue number>.


All changes and topics sent to GitHub must be well-formed, as described above.

Commit Message Body
===================

When editing the commit message, please briefly explain what your change
does and why it's needed. A change summary of ``"Fixes stuff"`` will be rejected.

.. warning::
   An empty change summary body is not permitted. Even for trivial changes, please
   include a summary body in the commit message.

The description body of the commit message must include:

* **what** the change does,
* **why** you chose that approach,
* **what** assumptions were made, and
* **how** you know it works -- for example, which tests you ran.

Other Commit Expectations
=========================

* Commits must build cleanly when applied on top of each other, thus avoiding
  breaking bisectability.

* Commits must pass all CI checks (see `Continuous Integration`_ for more
  information)

* Each commit must address a single identifiable issue and must be
  logically self-contained. Unrelated changes should be submitted as
  separate commits.

* You may submit pull request RFCs (requests for comments) to send work
  proposals, progress snapshots of your work, or to get early feedback on
  features or changes that will affect multiple areas in the code base.

* When major new functionality is added, tests for the new functionality MUST be
  added to the automated test suite. All new APIs MUST be documented and tested
  and tests MUST cover at least 80% of the added functionality using the code
  coverage tool and reporting provided by the project.

Submitting Proposals
====================

You can request a new feature or submit a proposal by submitting an issue to
our GitHub Repository.
If you would like to implement a new feature, please submit an issue with a
proposal (RFC) for your work first, to be sure that we can use it. Please
consider what kind of change it is:

* For a Major Feature, first open an issue and outline your proposal so that it
  can be discussed. This will also allow us to better coordinate our efforts,
  prevent duplication of work, and help you to craft the change so that it is
  successfully accepted into the project. Providing the following information
  will increase the chances of your issue being dealt with quickly:

  * Overview of the Proposal
  * Motivation for or Use Case
  * Design Details
  * Alternatives
  * Test Strategy

* Small Features can be crafted and directly submitted as a Pull Request.

Identifying Contribution Origin
===============================

When adding a new file to the tree, it is important to detail the source of
origin on the file, provide attributions, and detail the intended usage. In
cases where the file is an original to mJackets, the commit message should
include the following ("Original" is the assumption if no Origin tag is
present)::

    Origin: Original

In cases where the file is imported from an external project, the commit
message shall contain details regarding the original project, the location of
the project, the SHA-id of the origin commit for the file, the intended
purpose, and if the file will be maintained as a part of mJackets,
(whether or not mJackets will contain a localized branch or if
it is a downstream copy).

For example, a copy of a locally maintained import::

    Origin: Contiki OS
    License: BSD 3-Clause
    URL: http://www.contiki-os.org/
    commit: 853207acfdc6549b10eb3e44504b1a75ae1ad63a
    Purpose: Introduction of networking stack.
    Maintained-by: mJackets

For example, a copy of an externally maintained import::

    Origin: Tiny Crypt
    License: BSD 3-Clause
    URL: https://github.com/01org/tinycrypt
    commit: 08ded7f21529c39e5133688ffb93a9d0c94e5c6e
    Purpose: Introduction of TinyCrypt
    Maintained-by: External

.. _contribute_non-Apache:

Contributing non-Apache 2.0 licensed components
***********************************************

Importing code into mJackets from other projects that use a license
other than the Apache 2.0 license needs to be fully understood in
context and approved by the STM32 Development comittee. The comittee will
automatically reject licenses that have not been approved by the `Open Source
Initiative (OSI)`_.

.. _Open Source Initiative (OSI):
   https://opensource.org/licenses/alphabetical

Submission and review process
=============================

All contributions to mJackets are submitted through GitHub
pull requests (PR) following mJackets :ref:`Contribution workflow`.

Before you begin working on including a new component to mJackets
(Apache-2.0 licensed or not), you should start up a conversation
in the #stm32-dev slack channel to see what the community thinks 
about the idea.  Maybe there's someone else working on something 
similar you can collaborate with, or a different approach may make 
the new component unnecessary.

If the conclusion is that including a new component is the best
solution, and this new component uses a license other than Apache-2.0,
these additional steps must be followed:

#. Complete a README for your code component and add it to your source
   code pull request (PR).  A recommended README template can be found in
   :file:`doc/contribute/code_component_README` (and included
   `below`_ for reference)

#. The STM32 Development comittee will evaluate the code
   component README as part of the PR commit and vote on accepting it 
   using the GitHub PR review tools.

   - If rejected by the comittee, the contributor will be notified and 
     the PR will be closed.

   - If there are no objections, the matter is closed. Approval can be
     accelerated by unanimous approval of the comitteee before the 
     review time is up.

.. note::

   External components not under the Apache-2.0 license **cannot** be
   included in a mJackets release without approval of the STM32 Development
   comittee.

.. _below:

Code component README template
==============================

.. literalinclude:: code_component_README