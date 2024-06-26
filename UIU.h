#pragma once

#include <algorithm>
#include <codecvt>  // std::codecvt_utf8
#include <cstdint>
#include <cstring>
#include <fmt/format.h>
#include <locale>  // std::wstring_convert
#include <memory_resource>
#include <string>
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
  // 0x00010000 ...        Start
  // 0x000f1000 0x000f1fff PML4
  // 0x000f2000 0x000f2fff PDPT
  // ...        0x1ffffff0 Stack
  // 0x20000000 0x37ffffff Pool
  // 0x38000000 0x3fffffff App

  UIU(KVM& kvm)
      : machine(kvm),
        mbr(machine.create_ptr<void*>(0x2000'0000).get(), 0x3800'0000 - 0x2000'0000),
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
    case LocateProtocol:
      handle_io_call.operator()<LocateProtocol>(&UIU::locate_protocol);
      break;
    case InstallProtocolInterface:
      handle_io_call.operator()<InstallProtocolInterface>(&UIU::install_protocol_interface);
      break;
    case GetRNG:
      handle_io_call.operator()<GetRNG>(&UIU::get_rng);
      break;
    case SetVariable:
      handle_io_call.operator()<SetVariable>(&UIU::set_variable);
      break;
    case LocateHandleBuffer:
      handle_io_call.operator()<LocateHandleBuffer>(&UIU::locate_handle_buffer);
      break;
    default:
      std::terminate();
    }
    return IOExitStatus::Continue;
  }

  EFI_STATUS handle_protocol(EFI_HANDLE Handle, EFI_GUID* Protocol, VOID** Interface) {
    if (Handle == nullptr || Protocol == nullptr || Interface == nullptr) {
      return EFI_INVALID_PARAMETER;
    }
    if (!handle_db.contains(Handle)) {
      // This case is actually not defined by the specification
      return EFI_UNSUPPORTED;
    }
    const auto& protocol = *machine.create_ptr<EFI_GUID>((std::uint64_t)Protocol);
    if (!handle_db[Handle].contains(protocol)) {
      return EFI_UNSUPPORTED;
    }
    auto*& interface = *machine.create_ptr<void*>((std::uint64_t)Interface);
    interface = (void*)(std::uint64_t)handle_db[Handle][protocol];
    return EFI_SUCCESS;
  }

  EFI_STATUS locate_protocol(EFI_GUID* Protocol, VOID* Registration, VOID** Interface) {
    if (Registration != nullptr) {
      // not implemented
      std::terminate();
    }
    if (Protocol == nullptr || Interface == nullptr) {
      return EFI_INVALID_PARAMETER;
    }
    auto protocol = *machine.create_ptr<EFI_GUID>((std::uint64_t)Protocol);
    for (auto& [handle, guids] : handle_db) {
      if (auto it = guids.find(protocol); it != guids.end()) {
        *machine.create_ptr<void*>((std::uint64_t)Interface) = (void*)(std::uint64_t)it->second;
        return EFI_SUCCESS;
      }
    }
    return EFI_NOT_FOUND;
  }

  EFI_STATUS install_protocol_interface(EFI_HANDLE* Handle, EFI_GUID* Protocol, EFI_INTERFACE_TYPE InterfaceType, VOID* Interface) {
    if (Handle == nullptr || Protocol == nullptr) {
      return EFI_INVALID_PARAMETER;
    }
    if (InterfaceType != EFI_NATIVE_INTERFACE) {
      return EFI_INVALID_PARAMETER;
    }
    auto handle = machine.create_ptr<EFI_HANDLE>((std::uint64_t)Handle);
    if (*handle == nullptr) {
      auto [new_handle, ok] = handle_db.insert({(EFI_HANDLE)(handle_counter++), {}});
      if (!ok) {
        // should never happen
        std::terminate();
      }
      *handle = new_handle->first;
    }
    auto protocol = machine.create_ptr<EFI_GUID>((std::uint64_t)Protocol);
    auto interface = machine.create_ptr<void>((std::uint64_t)Interface);
    handle_db[*handle].insert({*protocol, interface});
    return EFI_SUCCESS;
  }

  EFI_STATUS get_variable(CHAR16* VariableName, EFI_GUID* VendorGuid, UINT32* Attributes, UINTN* DataSize, VOID* Data) {
    // Attributes is not implemented

    if (VariableName == nullptr || VendorGuid == nullptr || DataSize == nullptr) {
      return EFI_INVALID_PARAMETER;
    }
    std::u16string_view variable_name = machine.create_ptr<char16_t>((std::uint64_t)VariableName).get();
    const EFI_GUID& vendor_guid = *machine.create_ptr<EFI_GUID>((std::uint64_t)VendorGuid);
    UINTN& data_size = *machine.create_ptr<UINTN>((std::uint64_t)DataSize);
    if (auto it = variables.find(vendor_guid); it != variables.end()) {
      if (auto jt = it->second.find(std::u16string{variable_name}); jt != it->second.end()) {
        const auto& value = jt->second;
        if (data_size < value.size()) {
          return EFI_BUFFER_TOO_SMALL;
        }
        if (Data == nullptr) {
          return EFI_INVALID_PARAMETER;
        }
        auto data = machine.create_ptr<void>((std::uint64_t)Data);
        std::memcpy(data.get(), value.data(), value.size());
        return EFI_SUCCESS;
      }
    }
    return EFI_NOT_FOUND;
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
    std::u16string_view str = machine.create_ptr<char16_t>((std::uint64_t)String).get();
    fmt::print("{}", std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t>{}.to_bytes(str.begin(), str.end()));
    return EFI_SUCCESS;
  }

  EFI_STATUS get_rng(EFI_RNG_PROTOCOL*, EFI_RNG_ALGORITHM*, UINTN RNGValueLength, UINT8* RNGValue) {
    return EFI_SUCCESS;
  }

  EFI_STATUS set_variable(CHAR16* VariableName, EFI_GUID* VendorGuid, UINT32 Attributes, UINTN DataSize, VOID* Data) {
    // Attributes is not implemented

    if (VariableName == nullptr || VendorGuid == nullptr || Data == nullptr) {
      return EFI_INVALID_PARAMETER;
    }

    std::u16string_view variable_name = machine.create_ptr<char16_t>((std::uint64_t)VariableName).get();
    if (variable_name.size() == 0) {
      return EFI_INVALID_PARAMETER;
    }

    const EFI_GUID& vendor_guid = *machine.create_ptr<EFI_GUID>((std::uint64_t)VendorGuid);
    void* data = machine.create_ptr<void>((std::uint64_t)Data).get();

    if (DataSize != 0) {
      variables[vendor_guid][std::u16string{variable_name}] = std::vector<char>(static_cast<char*>(data), static_cast<char*>(data)+DataSize);
    } else {
      variables[vendor_guid].erase(std::u16string{variable_name});
    }

    return EFI_SUCCESS;
  }

  EFI_STATUS locate_handle_buffer(EFI_LOCATE_SEARCH_TYPE SearchType, EFI_GUID* Protocol, VOID* SearchKey, UINTN* NoHandles, EFI_HANDLE** Buffer) {
    if (SearchType != EFI_LOCATE_SEARCH_TYPE::ByProtocol) {
      // not implemented
      std::terminate();
    }
    const auto& protocol = *machine.create_ptr<EFI_GUID>((std::uint64_t)Protocol);
    std::vector<EFI_HANDLE> handles;
    for (const auto& [handle, protos] : handle_db) {
      if (protos.contains(protocol)) {
        handles.push_back(handle);
      }
    }
    allocate_pool(EFI_MEMORY_TYPE::EfiReservedMemoryType, sizeof(EFI_HANDLE)*handles.size(), (void**)Buffer);
    auto* buffer = machine.create_ptr<EFI_HANDLE>((std::uint64_t)machine.create_ptr<EFI_HANDLE*>((std::uint64_t)Buffer)).get();
    std::copy(handles.begin(), handles.end(), buffer);
    return EFI_SUCCESS;
  }

public:
  Machine machine;
  std::pmr::monotonic_buffer_resource mbr;
  std::pmr::unsynchronized_pool_resource upr;
  std::unordered_map<EFI_HANDLE, std::unordered_map<EFI_GUID, MachinePtr<void>>> handle_db;
  std::size_t handle_counter = 1;  // contains the next usable EFI_HANDLE
  std::unordered_map<EFI_GUID, std::unordered_map<std::u16string, std::vector<char>>> variables;
};
