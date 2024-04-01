#pragma once

#include <system_error>

extern "C" {
#include <fcntl.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
}

#include "Rflags.h"

class VCPU {
public:
  VCPU() = default;

  VCPU(int fd) : fd{fd} {}

  ~VCPU() {
    if (*this) {
      close(fd);
    }
  }

  VCPU(const VCPU&) = delete;
  VCPU& operator=(const VCPU&) = delete;

  VCPU(VCPU&& other) noexcept : fd(other.fd) {
    other.fd = -1;
  }

  VCPU& operator=(VCPU&& other) noexcept {
    std::swap(fd, other.fd);
    if (other.fd != -1) {
      close(other.fd);
      other.fd = -1;
    }
    return *this;
  }

  void run() {
    int ret = ioctl(fd, KVM_RUN, 0);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  kvm_sregs get_sregs() {
    kvm_sregs sregs;
    int ret = ioctl(fd, KVM_GET_SREGS, &sregs);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return sregs;
  }

  void set_sregs(const kvm_sregs& sregs) {
    int ret = ioctl(fd, KVM_SET_SREGS, &sregs);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  kvm_regs get_regs() {
    kvm_regs regs;
    int ret = ioctl(fd, KVM_GET_REGS, &regs);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return regs;
  }

  void set_regs(const kvm_regs& regs) {
    int ret = ioctl(fd, KVM_SET_REGS, &regs);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  operator bool() const {
    return fd != -1;
  }

  int get_fd() const {
    return fd;
  }

private:
  int fd = -1;
};

class VM {
public:
  VM() = default;

  VM(int fd) : fd{fd} {}

  ~VM() {
    if (*this) {
      close(fd);
    }
  }

  VM(const VM&) = delete;
  VM& operator=(const VM&) = delete;

  VM(VM&& other) noexcept : fd(other.fd) {
    other.fd = -1;
  }

  VM& operator=(VM&& other) noexcept {
    std::swap(fd, other.fd);
    if (other.fd != -1) {
      close(other.fd);
      other.fd = -1;
    }
    return *this;
  }

  void set_user_memory_region(const kvm_userspace_memory_region& region) {
    int ret = ioctl(fd, KVM_SET_USER_MEMORY_REGION, &region);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  VCPU create_vcpu(int vcpuid) {
    int ret = ioctl(fd, KVM_CREATE_VCPU, vcpuid);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    if (fcntl(ret, F_SETFD, FD_CLOEXEC) == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return VCPU(ret);
  }

  operator bool() const {
    return fd != -1;
  }

  int get_fd() const {
    return fd;
  }

private:
  int fd = -1;
};

class KVM {
public:
  KVM() {
    fd = open("/dev/kvm", O_CLOEXEC);
  };

  ~KVM() {
    if (*this) {
      close(fd);
    }
  }

  KVM(const KVM&) = delete;
  KVM& operator=(const KVM&) = delete;

  KVM(KVM&& other) noexcept : fd(other.fd) {
    other.fd = -1;
  }

  KVM& operator=(KVM&& other) noexcept {
    std::swap(fd, other.fd);
    if (other.fd != -1) {
      close(other.fd);
      other.fd = -1;
    }
    return *this;
  }

  int get_api_version() {
    int ret = ioctl(fd, KVM_GET_API_VERSION, 0);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return ret;
  }

  VM create_vm() {
    int ret = ioctl(fd, KVM_CREATE_VM, 0 /* KVM_X86_DEFAULT_VM */);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    if (fcntl(ret, F_SETFD, FD_CLOEXEC) == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return VM(ret);
  }

  int get_vcpu_mmap_size() {
    int ret = ioctl(fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (ret == -1) {
      throw std::system_error(errno, std::generic_category());
    }
    return ret;
  }

  operator bool() const {
    return fd != -1;
  }

  int get_fd() const {
    return fd;
  }

private:
  int fd = -1;
};

class KVMRun {
public:
  KVMRun() = default;

  KVMRun(KVM& kvm, VCPU& vcpu) {
    vcpu_run_size = kvm.get_vcpu_mmap_size();
    data = (kvm_run*)mmap(0, vcpu_run_size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, vcpu.get_fd(), 0);
    if (data == MAP_FAILED) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  ~KVMRun() {
    if (data != nullptr) {
      munmap(data, vcpu_run_size);
    }
  }

  KVMRun(const KVMRun&) = delete;
  KVMRun& operator=(const KVMRun&) = delete;

  KVMRun(KVMRun&& other) noexcept : vcpu_run_size(other.vcpu_run_size), data(other.data) {
    other.data = nullptr;
  }

  KVMRun& operator=(KVMRun&& other) noexcept {
    std::swap(vcpu_run_size, other.vcpu_run_size);
    std::swap(data, other.data);
    if (other.data != nullptr) {
      munmap(other.data, other.vcpu_run_size);
      other.data = nullptr;
    }
    return *this;
  }

  kvm_run* get() {
    return data;
  }

  void* io_data() {
    return static_cast<void*>(reinterpret_cast<char*>(data) + data->io.data_offset);
  }

private:
  int vcpu_run_size;
  kvm_run* data = nullptr;
};

inline auto format_as(const kvm_regs& regs) {
  return fmt::format(
      "rax = {:#018x}\n"
      "rbx = {:#018x}\n"
      "rcx = {:#018x}\n"
      "rdx = {:#018x}\n"
      "rsi = {:#018x}\n"
      "rdi = {:#018x}\n"
      "rbp = {:#018x}\n"
      "rsp = {:#018x}\n"
      "r8 =  {:#018x}\n"
      "r9 =  {:#018x}\n"
      "r10 = {:#018x}\n"
      "r11 = {:#018x}\n"
      "r12 = {:#018x}\n"
      "r13 = {:#018x}\n"
      "r14 = {:#018x}\n"
      "r15 = {:#018x}\n"
      "rip = {:#018x}\n"
      "rflags = {} ({:#018x})\n",
      regs.rax, regs.rbx, regs.rcx, regs.rdx, regs.rsi, regs.rdi, regs.rbp,
      regs.rsp, regs.r8, regs.r9, regs.r10, regs.r11, regs.r12, regs.r13,
      regs.r14, regs.r15, regs.rip, Rflags{regs.rflags}, regs.rflags);
}

