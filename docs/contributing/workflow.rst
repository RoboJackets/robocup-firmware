.. _contribution_workflow:

Contribution Workflow
*********************

One general practice we encourage, is to make small,
controlled changes. This practice simplifies review, makes merging and
rebasing easier, and keeps the change history clear and clean.

When contributing to mJackets, it is also important you provide as much
information as you can about your change, update appropriate documentation,
and test your changes thoroughly before submitting.

The general GitHub workflow used by mJackets developers uses a combination of
command line Git commands and browser interaction with GitHub.  As it is with
Git, there are multiple ways of getting a task done.  We'll describe a typical
workflow here:

.. _Create a Fork of mJackets:
   https://github.com/RoboJackets/mjackets-api#fork-destination-box

#. `Create a Fork of mJackets`_
   to your personal account on GitHub. (Click on the fork button in the top
   right corner of the mJackets API repo page in GitHub.)

#. On your development computer, change into the :file:`mjackets-api` folder that was
   created when you :ref:`obtained the code <get_the_code>`::

   Rename the default remote pointing to the `upstream repository
   <https://github.com/RoboJackets/mjackets-api>`_ from ``origin`` to
   ``upstream``::

     git remote rename origin upstream

   Let Git know about the fork you just created, naming it ``origin``::

     git remote add origin https://github.com/<your github id>/mjackets-api

   and verify the remote repos::

     git remote -v

   The output should look similar to::

     origin   https://github.com/<your github id>/mjackets-api (fetch)
     origin   https://github.com/<your github id>/mjackets-api (push)
     upstream https://github.com/RoboJackets/mjackets-api (fetch)
     upstream https://github.com/RoboJackets/mjackets-api (push)

#. Create a topic branch (off of master) for your work (if you're addressing
   an issue, we suggest including the issue number in the branch name)::

     git checkout master
     git checkout -b fix_comment_typo

#. Make changes, test locally, change, test, test again, ...  (Check out the
   prior chapter on `sanitycheck`_ as well).

#. When things look good, start the pull request process by adding your changed
   files::

     git add [file(s) that changed, add -p if you want to be more specific]

   You can see files that are not yet staged using::

     git status

#. Verify changes to be committed look as you expected::

     git diff --cached

#. Commit your changes to your local repo::

     git commit -s

   The ``-s`` option automatically adds your ``Signed-off-by:`` to your commit
   message. See the `Commit Guidelines`_ section for
   specific guidelines for writing your commit messages.

#. Push your topic branch with your changes to your fork in your personal
   GitHub account::

     git push origin fix_comment_typo

#. In your web browser, go to your forked repo and click on the
   ``Compare & pull request`` button for the branch you just worked on and
   you want to open a pull request with.

#. Review the pull request changes, and verify that you are opening a
   pull request for the appropriate branch. The title and message from your
   commit message should appear as well.

#. If you're working on a subsystem branch that's not ``master``,
   you may need to change the intended branch for the pull request
   here, for example, by changing the base branch from ``master`` to ``net``.

#. GitHub will assign one or more suggested reviewers (based on the
   CODEOWNERS file in the repo). If you are a project member, you can
   select additional reviewers now too.

#. Click on the submit button and your pull request is sent and awaits
   review.  Email will be sent as review comments are made, or you can check
   on your pull request at https://github.com/RoboJackets/mjackets-api/pulls.

#. While you're waiting for your pull request to be accepted and merged, you
   can create another branch to work on another issue. (Be sure to make your
   new branch off of master and not the previous branch.)::

     git checkout master
     git checkout -b fix_another_issue

   and use the same process described above to work on this new topic branch.

#. If reviewers do request changes to your patch, you can interactively rebase
   commit(s) to fix review issues.  In your development repo::

     git fetch --all
     git rebase --ignore-whitespace upstream/master

   The ``--ignore-whitespace`` option stops ``git apply`` (called by rebase)
   from changing any whitespace. Continuing::

     git rebase -i <offending-commit-id>^

   In the interactive rebase editor, replace ``pick`` with ``edit`` to select
   a specific commit (if there's more than one in your pull request), or
   remove the line to delete a commit entirely.  Then edit files to fix the
   issues in the review.

   As before, inspect and test your changes. When ready, continue the
   patch submission::

     git add [file(s)]
     git rebase --continue

   Update commit comment if needed, and continue::

     git push --force origin fix_comment_typo

   By force pushing your update, your original pull request will be updated
   with your changes so you won't need to resubmit the pull request.

   .. note:: While amending commits and force pushing is a common review model
      outside GitHub, and the one recommended by mJackets, it's not the main
      model supported by GitHub. Forced pushes can cause unexpected behavior,
      such as not being able to use "View Changes" buttons except for the last
      one - GitHub complains it can't find older commits. You're also not
      always able to compare the latest reviewed version with the latest
      submitted version. When rewriting history GitHub only guarantees access
      to the latest version.

#. If the CI run fails, you will need to make changes to your code in order
   to fix the issues and amend your commits by rebasing as described above.
   Additional information about the CI system can be found in
   `Continuous Integration`_.