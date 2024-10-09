/* date = August 6th 2024 10:51 pm */

#ifndef CONTEXT_CRACKING_H
#define CONTEXT_CRACKING_H

#if defined(_WIN32)
#define OS_WIN32
#elif defined (__linux__)
#define OS_LINUX
#elif defined(__APPLE__)
#define OS_APPLE
#endif

#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC
#else
#error This compiler is not supported
#endif

#if defined(COMPILER_CLANG)
#define TRAP() __builtin_trap()
#elif defined(COMPILER_MSVC)
#define TRAP() __debugbreak()
#elif defined(COMPILER_GCC)
#define TRAP() __builtin_trap()
#endif

#define DEFAULT_ALIGN sizeof(void *)

#define ARRAY_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define ENABLE_ASSERTS 1

#define _Assert_helper(expr)                         \
do                                               \
{                                                \
if (!(expr))                                 \
{ \
TRAP();\
}                                            \
} while (0)

#if ENABLE_ASSERTS
#define Assert(expr) _Assert_helper(expr)
#else
#define Assert(expr)
#endif

#define AssertAlways(expr) _Assert_helper(expr)

#define INVALID_CODE_PATH() _Assert_helper(0)

#define NOT_IMPLEMENTED() _Assert_helper(0)

#define Kilobytes(Value) ((uint64_t)(Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define function static
#define global static
#define local_persist static
#define read_only static const

#if defined OS_WIN32
#define export_function __declspec(dllexport)
#else
#define export_function __attribute__((visibility("default")))
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef int32_t b32;

// not super happy with this, know no other way

typedef void *(*os_reserve_fn)(u64 size);
typedef b32 (*os_commit_fn)(void *ptr, u64 size);
typedef void (*os_decommit_fn)(void *ptr, u64 size);
typedef void (*os_release_fn)(void *ptr, u64 size);

global os_reserve_fn os_reserve;
global os_commit_fn os_commit;
global os_decommit_fn os_decommit;
global os_release_fn os_release;

function u64 os_getPageSize();

struct OS_Api
{
	os_reserve_fn os_reserve;
	os_commit_fn os_commit;
	os_decommit_fn os_decommit;
	os_release_fn os_release;
};

#endif //BASE_CONTEXT_CRACKING_H