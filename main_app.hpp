#pragma once

#include "cbs_descriptors.hpp"
#include "cbs_device.hpp"
#include "cbs_game_object.hpp"
#include "cbs_renderer.hpp"
#include "cbs_window.hpp"

#include "cbs_pipeline.hpp"
#include "render_system.hpp"


// std
#include <memory>
#include <vector>

namespace cbs {
	class MainApp {
	public:
		static constexpr int WIDTH = 1280;
		static constexpr int HEIGHT = 720;

		MainApp();
		~MainApp();

		MainApp(const MainApp&) = delete;
		MainApp& operator=(const MainApp&) = delete;

		void run();

	private:
		void loadGameObjects();
		
		CbsWindow cbsWindow{ WIDTH, HEIGHT, "Cubus" };
		CbsDevice cbsDevice{ cbsWindow };
		CbsRenderer cbsRenderer{ cbsWindow, cbsDevice };
		// note: order of declarations matters
		std::unique_ptr<CbsDescriptorPool> globalPool{};
		std::vector<CbsGameObject> gameObjects;
	};
}  // namespace cbs