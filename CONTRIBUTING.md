# Contributing

We'd love to see your contribution. Here is the process:

1. Prepare your development environement using our [instructions](docs/dev/Build.md).
2. Create an issue describing your changes or pick existing issue.
   1. If you wish, ask us our opinion in the [Siodb Slack space](https://join.slack.com/t/siodb-squad/shared_invite/zt-e766wbf9-IfH9WiGlUpmRYlwCI_28ng).
3. Fork official Siodb repository.
4. Develop your contribution in your fork on the separate branch started from latest commit on the master branch.
5. When you are ready to submit your contribution, please make sure that:
   1. Your code follows [Siodb Coding Guidelines](docs/dev/coding_guidelines/CodingGuidelines.md).
   2. You have added new or properly modified existing unit tests for the changes you've implemented.
   3. Your modified version passes all unit tests.
   4. Your mofified version passes [SQL tests](tests/sql_tests/README.md).
   5. All mentioned above tests are passing for the both debug and release builds.
6. Create a pull request. Someone from the core team will review your changes.
   1. For your first pull request, you must add a comment 
      writing "I have read the CLA Document and I hereby sign the CLA" 
      in the PR to accept our [Individual Contributor License Agreement](cla.md).
7. Update your contribution according to the core team's feedback.
8. Finally, core team approves and merges your PR.
