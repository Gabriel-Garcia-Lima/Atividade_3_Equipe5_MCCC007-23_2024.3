#include "cube.hpp"
#include "ground.hpp"
#include <cmath>
#include <iostream>

#include <GL/gl.h> // Standard OpenGL header
// Or if using GLEW
#include <GL/glew.h>

#include <glm/gtx/fast_trigonometry.hpp>
#include <unordered_map>

// Especialização explícita de std::hash para Vertex
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};

void Cube::createBuffers() {
  // Deleta buffers anteriores
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteBuffers(1, &m_wireframeEBO);

  // VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices.at(0)) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // EBO for filled rendering
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices.at(0)) * m_indices.size(),
                     m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // EBO for wireframe rendering
  abcg::glGenBuffers(1, &m_wireframeEBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_wireframeEBO);

  // Create wireframe indices
  std::vector<GLuint> wireframeIndices;
  for (size_t i = 0; i < m_indices.size(); i += 3) {
    wireframeIndices.push_back(m_indices[i]);
    wireframeIndices.push_back(m_indices[i + 1]);
    wireframeIndices.push_back(m_indices[i + 1]);
    wireframeIndices.push_back(m_indices[i + 2]);
    wireframeIndices.push_back(m_indices[i + 2]);
    wireframeIndices.push_back(m_indices[i]);
  }

  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(wireframeIndices.at(0)) * wireframeIndices.size(),
                     wireframeIndices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cube::loadObj(std::string_view path) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A map for Vertex and index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (auto const &shape : shapes) {
    // Loop over faces(polygon)
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Vertex position
      auto const startIndex{3 * index.vertex_index};
      glm::vec3 position{attrib.vertices.at(startIndex + 0),
                         attrib.vertices.at(startIndex + 1),
                         attrib.vertices.at(startIndex + 2)};

      // Texture coordinates
      glm::vec2 texCoord{};
      if (index.texcoord_index >= 0) {
        auto const startTexCoordIndex{2 * index.texcoord_index};
        texCoord = glm::vec2{attrib.texcoords.at(startTexCoordIndex + 0),
                             attrib.texcoords.at(startTexCoordIndex + 1)};
      }

      Vertex const vertex{.position = position, .texCoord = texCoord};

      // If hash doesn't contain this vertex
      if (!hash.contains(vertex)) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  createBuffers();
}

// In cube.cpp, update the loadDiffuseTexture function:
void Cube::loadDiffuseTexture(std::string_view path) {
  abcg::glDeleteTextures(1, &m_diffuseTexture);

  abcg::OpenGLTextureCreateInfo textureCreateInfo;
  textureCreateInfo.path = path;
  textureCreateInfo.generateMipmaps = true;

  m_diffuseTexture = abcg::loadOpenGLTexture(textureCreateInfo);
}

void Cube::paint() {
  // Configura as variáveis uniformes para o cubo
  m_positionMatrix = glm::translate(glm::mat4{1.0f}, m_position);
  m_modelMatrix = m_positionMatrix * m_animationMatrix;

  // Ajusta a escala do prisma com base no estado
  glm::vec3 scaleVec{m_scale, m_scale * 2.0f, m_scale};

  if (m_state == State::LAYING_Z) {
    scaleVec.z *= 2.0f;
    scaleVec.y = m_scale;
  } else if (m_state == State::LAYING_X) {
    scaleVec.x *= 2.0f;
    scaleVec.y = m_scale;
  }

  m_modelMatrix = glm::scale(m_modelMatrix, scaleVec);

  abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  abcg::glBindVertexArray(m_VAO);

  // Activate texture unit 0
  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
  abcg::glUniform1i(m_diffuseTexLoc, 0);

  // Draw filled cube with texture
  abcg::glUniform4f(m_colorLoc, 1.0f, 1.0f, 1.0f,
                    1.0f); // Use white to not tint texture
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                       nullptr);

  // Draw wireframe
  abcg::glUniform4f(m_colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_wireframeEBO);
  abcg::glDrawElements(GL_LINES, m_indices.size() * 2, GL_UNSIGNED_INT,
                       nullptr);

  abcg::glBindVertexArray(0);
}

void Cube::create(GLuint program, GLint modelMatrixLoc, GLint colorLoc,
                  glm::mat4 viewMatrix, float scale, int N) {
  // Libera o VAO anterior
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Cria o VAO
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Vincula EBO e VBO
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Get location of texture uniform
  m_diffuseTexLoc = abcg::glGetUniformLocation(program, "diffuseTex");

  // Load texture
  auto const assetsPath{abcg::Application::getAssetsPath()};
  loadDiffuseTexture(assetsPath +
                     "cube_texture.png"); // You'll need to create this texture

  // Bind vertex attributes
  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  auto const texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};

  // Enable attributes
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }

  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    abcg::glVertexAttribPointer(
        texCoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, texCoord)));
  }

  // Fim da vinculação
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);

  m_modelMatrixLoc = modelMatrixLoc;
  m_viewMatrix = viewMatrix;
  m_colorLoc = colorLoc;
  m_scale = scale;
  m_maxPos = m_scale * N;
}

