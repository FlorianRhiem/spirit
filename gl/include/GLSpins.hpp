#ifndef GL_SPINS_H
#define GL_SPINS_H

#include <array>
#include <memory>

#include "glm/glm.hpp"
#include "Camera.hpp"
#include "options.hpp"
#include "utilities.hpp"

class ISpinRenderer;

class GLSpins
{
public:
  enum CameraMovementModes {
    TRANSLATE,
    ROTATE
  };
  enum WidgetLocation {
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_RIGHT
  };
  enum VisualizationMode {
    SURFACE,
    ISOSURFACE,
    ARROWS,
    SPHERE
  };
  enum Option {
    SHOW_BOUNDING_BOX,
    BOUNDING_BOX_MIN,
    BOUNDING_BOX_MAX,
    SYSTEM_CENTER,
    SHOW_MINIVIEW,
    MINIVIEW_LOCATION,
    SHOW_COORDINATE_SYSTEM,
    COORDINATE_SYSTEM_LOCATION,
    VISUALIZATION_MODE,
    TETRAHEDRA_INDICES
  };
  enum Colormap {
	HSV,
	HSV_NO_Z,
    BLUE_RED,
    BLUE_GREEN_RED,
    BLUE_WHITE_RED,
    OTHER
  };
  
  GLSpins(std::vector<int> n_cells, bool threeD=false);
  ~GLSpins();
  void draw();

  void mouseMove(const glm::vec2& position_before, const glm::vec2& position_after, CameraMovementModes mode);
  void mouseScroll(const float& wheel_delta);
  void setCameraToDefault();
  void setCameraToX();
  void setCameraToY();
  void setCameraToZ();
  void setCameraPositionTo(float x, float y, float z);
  void setCameraFocusTo(float x, float y, float z);
  void setCameraUpvectorTo(float x, float y, float z);
  glm::vec3 getCameraPosition();
  glm::vec3 getCameraFocus();
  glm::vec3 getCameraUpvector();
  void setFramebufferSize(float width, float height);
  float getFramerate() const;
  
  void updateOptions(const Options<GLSpins>& options);
  void options(const Options<GLSpins>& options);
  const Options<GLSpins>& options() const;
  
  void updateSpins(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& directions);
  void updateSystemGeometry(glm::vec3 bounds_min, glm::vec3 center, glm::vec3 bounds_max, const std::vector<std::array<int, 4>>& tetrahedra_indices);
  
private:
  void updateRenderers();
  void optionsHaveChanged(const std::vector<int>& changedOptions);
  
  std::vector<float> _positions;
  std::vector<float> _directions;
  std::vector<std::pair<std::shared_ptr<ISpinRenderer>, std::array<float, 4>>> _renderers;
  Camera _camera;
  FPSCounter _fps_counter;
  float _width;
  float _height;
  
  Options<GLSpins> _options;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::SHOW_BOUNDING_BOX> {
  bool default_value = true;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::BOUNDING_BOX_MIN> {
  glm::vec3 default_value = {-1, -1, -1};
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::BOUNDING_BOX_MAX> {
  glm::vec3 default_value = {1, 1, 1};
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::SYSTEM_CENTER> {
  glm::vec3 default_value = {0, 0, 0};
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::SHOW_MINIVIEW> {
  bool default_value = true;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::MINIVIEW_LOCATION> {
  GLSpins::WidgetLocation default_value = GLSpins::WidgetLocation::BOTTOM_LEFT;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::SHOW_COORDINATE_SYSTEM> {
  bool default_value = true;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::COORDINATE_SYSTEM_LOCATION> {
  GLSpins::WidgetLocation default_value = GLSpins::WidgetLocation::BOTTOM_RIGHT;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::VISUALIZATION_MODE> {
  GLSpins::VisualizationMode default_value = GLSpins::VisualizationMode::ARROWS;
};

template<> template<>
struct Options<GLSpins>::Option<GLSpins::Option::TETRAHEDRA_INDICES> {
  std::vector<std::array<int, 4>> default_value;
};

#endif