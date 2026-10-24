#pragma once

#include "simulation/Run.hpp"

namespace starfall::testing {

// Scenario setup only. Tests observe outcomes through Run::view(), like production.
class RunFixture {
public:
    static RunState& state(Run& run) { return run.state_; }
};

} // namespace starfall::testing
