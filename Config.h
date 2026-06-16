#pragma once

#pragma once

#include <graphics.h>
#include <windows.h>
#include <conio.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <memory>
#include <map>

using namespace std;

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "winmm.lib")

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;

const double EPS = 0.001;

const double GRAVITY = 1.98;
const double JUMP_SPEED = 28.0;
const double MAX_FALL_SPEED = -28.0;