# AGENTS.md

## Code review standards (rework-code)

This project uses two composed standards for code-quality review passes,
selected 2026-08-05:

1. **Boswell, Dustin, and Trevor Foucher. *The Art of Readable Code*.
   O'Reilly Media, 2011.** — general readability baseline: control flow
   shape, conditional ordering, default values, argument validation, and
   similar small-scale patterns.
2. **Holzmann, Gerard J. "The Power of Ten: Rules for Developing
   Safety-Critical Code." *IEEE Computer*, June 2006.** — stricter,
   narrower safety-critical rules layered on top.

### Composition

"The Art of Readable Code" is the base. "The Power of Ten" narrows that
base further — every one of its rules is an additive constraint, none of
them loosens anything the base already recommends. Where the two would
otherwise disagree, the stricter reading (Power of Ten) wins. This default
composition is expected to settle the ordinary case without further
special-casing.

rlog is a general-purpose C++20 header-only logging/i18n library, not
safety-critical/embedded/flight/medical software — the Power of Ten layer
was opted into deliberately rather than triggered by the project's own
domain, at the user's explicit request during dispatch.

### Known unresolved conflicts

None recorded yet — no review pass has been run against this composed
standard as of this writing.

### Project-specific rule exceptions

None recorded yet.
