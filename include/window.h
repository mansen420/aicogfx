#pragma once
namespace aicogfx
{
    enum WINDOW_STATUS
    {
        STATUS_FAILURE, STATUS_SUCCESS
    };
    int create_window(int width, int height, char* title);
    bool window_should_close();
    void process_shit();
    void destroy_window();
}
