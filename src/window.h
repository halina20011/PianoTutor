#ifndef WINDOW
#define WINDOW

void setWindow(float x, float y, float width, float height);

float windowX(float x);
float windowY(float y);
float windowWidth(float width);
float windowHeight(float height);

#define FORMAT(x, y, w, h) windowX(x), windowY(y), windowWidth(w), windowHeight(h)

#define FORMAT_2P(x1, y1, x2, y2) windowX(x1), windowY(y1), windowX(x2), windowY(y2)

#define FORMAT_P(x1, y1) windowX(x1), windowY(y1)

#define FORMAT_W(w) windowWidth(w)
#define FORMAT_H(h) windowHeight(h)

#endif
