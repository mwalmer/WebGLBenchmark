EM_JS(void, print, (const char* str), {
  console.log(UTF8ToString(str));
});

EM_JS(void, printi, (int i), {
  console.log(i);
});

EM_JS(void, printff, (float i), {
  console.log(i);
});

EM_JS(void, printd, (double i), {
  console.log(i);
});

EM_JS(void, printError, (const char* str), {
    console.error(str);
})

EM_JS(void, mousePos, (const char* str), {
    console.error(str);
})

EM_JS(void, initCanvas, (), {
    let canvas = document.querySelector("canvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
})

EM_JS(void, updateMetrics, (int fpsCount, int spriteCount), {
    let fps = document.querySelector("#fps");
    let sprite = document.querySelector("#sprite");
    fps.innerText = `fps: ${fpsCount}`;

    if(spriteCount === 0)
      sprite.innerText = "click anywhere to begin";
    else
      sprite.innerText = `sprite count: ${spriteCount}`;
})


static int mouseDownFlag = 0;
static int mouseX = 0;
static int mouseY = 0;
static vec3 color = {0, 0, 0};

EM_BOOL mousedown_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  mouseDownFlag = 1;
  mouseX = e->clientX;
  mouseY = e->clientY;

  vec3 c = {((float)rand() / (float)RAND_MAX) * 2 - 1.0,
  ((float)rand() / (float)RAND_MAX) * 2 - 1.0,
  ((float)rand() / (float)RAND_MAX) * 2 - 1.0};
  color = c;
  return 0;
}

EM_BOOL mouseup_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  mouseDownFlag = 0;
  return 0;
}

EM_BOOL mousemove_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  if(!mouseDownFlag)
    return 0;

  mouseX = e->clientX;
  mouseY = e->clientY;
  return 0;
}

EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *e, void *userData)
{
  initCanvas();
  emscripten_get_canvas_element_size("#glCanvas", &width, &height);
  return 0;
}

EM_BOOL touchstart_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  mouseDownFlag = 1;
  EmscriptenTouchPoint touch = e->touches[0];
  mouseX = touch.clientX;
  mouseY = touch.clientY;

  vec3 c = {((float)rand() / (float)RAND_MAX) * 2 - 1.0,
  ((float)rand() / (float)RAND_MAX) * 2 - 1.0,
  ((float)rand() / (float)RAND_MAX) * 2 - 1.0};
  color = c;
  return 0;
}

EM_BOOL touchend_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  mouseDownFlag = 0;
  return 0;
}

EM_BOOL touchmove_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  if(!mouseDownFlag)
    return 0;

  EmscriptenTouchPoint touch = e->touches[0];
  mouseX = touch.clientX;
  mouseY = touch.clientY;
  return 0;
}