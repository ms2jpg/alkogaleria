/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <libs/glew/include/GL/glew.h>
#include <libs/GLFW/include/GLFW/glfw3.h>
#include <libs/glm/glm.hpp>
#include <libs/glm/gtc/type_ptr.hpp>
#include <libs/glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include <lodepng.h>
#include "shaderprogram.h"
#include <map/RWModel.h>
#include <map/RWObject.h>
#include <map/Map.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <Game.h>
#include "main_file.h"

using nlohmann::json;

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void initOpenGLProgram(GLFWwindow* window) {
	if (!window) {
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	if (glewInit() != GLEW_OK) { // initialize glew
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initShaders();

	glClearColor(0, 0, 0, 1); // set clear buffer color
	glEnable(GL_DEPTH_TEST); // enable pixel depth test
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	//************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, Game* game) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spTextured->use();
	glUniform4f(spTextured->u("color"), 0, 1, 0, 1); // set object color
	glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(game->P)); // load perspective matrix to shader program
	glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(game->V)); // load view matrix to shader program
	
	// loop through objects
	for (int modelIndex = 0; modelIndex < game->map->objects.size(); modelIndex++) {
		int objectsNumber = game->map->objects[modelIndex]->objectsNumber;
		for (int objectIndex = 0; objectIndex < objectsNumber; objectIndex++) {
			RWObject* object = &(game->map->objects[modelIndex]->objects[objectIndex]);
			glm::mat4 M = object->M;
			
			
			glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M)); // load model matrix
			// vertices
			glEnableVertexAttribArray(spTextured->a("vertex"));
			glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, object->vertices);

			// normals
			if (object->hasNormals) {
				glEnableVertexAttribArray(spTextured->a("normal"));
				glVertexAttribPointer(spTextured->a("normal"), 4, GL_FLOAT, false, 0, object->normals);
			}

			// textures
			glEnableVertexAttribArray(spTextured->a("texCoord"));
			glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, object->texCoords);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *(object->texture));
			glUniform1i(spTextured->u("tex"), 0);

			glDrawElements(
				GL_TRIANGLES,
				object->vertexIndicesCount * 3,
				GL_UNSIGNED_INT,
				object->vertexIndices
			);

			glDisableVertexAttribArray(spTextured->a("vertex"));
			if (object->hasNormals) {
				glDisableVertexAttribArray(spTextured->a("normal"));
			}
			glDisableVertexAttribArray(spTextured->a("texCoord"));			
		}
	}
	glfwSwapBuffers(window); // swap front and back buffers
}


int main(void)
{
	// INITIALIZE OPENGL
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	GLFWwindow* window = glfwCreateWindow(1280, 1280, "Alkogaleria", NULL, NULL);
	initOpenGLProgram(window);

	Game &game = Game::getInstance();
	glfwSetKeyCallback(window, &Game::keyCallback_handler);

	glfwSetTime(0); //reset time
	while (!glfwWindowShouldClose(window))
	{
		game.timePassed(glfwGetTime());
		glfwSetTime(0); // reset timer
		drawScene(window, &game);
		glfwPollEvents(); // execute callbacks if needed
	}

	freeOpenGLProgram(window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
