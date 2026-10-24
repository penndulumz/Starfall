# Domain Docs

This repo uses a single-context layout: `GLOSSARY.md` at the repo root
and architecture decision records under `docs/adr/`.

## Before exploring, read these

- Root `GLOSSARY.md` for domain terminology.
- ADRs in `docs/adr/` that touch the area being explored.

If these files do not exist, proceed silently. Domain documentation is
created lazily by `/domain-modeling` when terms or decisions are resolved.

## Use the glossary's vocabulary

When naming domain concepts in issues, proposals, hypotheses, or tests,
use the terms defined in `GLOSSARY.md` and respect its avoided synonyms.

If a needed concept is missing, reconsider whether it fits the project's
language or note the gap for `/domain-modeling`.

## Flag ADR conflicts

Explicitly identify any existing ADR that a proposal contradicts and
explain why the decision should be reopened.
