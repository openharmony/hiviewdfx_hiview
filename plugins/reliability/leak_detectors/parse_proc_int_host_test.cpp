#include "parse_proc_int.h"
#include <cstdio>
#include <cstdlib>

static void Expect(bool cond, const char *msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        std::exit(1);
    }
}

int main()
{
    int32_t i32 = 0;
    int64_t i64 = 0;
    uint64_t u64 = 0;
    Expect(ParseProcInt32("1", i32) && i32 == 1, "i32");
    Expect(!ParseProcInt32("", i32), "empty");
    Expect(!ParseProcInt32("2147483648", i32), "i32big");
    Expect(ParseProcInt64("1", i64) && i64 == 1, "i64");
    Expect(!ParseProcInt64("9999999999999999999", i64), "i64big");
    Expect(ParseProcU64("0", u64) && u64 == 0, "u64");
    Expect(!ParseProcU64("18446744073709551616", u64), "u64big");
    std::puts("ok");
    return 0;
}
