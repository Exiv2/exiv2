// SPDX-License-Identifier: GPL-2.0-or-later

#include <windows.h>

extern int __cdecl main();

int wmain(int argc, wchar_t* argv[]) {
  char** args;
  int nbytes = (int)(sizeof(char*) * (argc + 1));
  HANDLE heap = GetProcessHeap();

  for (int i = 0; i < argc; ++i)
    nbytes += WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);

  args = HeapAlloc(heap, 0, nbytes);
  args[0] = (char*)(args + argc + 1);

  for (int i = 0; i < argc; ++i)
    args[i + 1] = args[i] + WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, args[i], nbytes, NULL, NULL);

  args[argc] = NULL;

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
