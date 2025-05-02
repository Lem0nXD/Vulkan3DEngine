#include "main_imgui.hpp"
#include "cbs_device.hpp"
#include "cbs_window.hpp"
#include "cbs_pipeline.hpp"

// libs
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

// std
#include <iostream>
#include <stdexcept>

namespace cbs {

    MainImgui::MainImgui(
        CbsWindow& window, CbsDevice& device, VkRenderPass renderPass, uint32_t imageCount)
        : cbsDevice{ device } {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000} };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if (vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up descriptor pool for imgui");
        }

        // Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan

        ImGui_ImplGlfw_InitForVulkan(window.getGLFWWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.device();
        init_info.QueueFamily = device.getGraphicsQueueFamily();
        init_info.Queue = device.graphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, renderPass);

        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        device.endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    MainImgui::~MainImgui() {
        vkDestroyDescriptorPool(cbsDevice.device(), descriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void MainImgui::newFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void MainImgui::render(VkCommandBuffer commandBuffer) {
        ImGui::Render();
        ImDrawData* drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }
    // *************** Menu File *********************

    static void ShowMenuFile(CbsWindow& window)
    {
        if (ImGui::MenuItem("Open", "Ctrl+O")) {}
        if (ImGui::BeginMenu("Open Recent"))
        {
            ImGui::MenuItem("cube.obj");
            ImGui::MenuItem("triangle.obj");
            ImGui::MenuItem("sphere.obj");
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {}

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Alt+F4")) { glfwSetWindowShouldClose(window.getGLFWWindow(), 1); }
    }

    // *************** Benchmark overlay *********************
    static void benchmarkOverlay(bool* p_open, CbsDevice& device)
    {
        static int location = 2;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = 10.0f;
            const float offset = 40.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD - offset) : (work_pos.y + PAD + offset);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        else if (location == -2)
        {
            // Center window
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
     
        if (ImGui::Begin("Benchmark Overlay", p_open, window_flags))
        {
            ImGui::Text("GPU: [%s]", device.properties.deviceName);
            ImGui::Text(
                "Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                if (p_open && ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    // *************** Delay graph *********************

    static void avgDelayWindow(bool* p_open) {
        static float values[90] = {};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (refresh_time == 0.0)
            refresh_time = ImGui::GetTime();
        while (refresh_time < ImGui::GetTime())
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            phase = values_offset;
            refresh_time = ImGui::GetTime();;

            float average = 0.0f;
            for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                average += values[n];
            average /= (float)IM_ARRAYSIZE(values);
            char overlay[32];
            sprintf_s(overlay, "avg %f", average);
            
            static int location = 3;
            ImGuiIO& io = ImGui::GetIO();
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
            if (location >= 0)
            {
                const float PAD = 10.0f;
                const float offset = 40.0f;
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
                ImVec2 work_size = viewport->WorkSize;
                ImVec2 window_pos, window_pos_pivot;
                window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
                window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD - offset) : (work_pos.y + PAD + offset);
                window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
                window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                window_flags |= ImGuiWindowFlags_NoMove;
            }
            else if (location == -2)
            {
                // Center window
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                window_flags |= ImGuiWindowFlags_NoMove;
            }
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

            if (ImGui::Begin("Delay Graph", p_open, window_flags))
            {
                ImGui::PlotLines("Avg\nDelay\nms/\nframe", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, 1.0f, ImVec2(0, 80.0f));
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                    if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                    if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                    if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                    if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                    if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                    if (p_open && ImGui::MenuItem("Close")) *p_open = false;
                    ImGui::EndPopup();
                }
            }
            ImGui::End();
        }
    }

    // *************** Progress bar *********************

    void progressBar() {
        static float progress = 0, progress_dir = 1.0f;
        progress += progress_dir * 2.5f * ImGui::GetIO().DeltaTime;
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    }

    // *************** Centered text *********************

    void TextCentered(std::string text) {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
    }

    // *************** About window *********************

    void ShowAboutWindow(bool* p_open) {
        ImGui::Begin("About Cubus", p_open, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(300, 400));
        TextCentered("Cubus\nDescription: Program for modeling\n3D objects\nVersion 1.0.0\nDeveloper: Vlad Kolodiy\nGroup: 4PI-20b");
        ImGui::End();
    }

    // *************** Tool bar *********************

    void ShowToolbarWindow(bool* p_open, bool* wireframe, bool* normal, bool* benchmark, bool* graph, bool* help) 
    {
        static int location = 0;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = .95f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos;
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x) : (work_pos.x);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        else if (location == -2)
        {
            // Center window
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        if (ImGui::Begin("Tool Bar", p_open, window_flags))
        {
            int clicked1 = 0;
            int clicked2 = 0;
            int clicked3 = 0;
            int clicked4 = 0;
            int clicked5 = 0;

            ImVec2 button_sz(100, 40);
            ImGuiStyle& style = ImGui::GetStyle();
            if (ImGui::Button("Wireframe", button_sz))
                clicked1 = 1;
            if (clicked1 & 1)
            {
                if (*wireframe)
                {
                    *wireframe = false;
                    std::cout << "Wireframe";
                }
                else { *wireframe = true; }
            }
            ImGui::SameLine();
            if (ImGui::Button("Benchmark", button_sz))
                clicked2 = 1;
            if (clicked2 & 1)
            {
                if (*benchmark)
                {
                    *benchmark = false;
                }
                else { *benchmark = true; }
            }
            ImGui::SameLine();
            if (ImGui::Button("Graph", button_sz))
                clicked3 = 1;
            if (clicked3 & 1)
            {
                if (*graph)
                {
                    *graph = false;
                }
                else { *graph = true; }
            }
            ImGui::SameLine();
            if (ImGui::Button("Manual", button_sz))
                clicked4 = 1;
            if (clicked4 & 1)
            {
                if (*help)
                {
                    *help = false;
                }
                else { *help = true; }
            }
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                if (p_open && ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndPopup();
            }
            ImGui::End();
        }
    }

    // *************** Status bar *********************

    void ShowStatusbarWindow(bool* p_open) {
        const char* statustext;
        static int location = 2;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = .0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos;
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x) : (work_pos.x);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        else if (location == -2)
        {
            // Center window
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        if (ImGui::Begin("Status bar", p_open, window_flags))
        {
            ImGui::SetWindowSize(ImVec2(1920, 30));
            if (ImGui::IsMousePosValid())
                ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("Mouse pos: <INVALID>");
            ImGui::SameLine();
            statustext = "Loaded file : smooth_vase.obj";
            ImGui::Text(statustext);
            ImGui::SameLine();
            progressBar();
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                if (p_open && ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndPopup();
            }
            ImGui::End();
        }
    }

    // *************** CONTROLS GUIDE *********************

    void ShowControlsGuide()
    {
        ImGui::BulletText("Double-click on title bar to collapse window.");
        ImGui::BulletText(
            "Click and drag on lower corner to resize window\n"
            "(double-click to auto fit window to its contents).");
        ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
        ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
        ImGui::BulletText("CTRL+Tab to select a window.");
    }

    // *************** CAMERA MOVEMENT/VIEW GUIDE *********************

    void ShowCameraMovementViewGuide()
    {
        ImGui::Text("Camera movement controls:");
        ImGui::BulletText("Move Forward - W.");
        ImGui::BulletText("Move Left - A.");
        ImGui::BulletText("Move Backward - S.");
        ImGui::BulletText("Move Right - D.");
        ImGui::BulletText("Move Up - SPACE.");
        ImGui::BulletText("Move Down - LEFT ALT.");
        ImGui::Separator();
        ImGui::Text("Camera view controls:");
        ImGui::BulletText("Look Left - LEFT ARROW.");
        ImGui::BulletText("Look Right - RIGHT ARROW.");
        ImGui::BulletText("Look Up - UP ARROW.");
        ImGui::BulletText("Look Down - DOWN ARROW.");
    }

    // *************** BENCHMARK GUIDE *********************

    void ShowBenchmarkGuide() 
    {
        ImGui::BulletText("Right-click on benchmark to change position");
        ImGui::Text("Benchmark output:");
        ImGui::BulletText("Your model of GPU.");
        ImGui::BulletText("Your average ms/frame and frame per second");
    }

    // *************** DELAY GRAPH GUIDE *********************

    void ShowDelayGraphGuide()
    {
        ImGui::BulletText("Right-click on delay graph to change position");
        ImGui::Text("Graph output:");
        ImGui::BulletText("Graph of average delay step each ms/frame");
    }

    // *************** Main Function *********************

    void MainImgui::runGUI(CbsWindow& window) {
        // --- Main Menu Bar ---
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ShowMenuFile(window);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Normal", NULL, &switch_to_normal)) {}
                if (ImGui::MenuItem("Wireframe", NULL, &switch_to_wireframe)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Benchmark overlay", NULL, &show_benchmark_window)) {}
                if (ImGui::MenuItem("Avg delay graph", NULL, &show_avg_delay_window)) {}
                if (ImGui::MenuItem("Tool bar", NULL, &show_toolbar_window)) {}
                if (ImGui::BeginMenu("Color style"))
                {
                    if (ImGui::MenuItem("Dark")) { ImGui::StyleColorsDark(); };
                    if (ImGui::MenuItem("Light")) { ImGui::StyleColorsLight(); };
                    if (ImGui::MenuItem("Classic")) { ImGui::StyleColorsClassic(); };
                    ImGui::EndMenu();
                }
                //if (ImGui::MenuItem("Demo window", NULL, &show_demo_window)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("View help", NULL, &show_help_window)) {}
                if (ImGui::MenuItem("About Cubus", NULL, &show_about_window)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        // --- Main Menu Bar (END)---
        if (show_benchmark_window)
        {
            benchmarkOverlay(&show_benchmark_window, cbsDevice);
        }
        //delay graph
        if (show_avg_delay_window)
        {
            avgDelayWindow(&show_avg_delay_window);
        }
        //help
        if (show_help_window)
        {
            ImGui::Begin("Manual", &show_help_window);
            if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_None))
            {
                ShowControlsGuide();
            }
            if (ImGui::CollapsingHeader("Camera movement/view", ImGuiTreeNodeFlags_None))
            {
                ShowCameraMovementViewGuide();
            }
            if (ImGui::CollapsingHeader("Benchmark", ImGuiTreeNodeFlags_None))
            {
                ShowBenchmarkGuide();
            }
            if (ImGui::CollapsingHeader("Delay graph", ImGuiTreeNodeFlags_None))
            {
                ShowDelayGraphGuide();
            }
            ImGui::End();
        }
        //about
        if (show_about_window)
        {
            ShowAboutWindow(&show_about_window);
        }

        if (show_toolbar_window)
        {
            ShowToolbarWindow(
                &show_toolbar_window,
                &switch_to_wireframe,
                &switch_to_normal,
                &show_benchmark_window, 
                &show_avg_delay_window, 
                &show_help_window
            );
        }
        if (show_status_bar)
        {
            ShowStatusbarWindow(&show_status_bar);
        }
        if (switch_to_normal) {}
        if (switch_to_wireframe) {}
    }
}  // namespace cbs