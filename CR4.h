#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <string>

struct CR4 {
  enum {
    VME = 1 << 0,
    PVI = 1 << 1,
    TSD = 1 << 2,
    DE = 1 << 3,
    PSE = 1 << 4,
    PAE = 1 << 5,
    MCE = 1 << 6,
    PGE = 1 << 7,
    PCE = 1 << 8,
    OSFXSR = 1 << 9,
    OSXMMEXCPT = 1 << 10,
    UMIP = 1 << 11,
    LA57 = 1 << 12,
    VMXE = 1 << 13,
    SMXE = 1 << 14,
    FSGSBASE = 1 << 16,
    PCIDE = 1 << 17,
    OSXSAVE = 1 << 18,
    KL = 1 << 19,
    SMEP = 1 << 20,
    SMAP = 1 << 21,
    PKE = 1 << 22,
    CET = 1 << 23,
    PKS = 1 << 24,
    UINTR = 1 << 25,
  };

  constexpr bool get_vme() const {
    return value & VME;
  }

  constexpr void set_vme() {
    value &= VME;
  }

  constexpr bool get_pvi() const {
    return value & PVI;
  }

  constexpr void set_pvi() {
    value |= PVI;
  }

  constexpr bool get_tsd() const {
    return value & TSD;
  }

  constexpr void set_tsd() {
    value |= TSD;
  }

  constexpr bool get_de() const {
    return value & DE;
  }

  constexpr void set_de() {
    value |= DE;
  }

  constexpr bool get_pse() const {
    return value & PSE;
  }

  constexpr void set_pse() {
    value |= PSE;
  }

  constexpr bool get_pae() const {
    return value & PAE;
  }

  constexpr void set_pae() {
    value |= PAE;
  }

  constexpr bool get_mce() const {
    return value & MCE;
  }

  constexpr void set_mce() {
    value |= MCE;
  }

  constexpr bool get_pge() const {
    return value & PGE;
  }

  constexpr void set_pge() {
    value |= PGE;
  }

  constexpr bool get_pce() const {
    return value & PCE;
  }

  constexpr void set_pce() {
    value |= PCE;
  }

  constexpr bool get_osfxsr() const {
    return value & OSFXSR;
  }

  constexpr void set_osfxsr() {
    value |= OSFXSR;
  }

  constexpr bool get_osxmmexcpt() const {
    return value & OSXMMEXCPT;
  }

  constexpr void set_osxmmexcpt() {
    value |= OSXMMEXCPT;
  }

  constexpr bool get_umip() const {
    return value & UMIP;
  }

  constexpr void set_umip() {
    value |= UMIP;
  }

  constexpr bool get_la57() const {
    return value & LA57;
  }

  constexpr void set_la57() {
    value |= LA57;
  }

  constexpr bool get_vmxe() const {
    return value & VMXE;
  }

  constexpr void set_vmxe() {
    value |= VMXE;
  }

  constexpr bool get_smxe() const {
    return value & SMXE;
  }

  constexpr void set_smxe() {
    value |= SMXE;
  }

  constexpr bool get_fsgsbase() const {
    return value & FSGSBASE;
  }

  constexpr void set_fsgsbase() {
    value |= FSGSBASE;
  }

  constexpr bool get_pcide() const {
    return value & PCIDE;
  }

  constexpr void set_pcide() {
    value |= PCIDE;
  }

  constexpr bool get_osxsave() const {
    return value & OSXSAVE;
  }

  constexpr void set_osxsave() {
    value |= OSXSAVE;
  }

  constexpr bool get_kl() const {
    return value & KL;
  }

  constexpr void set_kl() {
    value |= KL;
  }

  constexpr bool get_smep() const {
    return value & SMEP;
  }

  constexpr void set_smep() {
    value |= SMEP;
  }

  constexpr bool get_smap() const {
    return value & SMAP;
  }

  constexpr void set_smap() {
    value |= SMAP;
  }

  constexpr bool get_pke() const {
    return value & PKE;
  }

  constexpr void set_pke() {
    value |= PKE;
  }

  constexpr bool get_cet() const {
    return value & CET;
  }

  constexpr void set_cet() {
    value |= CET;
  }

  constexpr bool get_pks() const {
    return value & PKS;
  }

  constexpr void set_pks() {
    value |= PKS;
  }

  constexpr bool get_uintr() const {
    return value & UINTR;
  }

  constexpr void set_uintr() {
    value |= UINTR;
  }

  constexpr operator std::uint64_t() const {
    return value;
  }

  std::uint64_t value = 0;
};

inline std::string format_as(const CR4& cr4) {
  std::string result;

  if (cr4.get_vme()) {
    result += "VME ";
  }
  if (cr4.get_pvi()) {
    result += "PVI ";
  }
  if (cr4.get_tsd()) {
    result += "TSD ";
  }
  if (cr4.get_de()) {
    result += "DE ";
  }
  if (cr4.get_pse()) {
    result += "PSE ";
  }
  if (cr4.get_pae()) {
    result += "PAE ";
  }
  if (cr4.get_mce()) {
    result += "MCE ";
  }
  if (cr4.get_pge()) {
    result += "PGE ";
  }
  if (cr4.get_pce()) {
    result += "PCE ";
  }
  if (cr4.get_osfxsr()) {
    result += "OSFXSR ";
  }
  if (cr4.get_osxmmexcpt()) {
    result += "OSXMMEXCPT ";
  }
  if (cr4.get_umip()) {
    result += "UMIP ";
  }
  if (cr4.get_la57()) {
    result += "LA57 ";
  }
  if (cr4.get_vmxe()) {
    result += "VMXE ";
  }
  if (cr4.get_smxe()) {
    result += "SMXE ";
  }
  if (cr4.get_fsgsbase()) {
    result += "FSGSBASE ";
  }
  if (cr4.get_pcide()) {
    result += "PCIDE ";
  }
  if (cr4.get_osxsave()) {
    result += "OSXSAVE ";
  }
  if (cr4.get_kl()) {
    result += "KL ";
  }
  if (cr4.get_smep()) {
    result += "SMEP ";
  }
  if (cr4.get_smap()) {
    result += "SMAP ";
  }
  if (cr4.get_pke()) {
    result += "PKE ";
  }
  if (cr4.get_cet()) {
    result += "CET ";
  }
  if (cr4.get_pks()) {
    result += "PKS ";
  }
  if (cr4.get_uintr()) {
    result += "UINTR ";
  }

  if (result.size() > 0) {
    result.pop_back();
  }

  return result;
}
