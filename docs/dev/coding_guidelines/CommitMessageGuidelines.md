# Siodb Commit Guidelines

## Introduction

This document lists requirements to commit messages code. Many of them are based on established
standards collected from a number of sources, individual experience, local requirements/needs.

The key words “MUST”, “MUST NOT”, “REQUIRED”, “SHALL”, “SHALL NOT”, “SHOULD”, “SHOULD NOT”,
“RECOMMENDED”, “MAY”, and “OPTIONAL” in this document are to be interpreted as described
in the [RFC 2119](https://www.ietf.org/rfc/rfc2119.txt).

## 1. General Rules

**Rule 1.1** All commit messages must be written in the English language.

## 2. Rules for the PR and "Main" Branch Commit Messages

**Rule 2.1** All below rules (Rule 2.2 and further) listed in this section apply only to the commits
which go to the sort of main branch (`master` etc) and to the commits in the pull request
which have to be merged. You may use free style commit messages for intermediate commits which
are to be discarded before submitting PR.

**Rule 2.2** Commits MUST be prefixed with a type, which consists of a noun, feat, fix, etc.,
followed by the OPTIONAL scope, OPTIONAL `!`, and REQUIRED terminal colon and space.

**Rule 2.3** The type feat MUST be used when a commit adds a new feature to your application
or library.

**Rule 2.4** The type fix MUST be used when a commit represents a bug fix for your application.

**Rule 2.5** A scope MAY be provided after a type. A scope MUST consist of a noun describing
a section of  the codebase surrounded by parenthesis, e.g., `fix(parser)`:

**Rule 2.6** A description MUST immediately follow the colon and space after the type/scope prefix.
The description is a short summary of the code changes, e.g.,
`fix: array parsing issue when multiple spaces were contained in string.`

**Rule 2.7** If commit relates to the issue in the issue tracker, issue ID must be the first word
in the description. For example: `feat: #52 SQL: DESCRIBE TABLE`.

**Rule 2.8** A longer commit body MAY be provided after the short description, providing additional
contextual information about the code changes. The body MUST begin one blank line
after the description.

**Rule 2.9** A commit body is free-form and MAY consist of any number of newline
separated paragraphs.

**Rule 2.11** One or more footers MAY be provided one blank line after the body.

**Rule 2.12** Each footer MUST consist of a word token, followed by either a
`:<space>` or `<space>#` separator, followed by a string value
(this is inspired by the git trailer convention).

**Rule 2.13** A footer’s token MUST use - in place of whitespace characters, e.g.,
Acked-by (this helps differentiate the footer section from a multi-paragraph body).
An exception is made for the `BREAKING CHANGE`, which MAY also be used as a token.

**Rule 2.14** A footer’s value MAY contain spaces and newlines, and parsing MUST terminate
when the next valid footer token/separator pair is observed.

**Rule 2.15** Breaking changes MUST be indicated in the type/scope prefix of a commit,
or as an entry in the footer.

**Rule 2.16** If included as a footer, a breaking change MUST consist of the uppercase text
`BREAKING CHANGE`, followed by a colon, space, and description, e.g.,
`BREAKING CHANGE: environment variables now take precedence over config files`.

**Rule 2.17** If included in the type/scope prefix, breaking changes MUST be indicated by a `!`
immediately before the `:`. If `!` is used, `BREAKING CHANGE:` MAY be ommitted
from the footer section, and the commit description SHALL be used to describe the breaking change.

**Rule 2.18** Types other than feat and fix MAY be used in your commit messages,
e.g., `docs: updated ref docs`.

**Rule 2.19** The units of information that make up Conventional Commits MUST NOT be treated
as case sensitive by implementors, with the exception of `BREAKING CHANGE` which
MUST be uppercase. `BREAKING-CHANGE` MUST be synonymous with `BREAKING CHANGE`, when used
as a token in a footer.

## Appendix A. Useful Links

1. [Conventional Commits Specification v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/)
2. [RFC 2119 Key words for use in RFCs to Indicate Requirement Levels](https://www.ietf.org/rfc/rfc2119.txt)
