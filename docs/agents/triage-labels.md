# Triage Labels

The five canonical triage roles map to these tracker values.
For local Markdown issues, record the value in the `Status:` line.

| Canonical role | Tracker value | Meaning |
| --- | --- | --- |
| `needs-triage` | `needs-triage` | Maintainer needs to evaluate this issue |
| `needs-info` | `needs-info` | Waiting on reporter for more information |
| `ready-for-agent` | `ready-for-agent` | Fully specified, ready for an AFK agent |
| `ready-for-human` | `ready-for-human` | Requires human implementation |
| `wontfix` | `wontfix` | Will not be actioned |

When a skill mentions a triage role, use its tracker value.
Edit the tracker-value column to change this repo's vocabulary.
