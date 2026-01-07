#pragma once

namespace gsd {
class Log {
   public:
    static void init();
    static void debug(const char* msg);
};
}  // namespace gsd