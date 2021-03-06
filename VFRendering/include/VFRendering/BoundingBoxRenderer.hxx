#ifndef VFRENDERING_BOUNDING_BOX_RENDERER_HXX
#define VFRENDERING_BOUNDING_BOX_RENDERER_HXX

#include <VFRendering/RendererBase.hxx>

namespace VFRendering {
class BoundingBoxRenderer : public RendererBase {
public:
    enum Option {
        COLOR = 600
    };


    BoundingBoxRenderer(const View& view, const std::vector<glm::vec3>& vertices);
    static BoundingBoxRenderer forCuboid(const View& view, const glm::vec3& center, const glm::vec3& side_lengths);
    static BoundingBoxRenderer forParallelepiped(const View& view, const glm::vec3& center, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
    static BoundingBoxRenderer forHexagonalCell(const View& view, const glm::vec3& center, float radius, float height);
    virtual ~BoundingBoxRenderer();
    virtual void draw(float aspect_ratio) override;
    virtual void optionsHaveChanged(const std::vector<int>& changed_options) override;

protected:
    virtual void update(bool keep_geometry) override;

private:
    void initialize();
    
    bool m_is_initialized = false;
    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    std::vector<glm::vec3> m_vertices;
};

namespace Utilities {
template<>
struct Options::Option<BoundingBoxRenderer::Option::COLOR>{
    glm::vec3 default_value = {1.0, 1.0, 1.0};
};
}
}

#endif
