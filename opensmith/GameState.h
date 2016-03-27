#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "View.h"
#include "Model.h"
#include "Controller.h"

class GameState
{
public:
	static GameState* gameState;
	static GLFWwindow* window;
	virtual ~GameState() {}
	virtual void keyPressed(int key) {};
	virtual void draw(double time) {};
};

class Session : public GameState
{
public:
	Session();
	~Session();
	void keyPressed(int key);
	void draw(double time);
private:
	View* v;
	Controller* c;
	Model* m;
};