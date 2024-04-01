#include <bit>
#include <cstdint>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <vector>

#define GNU_EFI_USE_MS_ABI
extern "C" {
#include <efi.h>
#include <x86_64/pe.h>
#include <sys/mman.h>
}

#include "KVM.h"
#include "Machine.h"
#include "Rflags.h"
#include "UIU.h"


// vvvvvvvvvvvvvvvvv PE LOADER vvvvvvvvvvvvvvvvvvvvv

std::size_t load(std::ifstream ifs, void* base) {
  IMAGE_DOS_HEADER dos_header;
  if (!ifs.read(reinterpret_cast<char*>(&dos_header), sizeof(dos_header))) {
    fmt::println("Unable to read DOS header");
    std::terminate();
  }

  if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
    fmt::println("DOS header magic does not match");
    std::terminate();
  }

  ifs.seekg(dos_header.e_lfanew);

  IMAGE_NT_HEADERS pe_header;
  if (!ifs.read(reinterpret_cast<char*>(&pe_header), sizeof(pe_header))) {
    fmt::println("Unable to read PE header");
    std::terminate();
  }

  if (pe_header.Signature != IMAGE_NT_SIGNATURE) {
    fmt::println("PE header magic does not match");
    std::terminate();
  }

  ifs.seekg(dos_header.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER) + pe_header.FileHeader.SizeOfOptionalHeader);

  std::vector<IMAGE_SECTION_HEADER> section_table;
  section_table.resize(pe_header.FileHeader.NumberOfSections);
  for (std::uint32_t i = 0; i < pe_header.FileHeader.NumberOfSections; i++) {
    if (!ifs.read(reinterpret_cast<char*>(&section_table[i]), sizeof(IMAGE_SECTION_HEADER))) {
      fmt::println("Unable to read section header #{}", i);
      std::terminate();
    }
  }
  void* mapping = base;
  for (const auto& section : section_table) {
    ifs.seekg(section.PointerToRawData);
    // TODO assert that section.size_of_raw_data >= section.virtual_size
    if (!ifs.read(reinterpret_cast<char*>(mapping) + section.VirtualAddress, section.SizeOfRawData)) {
      fmt::println("loading section failed!");
      std::terminate();
    }
    //fmt::println("name: {}", (char*)section.Name);
    //fmt::println("  start: {:x}", section.VirtualAddress);
    //fmt::println("  size: {:x}", section.SizeOfRawData);
    //fmt::println("  characteristics: {:x}", section.Characteristics);
    //fmt::println("  content: {:x} {:x} {:x} ...", ((char*)mapping)[0], ((char*)mapping)[1], ((char*)mapping)[2]);
    //fmt::println("  pointer to relocations: {:x}", section.PointerToRelocations);
  }

  //fmt::println("address of entry point: {}", pe_header.OptionalHeader.AddressOfEntryPoint);
  return pe_header.OptionalHeader.AddressOfEntryPoint;
}

// ^^^^^^^^^^^^^^^^^ PE LOADER ^^^^^^^^^^^^^^^^^^^^^

