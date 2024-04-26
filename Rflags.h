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

  constexpr void set_cf() {
    value |= CF;
  }

  constexpr bool get_pf() const {
    return value & PF;
  }

  constexpr void set_pf() {
    value |= PF;
  }

  constexpr bool get_af() const {
    return value & AF;
  }

  constexpr void set_af() {
    value |= AF;
  }

  constexpr bool get_zf() const {
    return value & ZF;
  }

  constexpr void set_zf() {
    value |= ZF;
  }

  constexpr bool get_sf() const {
    return value & SF;
  }

  constexpr void set_sf() {
    value |= SF;
  }

  constexpr bool get_tf() const {
    return value & TF;
  }

  constexpr void set_tf() {
    value |= TF;
  }

  constexpr bool get_if() const {
    return value & IF;
  }

  constexpr void set_if() {
    value |= IF;
  }

  constexpr bool get_df() const {
    return value & DF;
  }

  constexpr void set_df() {
    value |= DF;
  }

  constexpr bool get_of() const {
    return value & OF;
  }

  constexpr void set_of() {
    value |= OF;
  }

  constexpr int get_iopl() const {
    int iopl_l = (value & IOPL_L) ? 1 : 0;
    int iopl_h = (value & IOPL_H) ? 1 : 0;
    return (iopl_h << 1) | iopl_l;
  }

  constexpr void set_iopl(int iopl) {
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
  }

  constexpr bool get_nt() const {
    return value & NT;
  }

  constexpr void set_nt() {
    value |= NT;
  }

  constexpr bool get_rf() const {
    return value & RF;
  }

  constexpr void set_rf() {
    value |= RF;
  }

  constexpr bool get_vm() const {
    return value & VM;
  }

  constexpr void set_vm() {
    value |= VM;
  }

  constexpr bool get_ac() const {
    return value & AC;
  }

  constexpr void set_ac() {
    value |= AC;
  }

  constexpr bool get_vif() const {
    return value & VIF;
  }

  constexpr void set_vif() {
    value |= VIF;
  }

  constexpr bool get_vip() const {
    return value & VIP;
  }

  constexpr void set_vip() {
    value |= VIP;
  }

  constexpr bool get_id() const {
    return value & ID;
  }

  constexpr void set_id() {
    value |= ID;
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
