#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <string>

struct Rflags {
  enum {
    CF = 1 << 0,
    PF = 1 << 2,
    AF = 1 << 4,
    ZF = 1 << 6,
    SF = 1 << 7,
    TF = 1 << 8,
    IF = 1 << 9,
    DF = 1 << 10,
    OF = 1 << 11,
    IOPL_L = 1 << 12,
    IOPL_H = 1 << 13,
    NT = 1 << 14,
    RF = 1 << 16,
    VM = 1 << 17,
    AC = 1 << 18,
    VIF = 1 << 19,
    VIP = 1 << 20,
    ID = 1 << 21,
  };

  constexpr bool get_cf() const {
    return value & CF;
  }

  constexpr Rflags& set_cf() {
    value |= CF;
    return *this;
  }

  constexpr bool get_pf() const {
    return value & PF;
  }

  constexpr Rflags& set_pf() {
    value |= PF;
    return *this;
  }

  constexpr bool get_af() const {
    return value & AF;
  }

  constexpr Rflags& set_af() {
    value |= AF;
    return *this;
  }

  constexpr bool get_zf() const {
    return value & ZF;
  }

  constexpr Rflags& set_zf() {
    value |= ZF;
    return *this;
  }

  constexpr bool get_sf() const {
    return value & SF;
  }

  constexpr Rflags& set_sf() {
    value |= SF;
    return *this;
  }

  constexpr bool get_tf() const {
    return value & TF;
  }

  constexpr Rflags& set_tf() {
    value |= TF;
    return *this;
  }

  constexpr bool get_if() const {
    return value & IF;
  }

  constexpr Rflags& set_if() {
    value |= IF;
    return *this;
  }

  constexpr bool get_df() const {
    return value & DF;
  }

  constexpr Rflags& set_df() {
    value |= DF;
    return *this;
  }

  constexpr bool get_of() const {
    return value & OF;
  }

  constexpr Rflags& set_of() {
    value |= OF;
    return *this;
  }

  constexpr int get_iopl() const {
    int iopl_l = (value & IOPL_L) ? 1 : 0;
    int iopl_h = (value & IOPL_H) ? 1 : 0;
    return (iopl_h << 1) | iopl_l;
  }

  constexpr Rflags& set_iopl(int iopl) {
    if (iopl & 1) {
      value |= IOPL_L;
    } else {
      value &= ~IOPL_L;
    }
    if (iopl & 2) {
      value |= IOPL_H;
    } else {
      value &= ~IOPL_H;
    }
    return *this;
  }

  constexpr bool get_nt() const {
    return value & NT;
  }

  constexpr Rflags& set_nt() {
    value |= NT;
    return *this;
  }

  constexpr bool get_rf() const {
    return value & RF;
  }

  constexpr Rflags& set_rf() {
    value |= RF;
    return *this;
  }

  constexpr bool get_vm() const {
    return value & VM;
  }

  constexpr Rflags& set_vm() {
    value |= VM;
    return *this;
  }

  constexpr bool get_ac() const {
    return value & AC;
  }

  constexpr Rflags& set_ac() {
    value |= AC;
    return *this;
  }

  constexpr bool get_vif() const {
    return value & VIF;
  }

  constexpr Rflags& set_vif() {
    value |= VIF;
    return *this;
  }

  constexpr bool get_vip() const {
    return value & VIP;
  }

  constexpr Rflags& set_vip() {
    value |= VIP;
    return *this;
  }

  constexpr bool get_id() const {
    return value & ID;
  }

  constexpr Rflags& set_id() {
    value |= ID;
    return *this;
  }

  constexpr operator std::uint64_t() const {
    return value;
  }

  std::uint64_t value = 2;
};

inline std::string format_as(const Rflags& rflags) {
  std::string result;

  if (rflags.get_cf()) {
    result += "CF ";
  }
  if (rflags.get_pf()) {
    result += "PF ";
  }
  if (rflags.get_af()) {
    result += "AF ";
  }
  if (rflags.get_zf()) {
    result += "ZF ";
  }
  if (rflags.get_sf()) {
    result += "SF ";
  }
  if (rflags.get_tf()) {
    result += "TF ";
  }
  if (rflags.get_if()) {
    result += "IF ";
  }
  if (rflags.get_df()) {
    result += "DF ";
  }
  if (rflags.get_of()) {
    result += "OF ";
  }
  result += fmt::format("IOPL={} ", rflags.get_iopl());
  if (rflags.get_nt()) {
    result += "NT ";
  }
  if (rflags.get_rf()) {
    result += "RF ";
  }
  if (rflags.get_vm()) {
    result += "VM ";
  }
  if (rflags.get_ac()) {
    result += "AC ";
  }
  if (rflags.get_vif()) {
    result += "VIF ";
  }
  if (rflags.get_vip()) {
    result += "VIP ";
  }
  if (rflags.get_id()) {
    result += "ID ";
  }

  result.pop_back();

  return result;
}
