// SPDX-License-Identifier: GPL-2.0-or-later

#include <windows.h>

extern int __cdecl main(int, char*[]);

int wmain(int argc, wchar_t* argv[]) {
  char** args;
  int nbytes = static_cast<int>(sizeof(char*) * (argc + 1));
  HANDLE heap = GetProcessHeap();

  for (int i = 0; i < argc; ++i)
    nbytes += WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr);

  args = reinterpret_cast<char**>(HeapAlloc(heap, 0, nbytes));
  args[0] = reinterpret_cast<char*>(args + argc + 1);

  for (int i = 0; i < argc; ++i)
    args[i + 1] = args[i] + WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, args[i], nbytes, nullptr, nullptr);

  args[argc] = nullptr;

  argc = main(argc, args);
  HeapFree(heap, 0, args);
  return argc;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nCmdShow;

  int argc;
  wchar_t** argv;
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  argc = wmain(argc, argv);
  LocalFree(argv);
  return argc;
}
