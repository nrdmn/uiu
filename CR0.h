#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <string>

struct CR0 {
  enum {
    PE = 1 << 0,
    MP = 1 << 1,
    EM = 1 << 2,
    TS = 1 << 3,
    ET = 1 << 4,
    NE = 1 << 5,
    WP = 1 << 16,
    AM = 1 << 18,
    NW = 1 << 29,
    CD = 1 << 30,
    PG = 1u << 31,
  };

  constexpr bool get_pe() const {
    return value & PE;
  }

  constexpr CR0& set_pe() {
    value |= PE;
    return *this;
  }

  constexpr CR0& clear_pe() {
    value &= ~PE;
    return *this;
  }

  constexpr bool get_mp() const {
    return value & MP;
  }

  constexpr CR0& set_mp() {
    value |= MP;
    return *this;
  }

  constexpr CR0& clear_mp() {
    value &= ~MP;
    return *this;
  }

  constexpr bool get_em() const {
    return value & EM;
  }

  constexpr CR0& set_em() {
    value |= EM;
    return *this;
  }

  constexpr CR0& clear_em() {
    value &= ~EM;
    return *this;
  }

  constexpr bool get_ts() const {
    return value & TS;
  }

  constexpr CR0& set_ts() {
    value |= TS;
    return *this;
  }

  constexpr CR0& clear_ts() {
    value &= ~TS;
    return *this;
  }

  constexpr bool get_et() const {
    return value & ET;
  }

  constexpr CR0& set_et() {
    value |= ET;
    return *this;
  }

  constexpr CR0& clear_et() {
    value &= ~ET;
    return *this;
  }

  constexpr bool get_ne() const {
    return value & NE;
  }

  constexpr CR0& set_ne() {
    value |= NE;
    return *this;
  }

  constexpr CR0& clear_ne() {
    value &= ~NE;
    return *this;
  }

  constexpr bool get_wp() const {
    return value & WP;
  }

  constexpr CR0& set_wp() {
    value |= WP;
    return *this;
  }

  constexpr CR0& clear_wp() {
    value &= ~WP;
    return *this;
  }

  constexpr bool get_am() const {
    return value & AM;
  }

  constexpr CR0& set_am() {
    value |= AM;
    return *this;
  }

  constexpr CR0& clear_am() {
    value &= ~AM;
    return *this;
  }

  constexpr bool get_nw() const {
    return value & NW;
  }

  constexpr CR0& set_nw() {
    value |= NW;
    return *this;
  }

  constexpr CR0& clear_nw() {
    value &= ~NW;
    return *this;
  }

  constexpr bool get_cd() const {
    return value & CD;
  }

  constexpr CR0& set_cd() {
    value |= CD;
    return *this;
  }

  constexpr CR0& clear_cd() {
    value &= ~CD;
    return *this;
  }

  constexpr bool get_pg() const {
    return value & PG;
  }

  constexpr CR0& set_pg() {
    value |= PG;
    return *this;
  }

  constexpr CR0& clear_pg() {
    value &= ~PG;
    return *this;
  }

  constexpr operator std::uint64_t() const {
    return value;
  }

  std::uint64_t value = 0;
};

inline std::string format_as(const CR0& cr0) {
  std::string result;

  if (cr0.get_pe()) {
    result += "PE ";
  }
  if (cr0.get_mp()) {
    result += "MP ";
  }
  if (cr0.get_em()) {
    result += "EM ";
  }
  if (cr0.get_ts()) {
    result += "TS ";
  }
  if (cr0.get_et()) {
    result += "ET ";
  }
  if (cr0.get_ne()) {
    result += "NE ";
  }
  if (cr0.get_wp()) {
    result += "WP ";
  }
  if (cr0.get_am()) {
    result += "AM ";
  }
  if (cr0.get_nw()) {
    result += "NW ";
  }
  if (cr0.get_cd()) {
    result += "CD ";
  }
  if (cr0.get_pg()) {
    result += "PG ";
  }

  result += fmt::format("{:#010x}", cr0 >> 32);

  return result;
}