void usage(int argc, char** argv) {
  const char* name = "uiu";
  if (argc > 0) {
    name = argv[0];
  }
  fmt::println("Usage: {} <efi executable>", name);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    usage(argc, argv);
    return EXIT_FAILURE;
  }
  const char* filename = argv[1];

  KVM kvm;
  if (!kvm) {
    fmt::println("kvm is not open");
    return EXIT_FAILURE;
  }

  fmt::println("api version = {}", kvm.get_api_version());

  UIU uiu(kvm);

  auto vm = kvm.create_vm();

  void* memory = uiu.machine.memory.data();

  kvm_userspace_memory_region region{
    .slot = 0,
    .guest_phys_addr = 0,
    .memory_size = 0x1'0000'0000,
    .userspace_addr = std::bit_cast<std::uint64_t>(memory),
  };
  uiu.machine.vm.set_user_memory_region(region);

  auto sregs = uiu.machine.vcpu.get_sregs();
  sregs.cs.base = 0;
  sregs.cs.selector = 0;
  {
    // See page 3041
    /*std::uint64_t pml4_addr = 0x1000;
    std::uint64_t* pml4 = (std::uint64_t*)((char*)memory + pml4_addr);
    std::uint64_t pdpt_addr = 0x2000;
    std::uint64_t* pdpt = (std::uint64_t*)((char*)memory + pdpt_addr);
    std::uint64_t pd_addr = 0x3000;
    std::uint64_t* pd = (std::uint64_t*)((char*)memory + pd_addr);
    pml4[0] = 0x7 | pdpt_addr;
    pdpt[0] = 0x7 | pd_addr;
    pd[0] = 0x87;*/
    std::uint64_t pml4_addr = 0x1000;
    std::uint64_t* pml4 = (std::uint64_t*)((char*)memory + pml4_addr);
    std::uint64_t pdpt_addr = 0x2000;
    std::uint64_t* pdpt = (std::uint64_t*)((char*)memory + pdpt_addr);
    pml4[0] = 0x7 | pdpt_addr;
    pdpt[0] = 0x87;
    sregs.cr3 = pml4_addr;
    sregs.cr4 = 0x6a0;
    sregs.cr0 = 0b1000'0000'0000'0101'0000'0000'0011'0011;
    sregs.efer = (1u<<8) | (1u<<10);
    kvm_segment seg{
      .base = 0,
      .limit = 0xffff'ffff,
      .selector = 1<<3,
      .type = 11,
      .present = 1,
      .dpl = 0,
      .db = 0,
      .s = 1,
      .l = 1,
      .g = 1,
    };
    sregs.cs = seg;
    seg.type = 3;
    seg.selector = 2<<3;
    sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg;

    sregs.cr4 |= (1<<13);
  }
  uiu.machine.vcpu.set_sregs(sregs);

  // END KVM
  //return EXIT_SUCCESS;

  std::ifstream ifs{filename};
  if (!ifs) {
    fmt::println("Unable to open file {}", filename);
    return EXIT_FAILURE;
  }

  //std::ifstream wrapper{"st/wrapper.efi"};
  std::ifstream wrapper{"build/start.efi"};
  if (!wrapper) {
    fmt::println("Unable to open file st/wrapper.efi");
    return EXIT_FAILURE;
  }

  void* start = (char*)0x1'0000 + load(std::move(wrapper), (char*)memory + 0x1'0000);
  void* efi_main_kvm = (char*)0x10'0000 + load(std::move(ifs), (char*)memory + 0x10'0000);
  ((unsigned char*)memory)[0] = 0xf4;

  /*EFI_LOADED_IMAGE_PROTOCOL lip{
    .Revision = EFI_LOADED_IMAGE_PROTOCOL_REVISION,
    .ParentHandle = (void*)0,
    .SystemTable = &st,
    .DeviceHandle = nullptr,  // TODO what's this?
    .FilePath = nullptr,  // TODO what's this?
    .LoadOptionsSize = 0,
    .LoadOptions = nullptr,
    .ImageBase = nullptr,  // TODO
    .ImageSize = 0,  // TODO
    .ImageCodeType = EfiReservedMemoryType,  // TODO
    .ImageDataType = EfiReservedMemoryType,  // TODO
    .Unload = nullptr,  // TODO can this be null?
  };*/

  //uiu.handle_db.emplace_back(std::unordered_map<EFI_GUID, void*>{{EFI_GUID(EFI_LOADED_IMAGE_PROTOCOL_GUID), &lip}});

  kvm_regs regs{
    .rax = 2,
    .rbx = 2,
    .rcx = std::uint64_t(efi_main_kvm),
    .rsp = 0x400'0000,
    .rip = std::uint64_t(start),
    .rflags = Rflags{},
  };
  uiu.machine.vcpu.set_regs(regs);

  fmt::println("ENTERING VM");
  uiu.run();
}
