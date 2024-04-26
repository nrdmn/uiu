#include "API.h"

#include <cstring>  // memset

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

[[noreturn]] EFIAPI  __attribute__((naked)) void trap() {
  asm volatile (
      "out %[nr], $0xff;"
    :
    : [nr] "r" (UIUAPITag::Trap)
  );
}

extern "C" [[noreturn]] EFIAPI void _start(EFI_STATUS (EFIAPI *efi_main)(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)) {
  wchar_t vendor[] = L"UIU";

  SIMPLE_TEXT_OUTPUT_MODE out_mode = {};
  for (int i = 0; i < (sizeof(out_mode)/8); i++) {
    auto& p = ((uint64_t*)&out_mode)[i];
    if (p == 0) {
      p = i + 0xf6f6'f6f6'f6f6'f6f6;
    }
  }

  EFI_SIMPLE_TEXT_OUT_PROTOCOL stop = {
    .Reset = EFI_TEXT_RESET(&trap),
    .OutputString = uiuapifn<UIUAPITag::OutputString>(),
    .TestString = EFI_TEXT_TEST_STRING(&trap),
    .QueryMode = EFI_TEXT_QUERY_MODE(&trap),
    .SetMode = EFI_TEXT_SET_MODE(&trap),
    .SetAttribute = EFI_TEXT_SET_ATTRIBUTE(&trap),
    .ClearScreen = EFI_TEXT_CLEAR_SCREEN(&trap),
    .SetCursorPosition = EFI_TEXT_SET_CURSOR_POSITION(&trap),
    .EnableCursor = EFI_TEXT_ENABLE_CURSOR(&trap),
    .Mode = &out_mode,
  };

  EFI_BOOT_SERVICES bs = {
    .Hdr = {
      .Signature = EFI_BOOT_SERVICES_SIGNATURE,
      .Revision = EFI_BOOT_SERVICES_REVISION,
      .HeaderSize = sizeof(EFI_BOOT_SERVICES),
      .CRC32 = 0xbebebebe,  // TODO calculate this
    },

    .RaiseTPL = EFI_RAISE_TPL(&trap),
    .RestoreTPL = EFI_RESTORE_TPL(&trap),

    .AllocatePages = uiuapifn<UIUAPITag::AllocatePages>(),
    .FreePages = EFI_FREE_PAGES(&trap),
    .GetMemoryMap = uiuapifn<UIUAPITag::GetMemoryMap>(),
    .AllocatePool = uiuapifn<UIUAPITag::AllocatePool>(),  // 0x40
    .FreePool = uiuapifn<UIUAPITag::FreePool>(),  // 0x48

    .CreateEvent = EFI_CREATE_EVENT(&trap),
    .SetTimer = EFI_SET_TIMER(&trap),
    .WaitForEvent = EFI_WAIT_FOR_EVENT(&trap),
    .SignalEvent = EFI_SIGNAL_EVENT(&trap),
    .CloseEvent = EFI_CLOSE_EVENT(&trap),
    .CheckEvent = EFI_CHECK_EVENT(&trap),

    .InstallProtocolInterface = uiuapifn<UIUAPITag::InstallProtocolInterface>(),
    .ReinstallProtocolInterface = EFI_REINSTALL_PROTOCOL_INTERFACE(&trap),
    .UninstallProtocolInterface = EFI_UNINSTALL_PROTOCOL_INTERFACE(&trap),
    .HandleProtocol = uiuapifn<UIUAPITag::HandleProtocol>(),
    .RegisterProtocolNotify = EFI_REGISTER_PROTOCOL_NOTIFY(&trap),
    .LocateHandle = uiuapifn<UIUAPITag::LocateHandle>(),
    .LocateDevicePath = EFI_LOCATE_DEVICE_PATH(&trap),
    .InstallConfigurationTable = EFI_INSTALL_CONFIGURATION_TABLE(&trap),

    .LoadImage = EFI_IMAGE_LOAD(&trap),
    .StartImage = EFI_IMAGE_START(&trap),
    .Exit = EFI_EXIT(&trap),
    .UnloadImage = EFI_IMAGE_UNLOAD(&trap),
    .ExitBootServices = uiuapifn<UIUAPITag::ExitBootServices>(),

    .GetNextMonotonicCount = EFI_GET_NEXT_MONOTONIC_COUNT(&trap),
    .Stall = EFI_STALL(&trap),
    .SetWatchdogTimer = EFI_SET_WATCHDOG_TIMER(&trap),

    .ConnectController = EFI_CONNECT_CONTROLLER(&trap),
    .DisconnectController = EFI_DISCONNECT_CONTROLLER(&trap),

    .OpenProtocol = EFI_OPEN_PROTOCOL(&trap),
    .CloseProtocol = EFI_CLOSE_PROTOCOL(&trap),
    .OpenProtocolInformation = EFI_OPEN_PROTOCOL_INFORMATION(&trap),

    .ProtocolsPerHandle = EFI_PROTOCOLS_PER_HANDLE(&trap),
    .LocateHandleBuffer = EFI_LOCATE_HANDLE_BUFFER(&trap),
    .LocateProtocol = uiuapifn<UIUAPITag::LocateProtocol>(),
    .InstallMultipleProtocolInterfaces = EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES(&trap),
    .UninstallMultipleProtocolInterfaces = EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES(&trap),

    .CalculateCrc32 = EFI_CALCULATE_CRC32(&trap),

    .CopyMem = EFI_COPY_MEM(&trap),
    .SetMem = EFI_SET_MEM(&trap),
    .CreateEventEx = EFI_CREATE_EVENT_EX(&trap),
  };

  EFI_RUNTIME_SERVICES rs = {
    .Hdr = {
      .Signature = EFI_RUNTIME_SERVICES_SIGNATURE,
      .Revision = EFI_RUNTIME_SERVICES_REVISION,
      .HeaderSize = sizeof(EFI_RUNTIME_SERVICES),
      .CRC32 = 0x23456789,  // TODO calculate this
    },
    .GetTime = EFI_GET_TIME(&trap),
    .SetTime = EFI_SET_TIME(&trap),
    .GetWakeupTime = EFI_GET_WAKEUP_TIME(&trap),
    .SetWakeupTime = EFI_SET_WAKEUP_TIME(&trap),

    .SetVirtualAddressMap = EFI_SET_VIRTUAL_ADDRESS_MAP(&trap),
    .ConvertPointer = EFI_CONVERT_POINTER(&trap),

    .GetVariable = uiuapifn<UIUAPITag::GetVariable>(),  // 0x48
    .GetNextVariableName = EFI_GET_NEXT_VARIABLE_NAME(&trap),
    .SetVariable = uiuapifn<UIUAPITag::SetVariable>(),

    .GetNextHighMonotonicCount = EFI_GET_NEXT_HIGH_MONO_COUNT(&trap),
    .ResetSystem = EFI_RESET_SYSTEM(&trap),

    .UpdateCapsule = EFI_UPDATE_CAPSULE(&trap),
    .QueryCapsuleCapabilities = EFI_QUERY_CAPSULE_CAPABILITIES(&trap),

    .QueryVariableInfo = EFI_QUERY_VARIABLE_INFO(&trap),
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
  for (int i = 0; i < (sizeof(st)/8); i++) {
    auto& p = ((uint64_t*)&st)[i];
    if (p == 0) {
      p = i + 0xf7f7'f7f7'f7f7'f7f7;
    }
  }

  EFI_RNG_PROTOCOL rng_proto = {
    .GetInfo = EFI_RNG_GET_INFO(&trap),
    .GetRNG = uiuapifn<UIUAPITag::GetRNG>(),
  };

  EFI_LOADED_IMAGE_PROTOCOL lip_proto = {
    .ImageBase = (void*)0x10'0000,
    .ImageSize = 5283136,
  };
  for (int i = 0; i < (sizeof(lip_proto)/8); i++) {
    auto& p = ((uint64_t*)&lip_proto)[i];
    if (p == 0) {
      p = i + 0xf9f9'f9f9'f9f9'f9f9;
    }
  }
  char16_t opt[] = {0};
  lip_proto.LoadOptions = &opt;

  EFI_HANDLE handle = nullptr;

  EFI_GUID lip_guid = LOADED_IMAGE_PROTOCOL;
  uiuapifn<UIUAPITag::InstallProtocolInterface>()(&handle, &lip_guid, EFI_NATIVE_INTERFACE, (void*)&lip_proto);
  EFI_GUID rng_guid = EFI_RNG_PROTOCOL_GUID;
  uiuapifn<UIUAPITag::InstallProtocolInterface>()(&handle, &rng_guid, EFI_NATIVE_INTERFACE, (void*)&rng_proto);

  wchar_t secureboot[] = L"SecureBoot";
  char zero = 0;
  wchar_t setupmode[] = L"SetupMode";
  EFI_GUID securebootguid{0x8be4df61, 0x93ca, 0x11d2, {0xaa, 0x0d, 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c}};
  uiuapifn<UIUAPITag::SetVariable>()(secureboot, &securebootguid, (UINT32)0, (UINTN)1, (void*)&zero);
  uiuapifn<UIUAPITag::SetVariable>()(setupmode, &securebootguid, (UINT32)0, (UINTN)1, (void*)&zero);

  uiuapifn<UIUAPITag::Exit>()(efi_main(handle, &st));
}
