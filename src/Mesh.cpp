#include "Mesh.hpp"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures) {
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
    this->textures = std::move(textures);

    vbo = std::make_unique<VBO>(this->vertices);
    ebo = std::make_unique<EBO>(this->indices);

    vao.Bind();
    vbo->Bind();
    ebo->Bind();

    vao.linkAttrib(*vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.linkAttrib(*vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    vao.linkAttrib(*vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    vao.linkAttrib(*vbo, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    vao.linkAttrib(*vbo, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    vao.UnBind();
    vbo->UnBind();
    ebo->UnBind();
}

void Mesh::Draw(Shader& shader, Camera& camera, glm::mat4 matrix,
    glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {

    shader.use();
    vao.Bind();

    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;
    unsigned int numNormal = 0;
    unsigned int numHeight = 0;

    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string num;
        std::string type = textures[i].type_r;

        if (type == "texture_diffuse") {
            num = std::to_string(numDiffuse++);
        }
        else if (type == "texture_specular") {
            num = std::to_string(numSpecular++);
        }
        else if (type == "texture_normal") {
            num = std::to_string(numNormal++);
        }
        else if (type == "texture_height") {
            num = std::to_string(numHeight++);
        }

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].ID);

        std::string uniformName = type + num;
        shader.setInt(uniformName, i);
    }

    glm::mat4 trans = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 sca = glm::mat4(1.0f);

    trans = glm::translate(trans, translation);
    rot = glm::mat4_cast(rotation);
    sca = glm::scale(sca, scale);

    shader.setMat4("model", matrix * trans * rot * sca);
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix(800.0f / 600.0f)); // Можно передать aspect ratio как параметр

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    vao.UnBind();
}