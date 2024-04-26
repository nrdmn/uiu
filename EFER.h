#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <string>

struct EFER {
  enum {
    SCE = 1 << 0,
    LME = 1 << 8,
    LMA = 1 << 10,
    NXE = 1 << 11,
  };

  constexpr bool get_sce() const {
    return value & SCE;
  }

  constexpr void set_sce() {
    value |= SCE;
  }

  constexpr bool get_lme() const {
    return value & LME;
  }

  constexpr void set_lme() {
    value |= LME;
  }

  constexpr bool get_lma() const {
    return value & LMA;
  }

  constexpr void set_lma() {
    value |= LMA;
  }

  constexpr bool get_nxe() const {
    return value & NXE;
  }

  constexpr void set_nxe() {
    value |= NXE;
  }

  constexpr operator std::uint64_t() const {
    return value;
  }

  std::uint64_t value = 0;
};

inline std::string format_as(const EFER& efer) {
  std::string result;

  if (efer.get_sce()) {
    result += "SCE ";
  }
  if (efer.get_lme()) {
    result += "LME ";
  }
  if (efer.get_lma()) {
    result += "LMA ";
  }
  if (efer.get_nxe()) {
    result += "NXE ";
  }

  if (result.size() > 0) {
    result.pop_back();
  }

  return result;
}
