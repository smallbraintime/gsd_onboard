#pragma once

#include <Arduino.h>
#include <etl/array.h>
#include <etl/deque.h>
#include <etl/optional.h>

template <size_t SIZE, size_t DOF>
class DriveHistory {
   public:
    struct MovementRecord {
        int16_t movement = 0;
        uint16_t count = 1;
    };

    DriveHistory(uint16_t tolerance) : _tolerance(tolerance) {}

    void push(size_t dofIndex, int16_t movementValue) {
        if (dofIndex > SIZE)
            return;

        auto& history = _histories[dofIndex];

        if (history.full())
            history.pop_front();

        if (!history.empty()) {
            MovementRecord& lastMovement = history.back();
            if (isWithinTolerance(movementValue, lastMovement.movement)) {
                lastMovement.count++;
                return;
            }
        }

        history.push_back({movementValue});
    }

    etl::optional<int16_t> rewind(size_t dofIndex) {
        if (dofIndex > SIZE)
            return etl::nullopt;

        auto& history = _histories[dofIndex];
        if (history.empty())
            return etl::nullopt;

        MovementRecord& lastMovement = history.back();
        lastMovement.count--;

        if (!lastMovement.count)
            history.pop_back();

        return -lastMovement.movement;
    }

    void reset() {
        for (auto& history : _histories)
            history.clear();
    }

   private:
    bool isWithinTolerance(int16_t current, int16_t previous) {
        return abs(current - previous) <= static_cast<int16_t>(_tolerance);
    }

    etl::array<etl::deque<MovementRecord, SIZE>, DOF> _histories;
    const uint16_t _tolerance;
};