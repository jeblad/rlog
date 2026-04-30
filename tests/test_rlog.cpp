#include <rlog/rlog.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

// Helper to capture stdout for testing report levels
struct StdoutCapture {
    std::stringstream buffer;
    std::streambuf* old;
    StdoutCapture() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~StdoutCapture() { std::cout.rdbuf(old); }
    std::string str() { return buffer.str(); }
};

void test_version() {
    std::cout << "Testing version macros..." << std::endl;
    
    // Tester verdiene i rlog.hpp mot verdiene injisert direkte fra CMake
    assert(RLOG_VERSION_MAJOR == CMAKE_EXPECTED_VERSION_MAJOR);
    assert(RLOG_VERSION_MINOR == CMAKE_EXPECTED_VERSION_MINOR);
    assert(RLOG_VERSION_PATCH == CMAKE_EXPECTED_VERSION_PATCH);
    
    assert(std::string(RLOG_VERSION) == CMAKE_EXPECTED_VERSION);
}

void test_detail_helpers() {
    std::cout << "Testing detail helpers..." << std::endl;
    
    std::string test_str = "hello";
    const char* test_char = "world";
    
    assert(std::strcmp(rlog::detail::to_c_str(test_str), "hello") == 0);
    assert(std::strcmp(rlog::detail::to_c_str(test_char), "world") == 0);
}

void test_report_levels() {
    std::cout << "Testing report levels..." << std::endl;
    
    rlog::openreport(LOG_NOTICE);
    
    {
        StdoutCapture cap;
        rlog::info("This should not appear");
        assert(cap.str().empty());
    }
    
    {
        StdoutCapture cap;
        rlog::notice("This should appear");
        assert(!cap.str().empty());
        assert(cap.str().find("This should appear") != std::string::npos);
    }
}

void test_formatting() {
    std::cout << "Testing i18n formatting (fallback path)..." << std::endl;
    
    // Note: Without a loaded .mo file, RLOG_GETTEXT returns the input string.
    // We test that the std::vformat integration works.
    
    std::string result = rlog::i18n::format("Hello {}!", "World");
    assert(result == "Hello World!");
    
    std::string n_result = rlog::i18n::nformat("{} item", "{} items", 1, 1);
    assert(n_result == "1 item");
    
    std::string n_result_p = rlog::i18n::nformat("{} item", "{} items", 5, 5);
    assert(n_result_p == "5 items");
}

void test_macros_compilation() {
    std::cout << "Testing macro expansion..." << std::endl;

    {
        rlog::ContextGuard app_ctx("test-app");
        assert(rlog::Context::instance_.get_syslog_prefix() == "test-app");

        {
            rlog::ContextGuard sub_ctx("worker-1");
            assert(rlog::Context::instance_.get_syslog_prefix() == "test-app:worker-1");

            // These call syslog. We verify they compile and handle dynamic context.
            NOTICE_("Testing notice macro");
            INFO_FMT_("Testing info fmt: {}", 42);
            ERROR_N_("Singular error", "Plural errors", 1);
        }

        assert(rlog::Context::instance_.get_syslog_prefix() == "test-app");
    }
    assert(rlog::Context::instance_.get_syslog_prefix().empty());
}

void test_context_tracing() {
    std::cout << "Testing context tracing..." << std::endl;

    // 1. Test tracing enabled at LOG_DEBUG
    rlog::openreport(LOG_DEBUG);
    rlog::opentrace(true);
    {
        StdoutCapture cap;
        {
            rlog::ContextGuard trace_ctx("trace-me");
        }
        std::string out = cap.str();
        assert(out.find("entering: trace-me") != std::string::npos);
        assert(out.find("leaving: trace-me") != std::string::npos);
    }

    // 2. Test tracing disabled
    rlog::opentrace(false);
    {
        StdoutCapture cap;
        {
            rlog::ContextGuard no_trace_ctx("no-trace");
        }
        assert(cap.str().empty());
    }

    // 3. Test tracing enabled but report level too low (LOG_INFO < LOG_DEBUG)
    rlog::opentrace(true);
    rlog::openreport(LOG_INFO);
    {
        StdoutCapture cap;
        {
            rlog::ContextGuard hidden_trace_ctx("hidden");
        }
        assert(cap.str().empty());
    }
}

int main() {
    test_version();
    test_detail_helpers();
    test_report_levels();
    test_formatting();
    test_macros_compilation();
    test_context_tracing();
    
    std::cout << "\nAll rlog tests passed!" << std::endl;
    return 0;
}