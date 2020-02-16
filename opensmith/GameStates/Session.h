#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GameState.h"
#include "../View.h"
#include "../Model.h"
#include "../Controller.h"

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