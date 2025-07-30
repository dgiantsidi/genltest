#pragma once

#if defined(__cplusplus)
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#define MAYBE_UNUSED __attribute__((__unused__))
#else
#define MAYBE_UNUSED
#endif