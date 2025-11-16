// Eletronic & Eletronic Circut Semester Prject: I dont car
// Version 0.2
// Content: Rolling chassis pin setup, control test code

// ----- constant setup -----
const int MAX_PWM = 255; // maxium PWM value
const float DEFAULT_SPEED = 0.6; // WASD control default speed scale

// ----- motor variable setup -----
struct Motor {
  uint8_t pinIn1;
  uint8_t pinIn2;
  uint8_t pinEn;
  bool invert;   // 若馬達方向相反，可設為 true

  Motor(uint8_t in1, uint8_t in2, uint8_t en, bool inv = false)
    : pinIn1(in1), pinIn2(in2), pinEn(en), invert(inv) {}

  void begin() {
    pinMode(pinIn1, OUTPUT);
    pinMode(pinIn2, OUTPUT);
    pinMode(pinEn, OUTPUT);
    stop();
  }

  void stop() {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
    analogWrite(pinEn, 0);
  }

  // speed: -255 ~ +255
  void setSpeed(int speed) {
    if (invert) {
      speed = -speed;  // 方向反轉
    }

    speed = constrain(speed, -MAX_PWM, MAX_PWM);

    if (speed > 0) {
      digitalWrite(pinIn1, HIGH);
      digitalWrite(pinIn2, LOW);
      analogWrite(pinEn, speed);
    } else if (speed < 0) {
      digitalWrite(pinIn1, LOW);
      digitalWrite(pinIn2, HIGH);
      analogWrite(pinEn, -speed);
    } else {
      // 停止
      stop();
    }
  }
};

// ----- Pin setup -----
// L298N #1 for left
// Front Left
const uint8_t FL_IN1 = 22;
const uint8_t FL_IN2 = 23;
const uint8_t FL_EN  = 5;   // PWM

// Rear Left
const uint8_t RL_IN1 = 24;
const uint8_t RL_IN2 = 25;
const uint8_t RL_EN  = 6;   // PWM

// L298N #2 for right
// Front Right
const uint8_t FR_IN1 = 26;
const uint8_t FR_IN2 = 27;
const uint8_t FR_EN  = 7;   // PWM

// Rear Right
const uint8_t RR_IN1 = 28;
const uint8_t RR_IN2 = 29;
const uint8_t RR_EN  = 8;   // PWM

// ----- 建立四顆馬達物件 -----
// 若某顆馬達方向相反，可把最後一個參數改成 true
Motor motorFL(FL_IN1, FL_IN2, FL_EN, false);
Motor motorFR(FR_IN1, FR_IN2, FR_EN, false);
Motor motorRL(RL_IN1, RL_IN2, RL_EN, false);
Motor motorRR(RR_IN1, RR_IN2, RR_EN, false);

// vvvvv By ChatGPT 5.1 Thinking vvvvv
// vx: 前進(+) / 後退(-)
// vy: 右(+) / 左(-)
// wz: 逆時針(+) / 順時針(-)
// 三者範圍建議 -1.0 ~ +1.0
void setMecanum(float vx, float vy, float wz) {
  // 計算各輪理論速度
  float fl = vx - vy - wz;
  float fr = vx + vy + wz;
  float rl = vx + vy - wz;
  float rr = vx - vy + wz;

  // 找出最大絕對值，若大於 1.0 就全部等比例縮小
  float maxVal = fabs(fl);
  if (fabs(fr) > maxVal) maxVal = fabs(fr);
  if (fabs(rl) > maxVal) maxVal = fabs(rl);
  if (fabs(rr) > maxVal) maxVal = fabs(rr);

  if (maxVal < 1.0f) {
    maxVal = 1.0f; // 避免被放大
  }

  fl /= maxVal;
  fr /= maxVal;
  rl /= maxVal;
  rr /= maxVal;

  // 映射到實際 PWM
  int flPWM = (int)(fl * MAX_PWM);
  int frPWM = (int)(fr * MAX_PWM);
  int rlPWM = (int)(rl * MAX_PWM);
  int rrPWM = (int)(rr * MAX_PWM);

  motorFL.setSpeed(flPWM);
  motorFR.setSpeed(frPWM);
  motorRL.setSpeed(rlPWM);
  motorRR.setSpeed(rrPWM);
}

// Stop every motor
void stopAll() {
  motorFL.stop();
  motorFR.stop();
  motorRL.stop();
  motorRR.stop();
  // add arm in future
}

// WASD motor control (for testing)
void handleSerialCommand() {
  if (!Serial.available()) return;

  char c = Serial.read();
  c = tolower(c);

  switch (c) {
    case 'w': // move foward
      setMecanum(DEFAULT_SPEED, 0.0, 0.0);
      Serial.println("Forward");
      break;

    case 's': // move backward
      setMecanum(-DEFAULT_SPEED, 0.0, 0.0);
      Serial.println("Backward");
      break;

    case 'a': // strade left
      setMecanum(0.0, -DEFAULT_SPEED, 0.0);
      Serial.println("Strafe Left");
      break;

    case 'd': // strafe right
      setMecanum(0.0, DEFAULT_SPEED, 0.0);
      Serial.println("Strafe Right");
      break;

    case 'q': // rotate anti-clockwise
      setMecanum(0.0, 0.0, DEFAULT_SPEED);
      Serial.println("Rotate aCW");
      break;

    case 'e': // rotate clockwise
      setMecanum(0.0, 0.0, -DEFAULT_SPEED);
      Serial.println("Rotate CW");
      break;

    case 'x': // stop
      stopAll();
      Serial.println("Stop");
      break;

    default:
      // 不認得的指令就忽略
      break;
  }
}

void setup() {
  Serial.begin(115200);

  motorFL.begin();
  motorFR.begin();
  motorRL.begin();
  motorRR.begin();

  stopAll();

  Serial.println("Mecanum car ready. Use WASDQE + X over Serial:");
  Serial.println("---------- How to Control ----------");
  Serial.println("  W: forward, S: backward");
  Serial.println("  A: left,    D: right");
  Serial.println("  Q: rotate anti-clockwise (aCW)");
  Serial.println("  E: rotate clockwise (CW)");
  Serial.println("  X: stop");
}

void loop() {
  handleSerialCommand();
}
