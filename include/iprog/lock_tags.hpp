//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_LOCK_TAGS_
#define _IPROG_LOCK_TAGS_

namespace iprog {

struct adopt_lock_t  { explicit adopt_lock_t()  = default; };
struct defer_lock_t  { explicit defer_lock_t()  = default; };
struct try_to_lock_t { explicit try_to_lock_t() = default; };

constexpr adopt_lock_t  adopt_lock{};
constexpr defer_lock_t  defer_lock{};
constexpr try_to_lock_t try_to_lock{};

} // namespace iprog

#endif//_IPROG_LOCK_TAGS_