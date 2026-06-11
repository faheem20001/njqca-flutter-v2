#pragma once
// ─────────────────────────────────────────────────────────────────
// emit.h  —  Flutter event bridge for NJQCA
//
// Two helpers used by NJQCA.cpp, PartTestService.h, and check headers:
//
//   nj_emit(json)        → writes one JSON line to stdout
//                          Flutter reads it via NjqcaRunner.launch()
//
//   nj_poll_ack()        → blocks until Flutter writes
//                          Temp_Data\interactive_ack.txt ("pass"/"fail")
//                          Returns true = pass, false = fail/timeout
//                          Replaces all Win32 generatePopup() calls for
//                          interactive tests (Lid, HDMI, USB, Bluetooth)
// ─────────────────────────────────────────────────────────────────

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <windows.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ── Emit one JSON event to stdout ─────────────────────────────────
inline void nj_emit(json data) {    // by-value allows direct init from {{...}}
    std::cout << data.dump() << std::endl;
    std::cout.flush();
}

// ── Poll ack file — replaces Win32 popup for interactive tests ────
// Flutter writes Temp_Data\interactive_ack.txt with "pass" or "fail"
// after the user completes the interactive screen.
// timeoutMinutes: how long to wait before treating as fail (default 10).
inline bool nj_poll_ack(
    const std::string& ackPath   = "Temp_Data\\interactive_ack.txt",
    int                timeoutMinutes = 10)
{
    using namespace std::chrono;
    auto deadline = steady_clock::now() + minutes(timeoutMinutes);

    // Delete any stale ack file before waiting
    std::remove(ackPath.c_str());

    while (steady_clock::now() < deadline) {
        std::ifstream f(ackPath);
        if (f.good()) {
            std::string result;
            f >> result;
            f.close();
            std::remove(ackPath.c_str());
            return (result == "pass");
        }
        Sleep(500);
    }
    return false; // timeout → treat as fail
}
