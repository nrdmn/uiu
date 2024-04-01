#include "API.h"

template <UIUAPITag T>
auto uiuapifn() {
  using Fn = UIUAPIFn<T>;
  return []<auto... Is>(std::index_sequence<Is...>){
    return [](std::tuple_element_t<Is, typename Fn::Args>... args) EFIAPI {
      uint64_t params[] = {
        uint64_t(args)...
      };
      register typename Fn::R result __asm__("rax");
      register uint64_t* params_ptr __asm__("rdx") = params;
      asm volatile (
          "out %[nr], $0xff;"
        : "=r" (result)
        : "r" (params_ptr),
          [nr] "r" (T)
        : "memory"
      );
      return auto(result);
    };
  }(std::make_index_sequence<std::tuple_size_v<typename Fn::Args>>{});
}

extern "C" [[noreturn]] EFIAPI void _start(EFI_STATUS (EFIAPI *efi_main)(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)) {
  wchar_t vendor[] = L"UIU";

  SIMPLE_TEXT_OUTPUT_MODE out_mode = {};

  EFI_SIMPLE_TEXT_OUT_PROTOCOL stop = {
    .OutputString = uiuapifn<UIUAPITag::OutputString>(),
    .SetAttribute = (EFI_TEXT_SET_ATTRIBUTE)0xf1f1f1f1f1f1,
    .Mode = &out_mode,
  };

  EFI_BOOT_SERVICES bs = {
   .Hdr = {
      .Signature = EFI_BOOT_SERVICES_SIGNATURE,
      .Revision = EFI_BOOT_SERVICES_REVISION,
      .HeaderSize = sizeof(EFI_BOOT_SERVICES),
      .CRC32 = 0xbebebebe,  // TODO calculate this
    },
    .AllocatePool = uiuapifn<UIUAPITag::AllocatePool>(),  // 0x40
    .FreePool = uiuapifn<UIUAPITag::FreePool>(),  // 0x48
    .HandleProtocol = uiuapifn<UIUAPITag::HandleProtocol>(),
    .LocateHandle = uiuapifn<UIUAPITag::LocateHandle>(),
  };

  EFI_RUNTIME_SERVICES rs = {
    .Hdr = {
      .Signature = EFI_RUNTIME_SERVICES_SIGNATURE,
      .Revision = EFI_RUNTIME_SERVICES_REVISION,
      .HeaderSize = sizeof(EFI_RUNTIME_SERVICES),
      .CRC32 = 0x23456789,  // TODO calculate this
    },
    .GetVariable = uiuapifn<UIUAPITag::GetVariable>(),  // 0x48
  };

  EFI_SYSTEM_TABLE st = {
  .Hdr = {
      .Signature = EFI_SYSTEM_TABLE_SIGNATURE,
      .Revision = EFI_SYSTEM_TABLE_REVISION,
      .HeaderSize = sizeof(EFI_SYSTEM_TABLE),
      .CRC32 = 0xbabababa,  // TODO calculate this
    },
    .FirmwareVendor = vendor,
    .FirmwareRevision = 0,
    .ConsoleInHandle = (void*)0xfafafafafafa,
    .ConIn = (SIMPLE_INPUT_INTERFACE*)0xfbfbfbfbfbfb,
    .ConsoleOutHandle = (void*)0xfcfcfcfcfcfc,
    .ConOut = &stop,
    .StandardErrorHandle = (void*)0xfefefefefefe,
    .StdErr = (SIMPLE_TEXT_OUTPUT_INTERFACE*)0xf0f0f0f0f0f0,
    .RuntimeServices = &rs,
    .BootServices = &bs,
    .NumberOfTableEntries = 0,
    .ConfigurationTable = 0,
  };
  uiuapifn<UIUAPITag::Exit>()(efi_main((void*)0x1, &st));
}
