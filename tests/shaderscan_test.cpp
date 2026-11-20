#include "../src/shaderscan.h"
#include <cstdio>
#include <vector>
using namespace ss;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); ++fails; } } while (0)
static uint32_t param(int type, int num, uint32_t extra = 0) { return 0x80000000u | ((uint32_t)(type & 7) << 28) | ((uint32_t)(type & 0x18) << 8) | (uint32_t)num | extra; }
static uint32_t ins(uint32_t op, int len) { return op | ((uint32_t)len << 24); }
int main()
{
    // vs_3_0:  dcl_position v0 ; dcl_texcoord v1 ; m4x4 oPos, v0, c0 ; mov r0, c14 ; add r1, r0, c23 ; def c40, 1,2,-3,4 ; mov oT0, v1 ; end
    std::vector<uint32_t> v = { 0xFFFE0300u,
        ins(31, 2), 0x80000000u, param(1, 0),                                  // dcl_position v0
        ins(31, 2), 0x80010005u, param(1, 1),                                  // dcl_texcoord v1
        ins(20, 3), param(4, 0), param(1, 0), param(2, 0),                     // m4x4 oPos, v0, c0 (reads c0..c3 through one operand: c0 marked)
        ins(1, 2), param(0, 0), param(2, 14),                                  // mov r0, c14
        ins(2, 3), param(0, 1), param(0, 0), param(2, 23),                     // add r1, r0, c23
        ins(81, 5), param(2, 40), 0x3F800000u, 0x40000000u, 0xC0400000u, 0x40800000u,   // def c40, 1, 2, -3, 4   (a float with the top bit set)
        ins(1, 2), param(6, 0), param(1, 1),                                   // mov oT0, v1
        0x0000FFFFu };
    ConstMask m;
    CHECK(analyse(v.data(), v.size(), &m) && m.valid);
    CHECK(m.test(0) && m.test(14) && m.test(23));
    CHECK(!m.test(40) && !m.test(1) && !m.test(15) && !m.test(157));            // def'd constants and unrelated registers are not "read from the device"
    CHECK(m.usesAny(0, 4) && m.usesAny(14, 1) && !m.usesAny(150, 8) && !m.usesAny(24, 100));

    // relative addressing and comments: mov r0, c[a0.x + 40]  -> everything from c40 up may be read
    std::vector<uint32_t> r = { 0xFFFE0300u,
        (uint32_t)(0xFFFE | (2u << 16)), 0x11111111u, 0x22222222u,             // comment block, two DWORDs
        ins(1, 3), param(0, 0), param(2, 40, 0x2000u), param(3, 0),            // mov r0, c40[a0.x]
        0x0000FFFFu };
    CHECK(analyse(r.data(), r.size(), &m) && m.test(40) && m.test(100) && m.test(255) && !m.test(39));

    // pixel shader 2.0, a volume-texture sampler declaration must not confuse it (its token has the type bits set)
    std::vector<uint32_t> p = { 0xFFFF0200u,
        ins(31, 2), 0xA0000000u | (4u << 27), param(10, 0),                    // dcl_volume s0 (token decodes like a const, only conservatively)
        ins(1, 2), param(8, 0), param(2, 7),                                   // mov oC0, c7
        0x0000FFFFu };
    CHECK(analyse(p.data(), p.size(), &m) && m.test(7) && !m.test(8));

    // shader model 1.x and garbage are refused (the caller then patches everything)
    std::vector<uint32_t> old = { 0xFFFE0101u, ins(1, 0), 0x0000FFFFu };
    CHECK(!analyse(old.data(), old.size(), &m) && !m.valid);
    std::vector<uint32_t> trunc = { 0xFFFE0300u, ins(1, 2), param(0, 0) };      // no END, instruction runs off the end
    CHECK(!analyse(trunc.data(), trunc.size(), &m));
    CHECK(!analyse(nullptr, 0, &m));
    ConstMask unknown; CHECK(unknown.usesAny(5, 1));                           // an unanalysed shader uses "everything"

    std::printf(fails ? "shaderscan tests FAILED (%d)\n" : "shaderscan tests passed\n", fails);
    return fails ? 1 : 0;
}