void Cube::update(float deltaTime) { move(deltaTime); }

void Cube::setGround(Ground *ground) { m_ground = ground; }

void Cube::destroy() const {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteBuffers(1, &m_wireframeEBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
  abcg::glDeleteTextures(1, &m_diffuseTexture);
}

// Novo método para gerar posição aleatória
glm::vec3 Cube::generateRandomPosition() {
  int randomX = m_xDistribution(m_randomEngine);
  int randomZ = m_zDistribution(m_randomEngine);

  // Converte coordenadas do grid para coordenadas do mundo
  float worldX = randomX * m_scale * 2.0f - m_maxPos;
  float worldZ = randomZ * m_scale * 2.0f - m_maxPos;

  return glm::vec3(worldX, 0.0f, worldZ);
}

void Cube::move(float deltaTime) {
  if (!m_isMoving && !m_isFalling)
    return;

  if (m_isFalling) {
    // Animação de queda permanece a mesma
    m_fallTime += deltaTime;
    m_position.y -= m_fallSpeed * deltaTime;
    if (m_fallTime > m_fallDuration) {
      resetGame();
    }
    return;
  }

  float max_angle = 90.0f;

  if (m_angle < max_angle) {
    // Incrementa o ângulo com base na direção de rotação
    m_angle += deltaTime * m_angleVelocity;
    if (m_angle > max_angle)
      m_angle = max_angle;

    glm::vec3 rotationAxis{0.0f, 0.0f, 0.0f};
    glm::vec3 pivotPoint{0.0f, 0.0f, 0.0f};

    float offset = m_scale / 2.0f;

    switch (m_state) {
    case State::STANDING:
      switch (m_orientation) {
      case Orientation::UP:
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        pivotPoint =
            m_position + glm::vec3(0.0f, 0.0f, -offset); // Ajuste Y para 0.0f
        break;
      case Orientation::DOWN:
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        pivotPoint =
            m_position + glm::vec3(0.0f, 0.0f, offset); // Ajuste Y para 0.0f
        break;
      case Orientation::LEFT:
        rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        pivotPoint =
            m_position + glm::vec3(-offset, 0.0f, 0.0f); // Ajuste Y para 0.0f
        break;
      case Orientation::RIGHT:
        rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        pivotPoint =
            m_position + glm::vec3(offset, 0.0f, 0.0f); // Ajuste Y para 0.0f
        break;
      }
      break;

    case State::LAYING_Z:
      if (m_orientation == Orientation::UP ||
          m_orientation == Orientation::DOWN) {
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        float pivotOffsetZ =
            (m_orientation == Orientation::UP ? -m_scale : m_scale);
        pivotPoint = m_position + glm::vec3(0.0f, 0.0f, pivotOffsetZ);
      } else {
        // Movendo para a esquerda/direita enquanto está LAYING_Z
        rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        pivotPoint =
            m_position +
            glm::vec3((m_orientation == Orientation::LEFT ? -offset : offset),
                      0.0f, 0.0f);
      }
      break;

    case State::LAYING_X:
      if (m_orientation == Orientation::LEFT ||
          m_orientation == Orientation::RIGHT) {
        rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        float pivotOffsetX =
            (m_orientation == Orientation::LEFT ? -m_scale : m_scale);
        pivotPoint =
            m_position + glm::vec3(pivotOffsetX, 0.0f, 0.0f); // Y = 0.0f
      } else {
        // Movendo para cima/baixo enquanto está LAYING_X
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        pivotPoint = m_position + glm::vec3(0.0f, 0.0f,
                                            (m_orientation == Orientation::UP
                                                 ? -offset
                                                 : offset)); // Y = 0.0f
      }
      break;
    }

    // Aplicar as transformações de rotação com a direção correta
    m_animationMatrix = glm::translate(glm::mat4(1.0f), pivotPoint);
    m_animationMatrix =
        glm::rotate(m_animationMatrix,
                    glm::radians(m_rotationDirection * m_angle), rotationAxis);
    m_animationMatrix = glm::translate(m_animationMatrix, -pivotPoint);

  } else {
    translate();
    resetAnimation();
  }
}

void Cube::resetAnimation() {
  m_animationMatrix = glm::mat4{1.0f};
  m_angle = 0.0f;
  m_isMoving = false;
  m_border = false;
}

void Cube::translate() {
  float moveDistance = m_scale;

  switch (m_state) {
  case State::STANDING:
    if (m_orientation == Orientation::UP) {
      m_position.z -= moveDistance;
      m_position.z -= moveDistance / 2.0f; // Shift para estado LAYING
      m_state = State::LAYING_Z;
    } else if (m_orientation == Orientation::DOWN) {
      m_position.z += moveDistance;
      m_position.z += moveDistance / 2.0f;
      m_state = State::LAYING_Z;
    } else if (m_orientation == Orientation::LEFT) {
      m_position.x -= moveDistance;
      m_position.x -= moveDistance / 2.0f;
      m_state = State::LAYING_X;
    } else if (m_orientation == Orientation::RIGHT) {
      m_position.x += moveDistance;
      m_position.x += moveDistance / 2.0f;
      m_state = State::LAYING_X;
    }
    break;

  case State::LAYING_Z:
    if (m_orientation == Orientation::UP ||
        m_orientation == Orientation::DOWN) {
      // Move duas casas
      m_position.z += (m_orientation == Orientation::UP ? -2.0f * moveDistance
                                                        : 2.0f * moveDistance);
      // Ajusta a posição para o centro
      m_position.z +=
          (m_orientation == Orientation::UP ? moveDistance : -moveDistance) /
          2.0f;
      m_state = State::STANDING;
    } else {
      // Move uma casa na direção X
      m_position.x +=
          (m_orientation == Orientation::LEFT ? -moveDistance : moveDistance);
    }
    break;

  case State::LAYING_X:
    if (m_orientation == Orientation::LEFT ||
        m_orientation == Orientation::RIGHT) {
      // Move duas casas
      m_position.x +=
          (m_orientation == Orientation::LEFT ? -2.0f * moveDistance
                                              : 2.0f * moveDistance);
      // Ajusta a posição para o centro
      m_position.x +=
          (m_orientation == Orientation::LEFT ? moveDistance : -moveDistance) /
          2.0f;
      m_state = State::STANDING;
    } else {
      // Move uma casa na direção Z
      m_position.z +=
          (m_orientation == Orientation::UP ? -moveDistance : moveDistance);
    }
    break;
  }

  // Garantir que Y permaneça constante após translação
  m_position.y = 0.0f; // Mantém o prisma na superfície da plataforma

  // Após atualizar a posição, verifique se o Cube está sobre o buraco **e está
  // em pé**
  if (m_ground != nullptr) {
    // Converte as coordenadas do mundo para coordenadas do grid
    int gridX = static_cast<int>(round(m_position.x / m_scale));
    int gridZ = static_cast<int>(round(m_position.z / m_scale));

    int holeX, holeZ;
    m_ground->getHolePosition(holeX, holeZ);

    // **Adicione a verificação do estado STANDING**
    if (m_state == State::STANDING && gridX == holeX && gridZ == holeZ) {
      // O Cube está sobre o buraco **e está em pé**; inicia a queda
      m_isFalling = true;
      m_fallTime = 0.0f;
    }
    if (!m_ground->isTile(gridX, gridZ)) {
      m_isFalling = true;
      m_fallTime = 0.0f;
    }
  }
}

void Cube::moveUp() {
  if (m_isMoving || m_isFalling)
    return;

  m_isMoving = true;
  m_orientation = Orientation::UP;
  m_rotationDirection = -1;
}

void Cube::moveDown() {
  if (m_isMoving || m_isFalling)
    return;

  m_isMoving = true;
  m_orientation = Orientation::DOWN;
  m_rotationDirection = 1;
}

void Cube::moveLeft() {
  if (m_isMoving || m_isFalling)
    return;

  m_isMoving = true;
  m_orientation = Orientation::LEFT;
  m_rotationDirection = 1;
}

void Cube::moveRight() {
  if (m_isMoving || m_isFalling)
    return;

  m_isMoving = true;
  m_orientation = Orientation::RIGHT;
  m_rotationDirection = -1;
}

void Cube::resetGame() {
  bool positionValid = false;
  glm::vec3 newPosition;

  while (!positionValid) {
    newPosition = generateRandomPosition();

    // Converte as coordenadas do mundo para coordenadas do grid
    int gridX = static_cast<int>(round(newPosition.x / m_scale));
    int gridZ = static_cast<int>(round(newPosition.z / m_scale));

    int holeX, holeZ;
    if (m_ground != nullptr) {
      m_ground->getHolePosition(holeX, holeZ);
      if (gridX != holeX || gridZ != holeZ) {
        positionValid = true;
      }
    } else {
      positionValid = true;
    }
  }

  m_position = newPosition;
  m_state = State::STANDING;
  m_isMoving = false;
  m_isFalling = false;
  m_animationMatrix = glm::mat4{1.0f};
  m_angle = 0.0f;
  m_border = false;
  m_fallTime = 0.0f;

  // Reseta o Ground para gerar um novo buraco
  if (m_ground != nullptr) {
    m_ground->reset();
  }
}