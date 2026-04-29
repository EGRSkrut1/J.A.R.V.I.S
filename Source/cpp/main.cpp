#include "core/application/jarvis_app.h"
#include <cstdlib>

int main() {
    system("chcp 65001 > nul");
    
    JarvisApp app;
    if (!app.initialize()) return 1;
    
    app.run();
    app.shutdown();
    
    return 0;
}