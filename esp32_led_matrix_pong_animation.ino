#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define R1_PIN 4
#define G1_PIN 5
#define B1_PIN 6
#define R2_PIN 17
#define G2_PIN 18
#define B2_PIN 8
#define A_PIN 10
#define B_PIN 11
#define C_PIN 12
#define D_PIN 13
#define E_PIN 9
#define LAT_PIN 47
#define OE_PIN 21
#define CLK_PIN 16

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

MatrixPanel_I2S_DMA *dma_display = nullptr;

class Paddle{
  int8_t x;
  int8_t y;
  uint16_t r;
  uint16_t g;
  uint16_t b;
  static uint8_t w;
  static uint8_t h;

  void moveFrame(int8_t ballXVel, int8_t ballYPos) {
    // Ball moving towards paddle
    if (((ballXVel < 0) && (x < PANEL_RES_X/2)) || ((ballXVel > 0) && (x > PANEL_RES_X/2))) {
      int8_t yNew = ballYPos - h / 2;
      // Cheap solution to stop crazy amounts of paddle teleporting
      if ((abs(yNew-y) < 4)) {
        y = yNew;
      } else {
        if (yNew < y) {
          y -= 3;
        } else {
          y += 3;
        }
      }
    }

    if (y < 0) {
      y = 0;
    } else if (y+h > PANEL_RES_Y-1) {
      y = PANEL_RES_Y - h;
    }
  }

public:
  Paddle(int8_t paddleX, int8_t paddleY,
    uint16_t paddleR, uint16_t paddleG, uint16_t paddleB) {
    x = paddleX;
    y = paddleY;
    r = paddleR;
    g = paddleG;
    b = paddleB;
  }

  static uint8_t getWidth() { return w; }
  static uint8_t getHeight() { return h; }

  void drawPaddle(int8_t ballXVel, int8_t ballYPos) {
    moveFrame(ballXVel, ballYPos);
    dma_display->fillRect(x, y, w, h, dma_display->color565(r, g, b));
  }
};

class Ball {
  int8_t x;
  int8_t y;
  uint8_t r;
  int8_t xVel;
  int8_t yVel;

  void collidesPaddle() {
    xVel = -xVel;
    yVel = random(-2, 3);
  }

  void collidesWall() {
    if ((y-r)<=0) {
      y = r;
    } else {
      y = PANEL_RES_Y - (r + 1);
    }
    yVel = -yVel;
  }

  void checkCollides() {
    if ((x-r)<=(3+Paddle::getWidth()) || (x+r)>=PANEL_RES_X-(4+Paddle::getWidth())) {
      collidesPaddle();
    }
    if ((y-r)<=0 || (y+r)>=PANEL_RES_Y-1) {
      collidesWall();
    }
  }

  void moveFrame() {
    x += xVel;
    y += yVel;
    checkCollides();
  }

public:
  Ball(int8_t ballX, int8_t ballY, uint8_t ballR = 2, int8_t ballXVel = 1, int8_t ballYVel = 0) {
    x = ballX;
    y = ballY;
    r = ballR;
    xVel = ballXVel;
    yVel = ballYVel;
  }

  int8_t getY() const { return y; }

  int8_t getXVel() const { return xVel; }

  void drawBall() {
    moveFrame();
    dma_display->fillCircle(x, y, r, dma_display->color565(90, 90, 90));
  }
};

uint8_t Paddle::w = 4;
uint8_t Paddle::h = 20;

Ball ball(PANEL_RES_X/2, PANEL_RES_Y/2);
Paddle redPaddle(3, (PANEL_RES_Y-Paddle::getHeight())/2, 70, 0, 0);
Paddle bluePaddle(PANEL_RES_X-(3+Paddle::getWidth()), (PANEL_RES_Y-Paddle::getHeight())/2, 0, 0, 110);

void setup() {
  // Matrix configuration
  Serial.begin(9600);
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X, // Module width
    PANEL_RES_Y, // Module height
    PANEL_CHAIN, // chain length
    _pins // pin mapping
  );
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // Preparing the matrix
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
  dma_display->clearScreen();
}

void loop() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();

  if (now - lastFrame >= 16) { // ~60 FPS
    lastFrame = now;

    dma_display->clearScreen();
    ball.drawBall();
    redPaddle.drawPaddle(ball.getXVel(), ball.getY());
    bluePaddle.drawPaddle(ball.getXVel(), ball.getY());
  }
}
