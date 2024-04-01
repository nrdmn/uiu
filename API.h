#pragma once

#include <tuple>

#define GNU_EFI_USE_MS_ABI
extern "C" {
#include <efi.h>
}

enum class UIUAPITag {
  Exit = 0,
  HandleProtocol,
  GetVariable,
  AllocatePool,
  FreePool,
  LocateHandle,
  OutputString,
};

template <UIUAPITag N>
struct UIUAPIFn;

template <>
struct UIUAPIFn<UIUAPITag::Exit> {
  using R = EFI_STATUS;
  using Args = std::tuple<EFI_STATUS>;
};

template <>
struct UIUAPIFn<UIUAPITag::HandleProtocol> {
  using R = EFI_STATUS;
  using Args = std::tuple<EFI_HANDLE, EFI_GUID*, VOID**>;
};

template <>
struct UIUAPIFn<UIUAPITag::GetVariable> {
  using R = EFI_STATUS;
  using Args = std::tuple<CHAR16*, EFI_GUID*, UINT32*, UINTN*, VOID*>;
};

template <>
struct UIUAPIFn<UIUAPITag::AllocatePool> {
  using R = EFI_STATUS;
  using Args = std::tuple<EFI_MEMORY_TYPE, UINTN, VOID**>;
};

template <>
struct UIUAPIFn<UIUAPITag::FreePool> {
  using R = EFI_STATUS;
  using Args = std::tuple<VOID*>;
};

template <>
struct UIUAPIFn<UIUAPITag::LocateHandle> {
  using R = EFI_STATUS;
  using Args = std::tuple<EFI_LOCATE_SEARCH_TYPE, EFI_GUID*, VOID*, UINTN*, EFI_HANDLE*>;
};

template <>
struct UIUAPIFn<UIUAPITag::OutputString> {
  using R = EFI_STATUS;
  using Args = std::tuple<EFI_SIMPLE_TEXT_OUT_PROTOCOL*, CHAR16*>;
};
