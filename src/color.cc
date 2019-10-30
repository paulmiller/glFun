#include "color.h"

const Color Color::Black {0, 0, 0};
const Color Color::White {1, 1, 1};

// h,s,v ∈ [0,1]; (0,1,1) and (1,1,1) are both bright red
Color ColorFromHsv(float h, float s, float v) {
  float c = v * s;
  float m = v - c;
  float h6 = h * 6;
  if(h6 < 1)
    return Color{ m + c          , m + c * h6    , m              };
  else if(h6 < 2)
    return Color{ m + c * (2-h6) , m + c         , m              };
  else if(h6 < 3)
    return Color{ m              , m + c         , m + c * (h6-2) };
  else if(h6 < 4)
    return Color{ m              , m + c * (4-h6), m + c          };
  else if(h6 < 5)
    return Color{ m + c * (h6-4) , m             , m + c          };
  else
    return Color{ m + c          , m             , m + c * (6-h6) };
}
