#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

struct CR3 {
  enum {
    PWT = 1 << 3,
    PCD = 1 << 4,
  };

  constexpr bool get_pwt() const {
    return value & PWT;
  }

  constexpr CR3& set_pwt() {
    value |= PWT;
    return *this;
  }

  constexpr CR3& clear_pwt() {
    value &= ~PWT;
    return *this;
  }

  constexpr bool get_pcd() const {
    return value & PCD;
  }

  constexpr CR3& set_pcd() {
    value |= PCD;
    return *this;
  }

  constexpr CR3& clear_pcd() {
    value &= ~PCD;
    return *this;
  }

  constexpr std::uint64_t get_page_directory_base() const {
   return value & ~std::uint64_t{0x1111'1111'1111};
  }

  constexpr CR3& set_page_directory_base(std::uint64_t pdb) {
    if ((pdb & 0x1111'1111'1111) != 0) {
      throw std::runtime_error(fmt::format("PDB {:#018x} is not 12 bit aligned", pdb));
    }
    value = pdb | (value & 0x1111'1111'1111);
    return *this;
  }

  constexpr operator std::uint64_t() const {
    return value;
  }

  std::uint64_t value = 0;
};

inline std::string format_as(const CR3& cr3) {
  std::string result;

  if (cr3.get_pwt()) {
    result += "PWT ";
  }
  if (cr3.get_pcd()) {
    result += "PCD ";
  }

  result += fmt::format("PBD={:#010x}", cr3 & ~std::uint64_t{0x1111'1111'1111});

  return result;
}
