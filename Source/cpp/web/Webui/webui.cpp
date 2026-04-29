#define WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL 1
#include "webview.h"
#include <windows.h>
#include <string>

int main() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    std::string exeDir = exePath.substr(0, exePath.find_last_of("\\/"));
    std::string url = "http://localhost:3000";
    webview::webview w(true, nullptr);
    w.set_title("J.A.R.V.I.S");
    w.set_size(1200, 800, WEBVIEW_HINT_NONE);
    w.navigate(url);
    w.run();
    return 0;
}