// shaderscan.h - which float constant registers (c#) does a Direct3D 9 shader read? (no Windows dependency, unit tested)
//
// The right-eye pass patches camera-dependent constants (view-projection, camera position ...) found in the shadow register file. Many of
// the matches are stale copies left behind by other shaders and are never read by the current one; patching them is pure wasted
// driver calls. Parsing the shader once, when the game creates it, tells us which registers matter.
//
// Token format (Shader Model 2.0 and 3.0, which carry an explicit instruction length):
//   version token  0xFFFE.... (vertex) / 0xFFFF.... (pixel), major version in bits 8..15
//   instruction    bits 0..15 opcode, bits 24..27 number of tokens that follow
//   comment        opcode 0xFFFE, size in DWORDs in bits 16..30
//   end            0x0000FFFF
//   parameter      bit 31 set; register number bits 0..10; type = bits 28..30 (low) | bits 11..12 (high, as bits 3..4)
//                  bit 13 = relative addressing (SM3: one extra address-register token follows the parameter token)
// Shader Model 1.x has no length field, so it is not analysed (analyse() returns false and the caller patches everything).
#pragma once
#include <cstdint>
#include <cstddef>

namespace ss {

struct ConstMask {
    uint64_t w[4] = {};          // bit n = float constant register c<n> may be read (n < 256)
    bool valid = false;          // false: not analysed, treat every register as used
    void set(int n) { if (n >= 0 && n < 256) w[n >> 6] |= (uint64_t)1 << (n & 63); }
    void setFrom(int n) { for (int i = n < 0 ? 0 : n; i < 256; ++i) set(i); }
    bool test(int n) const { return n >= 0 && n < 256 && ((w[n >> 6] >> (n & 63)) & 1); }
    // is any of c<reg> .. c<reg+count-1> read?
    bool usesAny(int reg, int count) const {
        if (!valid) return true;
        for (int i = 0; i < count; ++i) if (test(reg + i)) return true;
        return false;
    }
};

constexpr int kRegTypeConst = 2;
constexpr uint32_t kOpComment = 0xFFFE, kOpEnd = 0xFFFF, kOpDcl = 31, kOpDef = 81, kOpDefI = 82, kOpDefB = 83;

inline int regType(uint32_t tok) { return (int)(((tok & 0x70000000u) >> 28) | ((tok & 0x1800u) >> 8)); }
inline int regNum(uint32_t tok) { return (int)(tok & 0x7FFu); }

// Parse the bytecode. `maxTokens` guards against a missing END token.
inline bool analyse(const uint32_t* t, size_t maxTokens, ConstMask* out) {
    *out = ConstMask{};
    if (!t || maxTokens < 2) return false;
    const uint32_t ver = t[0];
    const uint32_t kind = ver >> 16;
    if (kind != 0xFFFE && kind != 0xFFFF) return false;            // not a shader version token
    if (((ver >> 8) & 0xFF) < 2) return false;                     // SM 1.x: no instruction length, not analysed
    size_t i = 1;
    while (i < maxTokens) {
        const uint32_t tok = t[i];
        const uint32_t op = tok & 0xFFFFu;
        if (op == kOpEnd && (tok & 0x80000000u) == 0 && tok == 0x0000FFFFu) { out->valid = true; return true; }
        if (op == kOpComment) { i += 1 + ((tok >> 16) & 0x7FFFu); continue; }
        const size_t len = (tok >> 24) & 0xFu;
        if (i + 1 + len > maxTokens) return false;
        if (op != kOpDef && op != kOpDefI && op != kOpDefB && op != kOpDcl) {      // def*: inline constants; dcl: input/sampler declarations
            for (size_t k = 1; k <= len; ++k) {
                const uint32_t p = t[i + k];
                if (!(p & 0x80000000u)) continue;                  // not a parameter token (e.g. a dcl usage token)
                if (regType(p) != kRegTypeConst) continue;
                const int n = regNum(p);
                if (p & 0x2000u) out->setFrom(n);                  // c[n + a0/aL]: any register from n on may be read
                else out->set(n);
            }
        }
        i += 1 + len;
    }
    return false;                                                  // ran off the end without an END token
}

} // namespace ss
