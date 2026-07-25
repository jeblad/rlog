# rlog

A small single header library for combined reporting and logging

*This library started out by being several files for logging and reporting in a command line app that didn't need interactions, it was strictly reporting its states. After writing double sets of messages for a while I ended with this.*

## Overview

`rlog` is a specialized reporting framework for non-interactive command-line utilities that must synchronize status updates across both the user's terminal and the system logger (`syslog`). It provides a streamlined solution to the "dual-reporting" problem, where message content and localization requirements differ significantly depending on the output destination.

Terminal reports are localized using `gettext` to provide a user-friendly experience in the operator's native language, assuming the context of the active session. Meanwhile, syslog entries remain strictly in English to preserve searchability for system administrators. To compensate for the lack of session context in logs, these entries are automatically enriched with a hierarchical, dynamic context stack. Through a single macro call, `rlog` handles pluralization and domain selection, ensuring that technical logs remain precise while user output remains accessible.

## Features

* **Dual-Destination Reporting:** Routes messages to both the terminal and `syslog` with a single macro call, ensuring consistency between user feedback and system logs.
* **Intelligent Localization:** Leverages GNU Gettext to provide localized terminal output while strictly maintaining English for system logs to ensure searchability.
* **Modern C++20 Formatting:** Fully integrates with `std::format` and `std::vformat` for high-performance, type-safe string formatting.
* **Optional Context Tracing:** Provides diagnostic "entering" and "leaving" messages for `ContextGuard` scopes, simplifying the debugging of hierarchical logic.
* **Hierarchical Context Stack:** Automatically enriches `syslog` entries with a dynamic stack of context elements (e.g., command name, dataset ID, function name), managed via RAII guards.
* **Robust Pluralization:** Comprehensive support for complex plural forms through integrated `ngettext` and `dngettext` wrappers.
* **Extensible Domains:** Designed for modular architectures, allowing plugins or extensions to define their own translation domains independently of the host.
* **Header-Only:** Lightweight and easy to integrate into existing projects without complex build-time dependencies.

## Limitations

* **C++20 Standard:** Requires a modern compiler with full support for C++20 features, specifically `std::format` and the `__VA_OPT__` macro.
* **Gettext Dependency:** Relies on the presence of `libintl` (GNU Gettext) for all localization and translation features.
* **Static Reporting Level:** The terminal reporting threshold is managed via a static internal variable, which may be restrictive for applications requiring isolated reporting configurations across different threads.
* **Thread-Local State:** The context stack is `thread_local`, which is ideal for multithreaded agents but requires care if context needs to be manually shared across thread boundaries.

## Usage

`rlog` is a header-only library requiring C++20. To use it, simply include the header. Context for syslog entries is managed dynamically using a stack.

### Basic Setup

```cpp
#include <rlog/rlog.hpp>

int main() {
    // RAII guards push context elements onto the stack
    rlog::ContextGuard app_ctx("my-app");

    // Set terminal reporting threshold (default is LOG_NOTICE)
    rlog::openreport(LOG_NOTICE);

    // This will be logged as "my-app: Service initialized"
    NOTICE_("Service initialized");
    
    // This will not be logged (INFO is below NOTICE)
    INFO_FMT_("Connected to endpoint: {0}", "127.0.0.1");
    
    {
        // RAII guard for a sub-task
        rlog::ContextGuard task_ctx("fancy-1");
        
        // This will be logged as "my-app:fancy-1: Service doing something fancy"
        NOTICE_("Service doing something fancy");
    } // task_ctx goes out of scope, context is popped
    
    // Back to "my-app: Service doing boring task"
    NOTICE_("Service doing boring task");
}
```

### Dual-Path Localization

The library ensures that **syslog** always receives the original English strings (critical for searchability and automated log analysis), while the **terminal** output is localized using `gettext`.

```cpp
// Standard localized string
ERROR_("Access denied");

// Localized plural forms
size_t count = 3;
NOTICE_N_("Found one file", "Found several files", count);

// Formatted localized plural forms
INFO_NFMT_("Processed {0} byte", "Processed {0} bytes", count, count);
```

### Plugin and Domain Support

For extensions or plugins that use their own translation domain, define `RLOG_GETTEXT` and `RLOG_NGETTEXT`:

```cpp
#define RLOG_GETTEXT(msg) dgettext("my-plugin", msg)
#define RLOG_NGETTEXT(s, p, n) dngettext("my-plugin", s, p, n)
#include <rlog/rlog.hpp>
```

### Message Extraction

Extracting strings for translation with `xgettext` requires the keyword and format-flag arguments matching the `rlog` macros. Rather than hand-maintaining that list here (it drifts out of sync with `rlog.hpp.in` otherwise), read the canonical, up-to-date list from the `rlog` CMake target's `RLOG_XGETTEXT_ARGS` property:

```cmake
get_target_property(RLOG_XGETTEXT_ARGS rlog::rlog RLOG_XGETTEXT_ARGS)

add_custom_target(pot
    COMMAND xgettext --from-code=UTF-8 --language=C++
        ${RLOG_XGETTEXT_ARGS}
        -o messages.pot main.cpp
    VERBATIM
)
```

`RLOG_XGETTEXT_ARGS` covers every macro (`_`, `N_`, `fmt_`, `nfmt_`, the per-level `EMERGENCY_` through `DEBUG_` variants, and `USER_`/`USER_FMT_`) along with the `--flag` entries needed to validate their format placeholders.

## Building and Testing

To build and test `rlog` as a standalone library, follow these steps:

1. **Clone the repository:**

    ```bash
    git clone git@github.com:jeblad/rlog.git
    cd rlog
    ```

2. **Configure the project:**

    ```bash
    cmake -B build
    ```

3. **Build and run tests:**

    ```bash
    cmake --build build
    ctest --test-dir build
    ```

### Versioning Workflow

This project uses a git `pre-commit` hook to keep the `rlog.hpp` version string synchronized with Git tags and commit hashes. 
The hook is automatically installed into your local `.git/hooks` directory when you run the `cmake` configuration command. 
This ensures that every commit contains an updated and tested header file.

## License

This project is licensed under the **GNU General Public License v3.0 Only**. See the `LICENSE` file for details.

## Acknowledgements

*Created with assistance from AI tools (Gemini 2.5, 3.0, and 3.1, in both Flash and Pro versions) across all parts of this work.*

This project was developed independently, with no external financial or institutional support other than the AI tools mentioned. The views and conclusions contained herein are those of the author(s) and should not be interpreted as representing the official policies or endorsements, either expressed or implied, of any external agency or entity.
