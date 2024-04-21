#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fmt/format.h>
#include <iostream>  // std::wcout
#include <memory_resource>
#include <unordered_map>
#include <utility>
#include <vector>

#include "API.h"
#include "Machine.h"

#define GNU_EFI_USE_MS_ABI
extern "C" {
#include <efi.h>
}

template <>
struct std::hash<EFI_GUID> {
    std::size_t operator()(const EFI_GUID& guid) const noexcept {
        std::size_t h1 = std::hash<UINT32>{}(guid.Data1);
        std::size_t h2 = std::hash<UINT16>{}(guid.Data2);
        std::size_t h3 = std::hash<UINT16>{}(guid.Data3);
        std::uint64_t Data4;
        static_assert(sizeof(Data4) == sizeof(guid.Data4));
        std::memcpy(&Data4, guid.Data4, sizeof(Data4));
        std::size_t h4 = std::hash<std::uint64_t>{}(Data4);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

inline bool operator==(const EFI_GUID& lhs, const EFI_GUID& rhs) noexcept {
  return std::memcmp(&lhs, &rhs, sizeof(EFI_GUID)) == 0;
}

inline auto format_as(const EFI_GUID& guid) {
  auto str = fmt::format("{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
      guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
      guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
      guid.Data4[6], guid.Data4[7]);
  if (guid == EFI_GUID(EFI_RNG_PROTOCOL_GUID)) {
    str += " (EFI_RNG_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID(EFI_LOADED_IMAGE_PROTOCOL_GUID)) {
    str += " (EFI_LOADED_IMAGE_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID{0xf4560cf6, 0x40ec, 0x4b4a, {0xa1, 0x92, 0xbf, 0x1d, 0x57, 0xd0, 0xb1, 0x89}}) {
    str += " (EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID{0x607f766c, 0x7455, 0x42be, {0x93, 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f}}) {
    str += " (EFI_TCG2_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID(EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID)) {
    str += " (EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID{0x982c298b, 0xf4fa, 0x41cb, {0xb8, 0x38, 0x77, 0xaa, 0x68, 0x8f, 0xb8, 0x39}}) {
    str += " (EFI_UGA_PROTOCOL_GUID)";
  } else if (guid == EFI_GUID(EFI_PCI_IO_PROTOCOL_GUID)) {
    str += " (EFI_PCI_IO_PROTOCOL_GUID)";
  }
  return str;
}

inline auto format_as(const EFI_MEMORY_TYPE& memory_type) {
  switch (memory_type) {
  case EfiReservedMemoryType:
    return "EfiReservedMemoryType";
  case EfiLoaderCode:
    return "EfiLoaderCode";
  case EfiLoaderData:
    return "EfiLoaderData";
  case EfiBootServicesCode:
    return "EfiBootServicesCode";
  case EfiBootServicesData:
    return "EfiBootServicesData";
  case EfiRuntimeServicesCode:
    return "EfiRuntimeServicesCode";
  case EfiRuntimeServicesData:
    return "EfiRuntimeServicesData";
  case EfiConventionalMemory:
    return "EfiConventionalMemory";
  case EfiUnusableMemory:
    return "EfiUnusableMemory";
  case EfiACPIReclaimMemory:
    return "EfiACPIReclaimMemory";
  case EfiACPIMemoryNVS:
    return "EfiACPIMemoryNVS";
  case EfiMemoryMappedIO:
    return "EfiMemoryMappedIO";
  case EfiMemoryMappedIOPortSpace:
    return "EfiMemoryMappedIOPortSpace";
  case EfiPalCode:
    return "EfiPalCode";
  case EfiPersistentMemory:
    return "EfiPersistentMemory";
  case EfiUnacceptedMemoryType:
    return "EfiUnacceptedMemoryType";
  case EfiMaxMemoryType:
    return "EfiMaxMemoryType";
  }
  return "<unknown>";
}
class UIU {
public:
  // MEMORY MAP
  //
  // 0x00001000 0x00001fff PML4
  // 0x00002000 0x00002fff PDPT
  // 0x00003000 0x00003fff PD
  //
  // 0x00010000 ...        Start
  //
  // 0x00100000 ...        App
  //
  // ...        0x04000000 Stack
  // 0x05000000 0x0fffffff Pool

  UIU(KVM& kvm)
      : machine(kvm),
        mbr(machine.create_ptr<void*>(0x500'0000).get(), 0x1'0000'0000 - 0x500'0000),
        upr(&mbr) {}

  MachinePtr<void> allocate(std::size_t size, std::size_t align = 8) {
    auto alloc = machine.create_ptr<std::uint64_t>(upr.allocate(size+8, std::min(8zu, align)));
    *alloc = size;
    return (alloc+1).cast<void>();
  }

  void deallocate(MachinePtr<void>&& ptr, std::size_t align = 8) {
    auto size = ptr.template cast<std::uint64_t>()[-1];
    upr.deallocate((ptr.template cast<std::uint64_t>()-1).get(), size+8, std::min(8zu, align));
  }

  template <typename T, typename... Args>
  MachinePtr<T> create_object(Args&&... args) {
    auto ptr = allocate(sizeof(T), alignof(T)).get();
    return machine.create_ptr<T>(new(ptr) T(std::forward<Args>(args)...));
  }

  template <typename T>
  MachinePtr<T> destroy_object(MachinePtr<T>&& ptr) {
    ~T(ptr.get());
    deallocate(ptr.template cast<void>());
  }

  void run() {
    for (;;) {
      machine.vcpu.run();

      kvm_run& vcpu_run = *machine.vcpu_run.get();

      if (vcpu_run.exit_reason == KVM_EXIT_IO) {
        const auto& io = vcpu_run.io;
        if (io.direction == KVM_EXIT_IO_OUT && io.port == 0xff) {
          auto status = dispatch_io_call(*this, *(short*)(machine.vcpu_run.io_data()));
          if (status == IOExitStatus::Continue) {
            continue;
          } else if (status == IOExitStatus::Exit) {
            break;
          } else {
            // trap
          }
        }
      }
      switch (vcpu_run.exit_reason) {
      case KVM_EXIT_IO:
        fmt::println("KVM_EXIT_IO");
        break;
      case KVM_EXIT_HLT:
        fmt::println("KVM_EXIT_HLT");
        break;
      case KVM_EXIT_MMIO:
        fmt::println("KVM_EXIT_MMIO");
        break;
      case KVM_EXIT_SHUTDOWN:
        fmt::println("KVM_EXIT_SHUTDOWN");
        break;
      default:
        fmt::println("unknown exit reason {}", vcpu_run.exit_reason);
        break;
      }

      auto regs = machine.vcpu.get_regs();

      fmt::println("{}", regs);
      for (int i = 0; i > -20; i--) {
        auto line = machine.create_ptr<std::uint64_t>(regs.rsp);
        fmt::println("{:#018x} {:#x}", (std::uint64_t)(line+i), line[i]);
      }
      break;
    }
  }

private:
  enum class IOExitStatus {
    Continue,
    Exit,
    Trap,
  };

  IOExitStatus dispatch_io_call(UIU& uiu, short nr) {
    auto handle_io_call = [&]<UIUAPITag T>(auto callable) {
      auto regs = machine.vcpu.get_regs();
      std::uint64_t* params = machine.create_ptr<std::uint64_t>(regs.rdx).get();
      using Fn = UIUAPIFn<T>;
      regs.rax = [&]<auto... Is>(std::index_sequence<Is...>){
        return (this->*callable)(std::tuple_element_t<Is, typename Fn::Args>(params[Is])...);
      }(std::make_index_sequence<std::tuple_size_v<typename Fn::Args>>{});
      machine.vcpu.set_regs(regs);
    };

    using enum UIUAPITag;
    switch (UIUAPITag{nr}) {
    case Trap:
      return IOExitStatus::Trap;
    case Exit:
      return IOExitStatus::Exit;
    case HandleProtocol:
      handle_io_call.operator()<HandleProtocol>(&UIU::handle_protocol);
      break;
    case GetVariable:
      handle_io_call.operator()<GetVariable>(&UIU::get_variable);
      break;
    case AllocatePool:
      handle_io_call.operator()<AllocatePool>(&UIU::allocate_pool);
      break;
    case FreePool:
      handle_io_call.operator()<FreePool>(&UIU::free_pool);
      break;
    case LocateHandle:
      handle_io_call.operator()<LocateHandle>(&UIU::locate_handle);
      break;
    case OutputString:
      handle_io_call.operator()<OutputString>(&UIU::output_string);
      break;
    default:
      std::terminate();
    }
    return IOExitStatus::Continue;
  }

  EFI_STATUS handle_protocol(EFI_HANDLE Handle, EFI_GUID* Protocol, VOID** Interface) {
    return EFI_UNSUPPORTED;
  }

  EFI_STATUS get_variable(CHAR16* VariableName, EFI_GUID* VendorGuid, UINT32* Attributes, UINTN* DataSize, VOID* Data) {
    return EFI_UNSUPPORTED;
  }

  EFI_STATUS allocate_pool(EFI_MEMORY_TYPE PoolType, UINTN Size, VOID** Buffer) {
    auto alloc = allocate(Size);
    *(machine.create_ptr<std::uint64_t>((std::uint64_t)Buffer)) = std::uint64_t{alloc};
    return EFI_SUCCESS;
  }

  EFI_STATUS free_pool(VOID* Buffer) {
    deallocate(machine.create_ptr<void>((std::uint64_t)Buffer));
    return EFI_SUCCESS;
  }

  EFI_STATUS locate_handle(EFI_LOCATE_SEARCH_TYPE SearchType, EFI_GUID* Protocol, VOID* SearchKey, UINTN* BufferSize, EFI_HANDLE* Buffer) {
    return EFI_NOT_FOUND;
  }

  EFI_STATUS output_string(EFI_SIMPLE_TEXT_OUT_PROTOCOL* This, CHAR16* String) {
    wchar_t* str = machine.create_ptr<wchar_t>((uint64_t)String).get();
    while (*str != L'\0') {
      std::wcout << *str;
      str++;
    }
    return EFI_SUCCESS;
  }

public:
  Machine machine;
  std::pmr::monotonic_buffer_resource mbr;
  std::pmr::unsynchronized_pool_resource upr;
};
