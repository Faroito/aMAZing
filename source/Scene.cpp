//
// Created by Timothée Couble on 17/10/2019.
//

#include "Scene.hpp"

scene::Scene::Scene() : App(1920, 1080, "Scene"), _player(_maze) {
    init();
}

scene::Scene::~Scene() {
    _maze.clear();
    for (auto &it : _crowd)
        delete it;
    for (auto &it : _models)
        delete it.second;
}
/*
std::ostream &operator<<(std::ostream &stream, const glm::vec3 &vec) {
    stream << vec.x << " " << vec.y << " " << vec.z;
    return stream;
}*/

void scene::Scene::init() {
    std::string vsModelPath = "../shader/model_vs.glsl";
    std::string fsModelPath = "../shader/model_fs.glsl";
    std::string vsLightPath = "../shader/light_vs.glsl";
    std::string fsLightPath = "../shader/light_fs.glsl";
    std::string vsTexturePath = "../shader/texture_vs.glsl";
    std::string fsTexturePath = "../shader/texture_fs.glsl";
    std::string vsParticlesPath = "../shader/instance_vs.glsl";
    std::string fsParticlesPath = "../shader/instance_fs.glsl";

    _shaders.push_back(std::make_unique<gl_wrapper::Shader>(
            gl_wrapper::Shader(vsModelPath, fsModelPath, gl_wrapper::ShaderType::MODEL)
    ));
    _shaders.push_back(std::make_unique<gl_wrapper::Shader>(
            gl_wrapper::Shader(vsLightPath, fsLightPath, gl_wrapper::ShaderType::LIGHT)
    ));
    _shaders.push_back(std::make_unique<gl_wrapper::Shader>(
            gl_wrapper::Shader(vsTexturePath, fsTexturePath, gl_wrapper::ShaderType::TEXTURE_DIFFUSE)
    ));
    _shaders.push_back(std::make_unique<gl_wrapper::Shader>(
            gl_wrapper::Shader(vsParticlesPath, fsParticlesPath, gl_wrapper::ShaderType::INSTANCE)
    ));

    std::string path = "../resource/cube.obj";
    _models.emplace(ModelType::CUBE, new Model(path));
    path = "../resource/ball.obj";
    _models.emplace(ModelType::BALL, new Model(path));

    _maze.init();

    for (int i = 0; i < 7; i++)
        _crowd.emplace_back(new Crowd());

    _dirLight.setAmbient(glm::vec3(0.5f, 0.5f, 0.5f));
    _dirLight.setShader(_shaders);

    _pointLights.emplace_back(scene::PointLight(glm::vec3(9.3f, 4.3f, 0.0f), 10));
    _pointLights.emplace_back(scene::PointLight(glm::vec3(1.2f, 3.5f, 0.0f), 25));
    _pointLights[0].setAmbient(glm::vec3(0.96f, 0.85f, 0.05f));
    _pointLights[1].setAmbient(glm::vec3(0.82f, 0.12f, 0.05f));
    _pointLights[1].setDiffuse(glm::vec3(0.82f, 0.12f, 0.05f));
    for (auto &it : _pointLights)
        it.setShader(_shaders);

    _particles.reset(_maze);
    for (auto &crowd : _crowd)
        crowd->reset(_maze);
}

void scene::Scene::onDraw() {
    glPolygonMode(GL_FRONT_AND_BACK, _mode);

    this->checkKey();

    double aTime = glfwGetTime();
    static bool end = false;
    static double lastTimeEnd = glfwGetTime();

    _player.update(_camera, _maze, _crowd);
    for (auto &crowd : _crowd)
        crowd->update(_maze, _player.getPosition(), _crowd);
    if (!end) {
        if (_maze.update(_player.getPosition())) {
            _particles.toggleActivation();
            end = true;
            lastTimeEnd = glfwGetTime();
        }
    } else if (aTime - lastTimeEnd > 10.05f) {
        _particles.toggleActivation();
        end = false;
        reset();
    }

    for (auto &shader : _shaders) {
        shader->bind();
        shader->setUniformMatrix4("view_matrix", _camera->getViewMatrix());
        shader->setUniformMatrix4("proj_matrix", _camera->getProjectionMatrix(getWindow()));
        if (shader->getType() == gl_wrapper::ShaderType::MODEL
            || shader->getType() == gl_wrapper::ShaderType::TEXTURE_DIFFUSE)
            shader->setUniformVector3("viewPos", _camera->getCameraPosition());
        if (shader->getType() == gl_wrapper::ShaderType::INSTANCE)
            _particles.draw(shader, _maze);
        gl_wrapper::Shader::unBind();
    }

    _maze.draw(_models, _shaders);
    _player.draw(_models, _shaders);
    for (auto &crowd : _crowd)
        crowd->draw(_models, _shaders);
}

void scene::Scene::reset() {
    _maze.reset();
    _particles.reset(_maze);
    _maze.update(_player.getPosition());
    for (auto &crowd : _crowd)
        crowd->reset(_maze);
}

void scene::Scene::checkKey() {
    for (const auto &it : _keyMapCamera) {
        if (_keyCode[it.first] && _pressed)
            (_camera.get()->*it.second)();
    }
    for (const auto &it : _keyMapPlayer)
        if (_keyCode[it.first] && _pressed)
            (_player.*it.second)();
    if (_keyCode[GLFW_KEY_SPACE])
        for (auto &crowd : _crowd)
            crowd->doJump();
    if (_keyCode[GLFW_KEY_ESCAPE])
        getWindow().setClose(true);
    if (_keyCode[GLFW_KEY_X] && _pressed) {
        if (_mode == GL_FILL)
            _mode = GL_LINE;
        else
            _mode = GL_FILL;
        _pressed = false;
    }
    if (_keyCode[GLFW_KEY_ESCAPE])
        getWindow().setClose(true);
}

void scene::Scene::onMouseScroll(double x, double y) {
    _camera->zoom(y);
}
