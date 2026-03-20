#include "FramebufferUI.hpp"
#include "TouchHandler.hpp"
#include "InteractionManager.hpp"
#include "SystemInterfaces.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using namespace UI;

int main() {
    try {
        // 1. 初始化底层驱动 (默认使用 /dev/fb0)
        FramebufferUI ui("/dev/fb0");
        // 确保获取的宽度高度正确
        std::cout << "Current Resolution: " << ui.getWidth() << "x" << ui.getHeight() << std::endl;
        std::cout << "Detected Resolution: " << ui.getWidth() << "x" << ui.getHeight() << std::endl;
        if (ui.getHeight() != 600) {
    std::cout << "Warning: System reported height " << ui.getHeight() 
              << " does not match physical 600. Adjusting layouts..." << std::endl;
}

        // 2. 初始化触摸处理器 (确保路径与树莓派识别的节点一致)
        TouchHandler touch("/dev/input/event0"); 

        // 3. 设置命令发射器 (模拟 FastDDS 发送指令)
        InteractionManager::setCommandEmitter([](const System::ControlCommand& cmd) {
            std::cout << "[Command Sent] Type: " << static_cast<int>(cmd.type) 
                      << " Value: " << cmd.intValue << std::endl;
        });

        // 4. 启动触摸监听，并将点击事件传递给 InteractionManager
        touch.startListening([](int x, int y) {
            InteractionManager::handleTouch(x, y);
        });

        // 5. 模拟主循环：模拟数据输入并刷新 UI
        bool running = true;
        float fakeIntensity = 0.0f;
        float step = 0.05f;

        std::cout << "Starting main loop. Press Ctrl+C to stop." << std::endl;

        while (running) {
            ui.fillScreen(0xFFFF0000);
            // 模拟音频跳动数据
            fakeIntensity += step;
            if (fakeIntensity > 1.0f || fakeIntensity < 0.0f) step = -step;

            // 模拟系统状态
            System::AudioVisualData avData = { std::abs(fakeIntensity), {} };
            System::PlaybackStatus status = { "Testing Track", "Unknown Artist", 45, 120, true, 50 };
            System::EnvironmentStatus env = { 24.5f, 60.0f, 1013.25f, 500, "2026-03-20 12:00" };

            // 根据当前页面调用对应的刷新函数
            if (InteractionManager::currentPage == UIPage::STANDBY) {
                ui.refreshStandby(env);
            } else {
                ui.refreshMusicAnimation(avData, status);
            }

            // 控制刷新率约为 30 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
