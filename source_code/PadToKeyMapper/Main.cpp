#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <Windows.h>
#include <Xinput.h>

using namespace std;

// Button State
struct ButtonState {
	bool is_pressed;
	int gamepad_key;
	int keyboard_key;
	ButtonState() : ButtonState(0, 0) {}
	ButtonState(int gamepad_key, int keyboard_key) : gamepad_key(gamepad_key), keyboard_key(keyboard_key) {
		is_pressed = false;
	}
};

// Trigger State
struct TriggerState {
	bool is_pressed;
	int keyboard_key;
	TriggerState() : TriggerState(0) {}
	TriggerState(int keyboard_key) : keyboard_key(keyboard_key) {
		is_pressed = false;
	}
};

// Input Timeout
const int g_inputTimeout = 1;
// Trigger Treshold
const byte g_triggerTreshold = 16;
// Config file
string g_configFileName = "config.txt";
// Gamepad
bool g_isGamepadConnected = false;
XINPUT_STATE g_gamepadState = {};
// Button States
vector<ButtonState> g_buttonStates = vector<ButtonState>();
// Trigger States
TriggerState g_leftTriggerState = TriggerState(0);
TriggerState g_rightTriggerState = TriggerState(0);

// Send virtual keyboard event
void sendVirtualKeyEvent(int keycode, bool pressed) {
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = keycode;
	if (pressed) { input.ki.dwFlags = 0; }
	else { input.ki.dwFlags = KEYEVENTF_KEYUP; }
	SendInput(1, &input, sizeof(INPUT));
}

// Process Input
void processInput() {
	// Get Gamepad State
	g_isGamepadConnected = (XInputGetState(0, &g_gamepadState) == ERROR_SUCCESS);
	if (g_isGamepadConnected) {
		// Check button states
		for (int i = 0; i < g_buttonStates.size(); i++) {
			ButtonState& buttonState = g_buttonStates.at(i);
			bool button_curr = g_gamepadState.Gamepad.wButtons & buttonState.gamepad_key;
			// Send Virtual Keyboard event if button state has changed
			if (button_curr != buttonState.is_pressed) { sendVirtualKeyEvent(buttonState.keyboard_key, button_curr); }
			buttonState.is_pressed = button_curr;
		}
		// Check left trigger state
		bool leftTriggerCurr = g_gamepadState.Gamepad.bLeftTrigger >= g_triggerTreshold;
		if (leftTriggerCurr != g_leftTriggerState.is_pressed) { sendVirtualKeyEvent(g_leftTriggerState.keyboard_key, leftTriggerCurr); }
		g_leftTriggerState.is_pressed = leftTriggerCurr;
		// Check right trigger state
		bool rightTriggerCurr = g_gamepadState.Gamepad.bRightTrigger >= g_triggerTreshold;
		if (rightTriggerCurr != g_rightTriggerState.is_pressed) { sendVirtualKeyEvent(g_rightTriggerState.keyboard_key, rightTriggerCurr); }
		g_rightTriggerState.is_pressed = rightTriggerCurr;
	}
}

// Tokenize String
void tokenizeString(std::string const& str, const char delim, std::vector<std::string>& out) {
	size_t start;
	size_t end = 0;
	while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

// Read Config File
bool readConfigFile() {
	string configFileContent;
	string line;
	// Read complete config file
	ifstream file(g_configFileName);
	if (file.is_open()) {
		while (getline(file, line)) {
			configFileContent = configFileContent + line + " ";
		}
		file.close();
	}
	else {
		return false;
	}
	// Vectorize config file content
	std::vector<std::string> vec;
	tokenizeString(configFileContent, ' ', vec);
	// Map gamepad keys to keyboard keys
	for (int i = 0; i < vec.size(); i = i + 2) {
		// Get gamepad key
		string gamepadkey = vec.at(i);
		// Get keyboard key
		int keyboardkey = stoi(vec.at(i + 1), 0, 16);
		// Map to Button and trigger states
		if (gamepadkey.compare("UP") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_DPAD_UP, keyboardkey)); }
		else if (gamepadkey.compare("DOWN") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_DPAD_DOWN, keyboardkey)); }
		else if (gamepadkey.compare("LEFT") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_DPAD_LEFT, keyboardkey)); }
		else if (gamepadkey.compare("RIGHT") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_DPAD_RIGHT, keyboardkey)); }
		else if (gamepadkey.compare("START") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_START, keyboardkey)); }
		else if (gamepadkey.compare("BACK") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_BACK, keyboardkey)); }
		else if (gamepadkey.compare("LSB") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_LEFT_THUMB, keyboardkey)); }
		else if (gamepadkey.compare("RSB") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_RIGHT_THUMB, keyboardkey)); }
		else if (gamepadkey.compare("LT") == 0) { g_leftTriggerState = TriggerState(keyboardkey); }
		else if (gamepadkey.compare("RT") == 0) { g_rightTriggerState = TriggerState(keyboardkey); }
		else if (gamepadkey.compare("LB") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_LEFT_SHOULDER, keyboardkey)); }
		else if (gamepadkey.compare("RB") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_RIGHT_SHOULDER, keyboardkey)); }
		else if (gamepadkey.compare("A") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_A, keyboardkey)); }
		else if (gamepadkey.compare("B") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_B, keyboardkey)); }
		else if (gamepadkey.compare("X") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_X, keyboardkey)); }
		else if (gamepadkey.compare("Y") == 0) { g_buttonStates.push_back(ButtonState(XINPUT_GAMEPAD_Y, keyboardkey)); }
	}
	return true;
}

// Main
int main(int argc, char* argv[]) {
	if (argc == 2) {
		g_configFileName = argv[1];
	}
	// Read Config File
	if (readConfigFile()) {
		cout << "Loaded config file: " << g_configFileName << endl;
	}
	else {
		cout << "ERROR: No config file found." << endl;
		cout << "Place your config file \"config.txt\" in the same folder with PadToKeyMapper.exe." << endl;
		cout << "Or call the program with \"PadToKeyMapper.exe <path to your config file>\"." << endl;
		return EXIT_FAILURE;
	}
	cout << "Gamepad to keyboard mapper started." << endl;
	// Main loop
	while (true) {
		// Sleep
		Sleep(g_inputTimeout);
		// Process Input
		processInput();
	}
	return EXIT_SUCCESS;
}