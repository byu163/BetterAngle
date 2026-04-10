#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <vector>
#include <windows.h>


struct Profile {
  std::wstring name;
  double scale_normal;
  double scale_diving;
  double scale_gliding;

  // Reference Metadata
  std::string fov;
  std::string resolution;
  std::string render_scale;

  // Detector Logic
  int roi_x, roi_y, roi_w, roi_h;
  COLORREF target_color;
  int tolerance;

  bool Load(const std::wstring &path);
  bool Save(const std::wstring &path);
};

std::vector<Profile> GetProfiles(const std::wstring &directory);

#endif // PROFILE_H
