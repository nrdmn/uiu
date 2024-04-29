#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <system_error>

extern "C" {
#include <sys/mman.h>
}

#include "KVM.h"

template <typename T>
struct MachinePtr {
  MachinePtr()
      : base(0), value(nullptr) {}

  MachinePtr(void* base, std::uint64_t offset)
      : base(base), value(reinterpret_cast<T*>(reinterpret_cast<std::byte*>(base) + offset)) {}

  MachinePtr(void* base, void* value)
      : base(base), value(reinterpret_cast<T*>(value)) {}

  template <typename U = T>
  const U& operator[](std::ptrdiff_t idx) const {
    return value[idx];
  }

  template <typename U = T>
  U& operator[](std::ptrdiff_t idx) {
    return value[idx];
  }

  template <typename U = T>
  const U& operator*() const {
    return *value;
  }

  template <typename U = T>
  U& operator*() {
    return *value;
  }

  const T* get() const {
    return value;
  }

  T* get() {
    return value;
  }

  explicit operator std::uint64_t() {
    return static_cast<std::uint64_t>(reinterpret_cast<std::byte*>(value) - reinterpret_cast<std::byte*>(base));
  }

  template <typename U>
  MachinePtr<U> cast() {
    return MachinePtr<U>(base, value);
  }

  MachinePtr<T> operator+(std::ptrdiff_t diff) {
    return MachinePtr<T>(base, value + diff);
  }

  MachinePtr<T> operator-(std::ptrdiff_t diff) {
    return MachinePtr<T>(base, value - diff);
  }

  void* base;
  T* value;
};

struct Machine {
  Machine(KVM& kvm) {
    vm = kvm.create_vm();
    vcpu = vm.create_vcpu(0);
    vcpu_run = KVMRun(kvm, vcpu);
    memory = {static_cast<std::byte*>(mmap(0, 0x4000'0000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED|MAP_ANONYMOUS, -1, 0)), 0x4000'0000};
    if (memory.data() == MAP_FAILED) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  ~Machine() {
    if (memory.data() != nullptr) {
      munmap(memory.data(), memory.size());
    }
  }

  Machine(const Machine&) = delete;
  Machine& operator=(const Machine&) = delete;
  Machine(Machine&&) noexcept = delete;
  Machine& operator=(Machine&&) noexcept = delete;


  template <typename T>
  MachinePtr<T> create_ptr(std::uint64_t offset) const {
    return MachinePtr<T>(memory.data(), offset);
  }

  template <typename T>
  MachinePtr<T> create_ptr(void* value) const {
    return MachinePtr<T>(memory.data(), value);
  }

  VM vm;
  VCPU vcpu;
  KVMRun vcpu_run;

  std::span<std::byte> memory;
};
