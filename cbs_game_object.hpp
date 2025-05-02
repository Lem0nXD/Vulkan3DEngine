#pragma once

#include "cbs_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace cbs {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };

    class CbsGameObject {
    public:
        using id_t = unsigned int;

        static CbsGameObject createGameObject() {
            static id_t currentId = 0;
            return CbsGameObject{ currentId++ };
        }

        CbsGameObject(const CbsGameObject&) = delete;
        CbsGameObject& operator=(const CbsGameObject&) = delete;
        CbsGameObject(CbsGameObject&&) = default;
        CbsGameObject& operator=(CbsGameObject&&) = default;

        id_t getId() { return id; }

        std::shared_ptr<CbsModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};

    private:
        CbsGameObject(id_t objId) : id{ objId } {}

        id_t id;
    };
}  // namespace cbs