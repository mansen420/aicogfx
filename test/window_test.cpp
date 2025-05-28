#include <cstdio>
#include <cstdlib>
#include "window.h"

int main()
{
    using namespace aicogfx;
    int status = create_window(600, 300, "TITLE");
    
    if(status == STATUS_FAILURE)
        return EXIT_FAILURE; //early return

    while(!window_should_close())
        process_shit();

    destroy_window();
    return EXIT_SUCCESS;
}
