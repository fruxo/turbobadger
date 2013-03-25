#ifndef APP_H
#define APP_H

// == CALLS FROM NATIVE TO JAVA =========================================================
void ShowKeyboard(bool show);

// == CALLS FROM JAVA TO NATIVE =========================================================
void Init(unsigned int width, unsigned int height);
void Resize(unsigned int width, unsigned int height);
void Update();
void Render();
void Shutdown();

#endif // APP_H
