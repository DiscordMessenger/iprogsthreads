#ifndef _IPROG_W32CONSTANTS_
#define _IPROG_W32CONSTANTS_

namespace iprog {

// NOTE: I hate having to do this, and I know I shouldn't, but I doubt
// that Windows will change these at all.

constexpr uint32_t __infinite = 0xFFFFFFFF;    // INFINITE
constexpr uint32_t __wait_failed = 0xFFFFFFFF; // WAIT_FAILED
constexpr uint32_t __wait_object_0 = 0;        // WAIT_OBJECT_0
constexpr uint32_t __wait_timeout = 258;       // WAIT_TIMEOUT
constexpr uint32_t __wait_abandoned = 0x80;    // WAIT_ABANDONED

} // namespace iprog

#endif//_IPROG_W32CONSTANTS_