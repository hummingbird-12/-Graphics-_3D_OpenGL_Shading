#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <queue>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
#include "forShading.h"
GLuint h_ShaderProgram_simple, h_ShaderProgram_PS, h_ShaderProgram_GS; // handle to shader program
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color; // indices of uniform variables

#define NUMBER_OF_LIGHT_SUPPORTED 1 // lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;

#define PHONG	0
#define GOURAUD 1
// for Phone Shading shaders
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;

int shader_selected;
GLuint *shader_program;
GLint *loc_ModelViewProjectionMatrix;
GLint *loc_ModelViewMatrix;
GLint *loc_ModelViewMatrixInvTrans;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER;
//glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()
bool pause_animation;
float car_rotation_angle;

#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f

#define NUMBER_OF_CAMERAS 8
#define MAIN_CAM 0
#define FRONT_CAM 1
#define SIDE_CAM 2
#define TOP_CAM 3
#define CCTV_1 4
#define CCTV_2 5
#define CCTV_3 6
#define CCTV_DYN 7

typedef struct _CAMERA {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
	float fov_y, aspect_ratio, near_clip, far_clip;
	int move_status;
} CAMERA;
CAMERA camera[NUMBER_OF_CAMERAS];
int camera_selected;

typedef struct _VIEWPORT {
	int x, y, w, h;
} VIEWPORT;
VIEWPORT viewport[NUMBER_OF_CAMERAS];

int ViewMode;
#define EXTERIOR_MODE 0
#define INTERIOR_MODE 1

typedef struct _CALLBACK_CONTEXT {
	int left_button, right_button;
	int prevX, prevY;
} CALLBACK_CONTEXT;
CALLBACK_CONTEXT CC;

typedef enum { X, Y } TRIGGER;
typedef enum { CLOCKWISE, COUNTERCLOCKWISE } DIRECTION;
typedef struct _TIGER_PATH_POINT {
	glm::vec3 center;
	TRIGGER trigger;
	DIRECTION direction;
	float radius;
	float angle;
} TIGER_PATH_POINT;

glm::mat4 TigerModellingMatrix;
int tiger_inRotation;
std::queue < TIGER_PATH_POINT > tiger_path_queue;

//glm::mat4 ModelViewMatrix, ViewMatrix, ProjectionMatrix;
glm::mat4 ViewMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ProjectionMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewProjectionMatrix; // Mp * Mv * Mm
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

void set_ViewMatrix(int camera_id) {
	ViewMatrix[camera_id] = glm::mat4(1.0f);
	ViewMatrix[camera_id][0].x = camera[camera_id].uaxis.x;
	ViewMatrix[camera_id][0].y = camera[camera_id].vaxis.x;
	ViewMatrix[camera_id][0].z = camera[camera_id].naxis.x;

	ViewMatrix[camera_id][1].x = camera[camera_id].uaxis.y;
	ViewMatrix[camera_id][1].y = camera[camera_id].vaxis.y;
	ViewMatrix[camera_id][1].z = camera[camera_id].naxis.y;

	ViewMatrix[camera_id][2].x = camera[camera_id].uaxis.z;
	ViewMatrix[camera_id][2].y = camera[camera_id].vaxis.z;
	ViewMatrix[camera_id][2].z = camera[camera_id].naxis.z;

	ViewMatrix[camera_id] = glm::translate(ViewMatrix[camera_id], -camera[camera_id].pos);
}

#include "Object_Definitions.h"

void display_camera(int camera_id) {
	glViewport(viewport[camera_id].x, viewport[camera_id].y, viewport[camera_id].w, viewport[camera_id].h);

	/*/
	glm::vec4 position_EC = ViewMatrix[camera_id] * glm::vec4(light[0].position[0], light[0].position[1],
		light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light[0].position, 1, &position_EC[0]);
	*/
	

	glUseProgram(h_ShaderProgram_simple);
	glLineWidth(2.0f);
	draw_axes(camera_id);
	glLineWidth(1.0f);

	for (int i = 0; i < NUMBER_OF_CAMERAS; i++) {
		glPointSize(10.0f);
		draw_camera(camera_id, i);
		glPointSize(1.0f);
	}

	if (camera_id != MAIN_CAM) {
		float cx, cy, cz;
		float offset;
		glm::vec3 farTR, farTL, farBL, farBR;
		glm::vec3 nearTR, nearTL, nearBL, nearBR;
		CAMERA tempCamera;

		tempCamera = camera[MAIN_CAM];

		cx = tempCamera.pos.x;
		cy = tempCamera.pos.y;
		cz = tempCamera.pos.z;

		// far clip coordinates
		offset = glm::tan(tempCamera.fov_y * TO_RADIAN / 2) * tempCamera.far_clip;
		farTR = tempCamera.pos +
			glm::normalize(tempCamera.uaxis) * offset +
			glm::normalize(tempCamera.vaxis) * offset -
			glm::normalize(tempCamera.naxis) * tempCamera.far_clip;
		farTL = farTR - tempCamera.uaxis * (offset * 2);
		farBR = farTR - tempCamera.vaxis * (offset * 2);
		farBL = farBR - tempCamera.uaxis * (offset * 2);

		// near clip coordinates
		offset = glm::tan(tempCamera.fov_y * TO_RADIAN / 2) * tempCamera.near_clip;
		nearTR = tempCamera.pos +
			glm::normalize(tempCamera.uaxis) * offset +
			glm::normalize(tempCamera.vaxis) * offset -
			glm::normalize(tempCamera.naxis) * tempCamera.near_clip;
		nearTL = nearTR - tempCamera.uaxis * (offset * 2);
		nearBR = nearTR - tempCamera.vaxis * (offset * 2);
		nearBL = nearBR - tempCamera.uaxis * (offset * 2);

		draw_line(camera_id, cx, cy, cz, farTR.x, farTR.y, farTR.z, 255, 255, 255);
		draw_line(camera_id, cx, cy, cz, farTL.x, farTL.y, farTL.z, 255, 255, 255);
		draw_line(camera_id, cx, cy, cz, farBR.x, farBR.y, farBR.z, 255, 255, 255);
		draw_line(camera_id, cx, cy, cz, farBL.x, farBL.y, farBL.z, 255, 255, 255);

		// far clip plane
		draw_line(camera_id, farTL.x, farTL.y, farTL.z, farTR.x, farTR.y, farTR.z, 180, 0, 210);
		draw_line(camera_id, farTL.x, farTL.y, farTL.z, farBL.x, farBL.y, farBL.z, 180, 0, 210);
		draw_line(camera_id, farBR.x, farBR.y, farBR.z, farTR.x, farTR.y, farTR.z, 180, 0, 210);
		draw_line(camera_id, farBR.x, farBR.y, farBR.z, farBL.x, farBL.y, farBL.z, 180, 0, 210);

		// near clip plane
		draw_line(camera_id, nearTL.x, nearTL.y, nearTL.z, nearTR.x, nearTR.y, nearTR.z, 180, 0, 210);
		draw_line(camera_id, nearTL.x, nearTL.y, nearTL.z, nearBL.x, nearBL.y, nearBL.z, 180, 0, 210);
		draw_line(camera_id, nearBR.x, nearBR.y, nearBR.z, nearTR.x, nearTR.y, nearTR.z, 180, 0, 210);
		draw_line(camera_id, nearBR.x, nearBR.y, nearBR.z, nearBL.x, nearBL.y, nearBL.z, 180, 0, 210);
	}

	glUseProgram(*shader_program);

	draw_car(camera_id);

	draw_static_object(&(static_objects[OBJ_BUILDING]), 0, camera_id);

	draw_static_object(&(static_objects[OBJ_TABLE]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_TABLE]), 1, camera_id);

	draw_static_object(&(static_objects[OBJ_LIGHT]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 1, camera_id);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 2, camera_id);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 3, camera_id);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 4, camera_id);

	draw_static_object(&(static_objects[OBJ_TEAPOT]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 1, camera_id);
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 2, camera_id);

	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_FRAME]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_COW]), 0, camera_id);

	draw_static_object(&(static_objects[OBJ_IRONMAN]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_IRONMAN]), 1, camera_id);

	draw_static_object(&(static_objects[OBJ_TANK]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_TANK]), 1, camera_id);

	draw_static_object(&(static_objects[OBJ_GODZILLA]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_BUS]), 0, camera_id);
	draw_static_object(&(static_objects[OBJ_BIKE]), 0, camera_id);

	set_material_tiger();
	draw_animated_tiger(camera_id);

	glUseProgram(0);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ModelMatrix_CAR_BODY = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 85.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, car_rotation_angle * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(0.0f, 160.0f, 20.0f));
	ModelMatrix_CAR_BODY = glm::scale(ModelMatrix_CAR_BODY, glm::vec3(4.0f, 4.0f, 4.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	if (ViewMode == EXTERIOR_MODE) { // MAIN_CAM, SIDE_CAM, FRONT_CAM, TOP_CAM
		display_camera(MAIN_CAM);
		display_camera(SIDE_CAM);
		display_camera(FRONT_CAM);
		display_camera(TOP_CAM);
	}
	else { // INTERIOR_MODE : CCTV_DYN, CCTV_1, CCTV_2, CCTV_3
		display_camera(CCTV_DYN);
		display_camera(CCTV_1);
		display_camera(CCTV_2);
		display_camera(CCTV_3);
	}
	glutSwapBuffers();
}

void camera_translate(int camera_id, float displacement, glm::vec3 axis) {
	camera[camera_id].pos += CAM_TRANSLATION_SPEED * displacement * axis;
}

void camera_rotate(int camera_id, float angle, glm::vec3 axis) {
	glm::mat3 rotation;

	rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED * TO_RADIAN * angle, axis));

	camera[camera_id].uaxis = rotation * camera[camera_id].uaxis;
	camera[camera_id].vaxis = rotation * camera[camera_id].vaxis;
	camera[camera_id].naxis = rotation * camera[camera_id].naxis;
}

void initialize_lights_and_material(void);
void set_up_scene_lights(void);

void switch_shader_to(int shader) {
	char string[256];

	if (shader == PHONG) {
		shader_selected = PHONG;
		shader_program = &h_ShaderProgram_PS;

		loc_ModelViewProjectionMatrix = &loc_ModelViewProjectionMatrix_PS;
		loc_ModelViewMatrix = &loc_ModelViewMatrix_PS;
		loc_ModelViewMatrixInvTrans = &loc_ModelViewMatrixInvTrans_PS;
	}
	else { // GOURAUD SHADING
		shader_selected = GOURAUD;
		shader_program = &h_ShaderProgram_GS;

		loc_ModelViewProjectionMatrix = &loc_ModelViewProjectionMatrix_GS;
		loc_ModelViewMatrix = &loc_ModelViewMatrix_GS;
		loc_ModelViewMatrixInvTrans = &loc_ModelViewMatrixInvTrans_GS;
	}

	glUseProgram(*shader_program);

	loc_global_ambient_color = glGetUniformLocation(*shader_program, "u_global_ambient_color");
	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(*shader_program, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(*shader_program, string);
	}

	loc_material.ambient_color = glGetUniformLocation(*shader_program, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(*shader_program, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(*shader_program, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(*shader_program, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(*shader_program, "u_material.specular_exponent");

	initialize_lights_and_material();
	set_up_scene_lights();

	glUseProgram(0);
}

#define SCROLL_UP 3
#define SCROLL_DOWN 4
void mouse(int button, int state, int x, int y) {
	int target_cam = (ViewMode == EXTERIOR_MODE ? MAIN_CAM : CCTV_DYN);

	switch (button) {
	case GLUT_LEFT_BUTTON: // left click
		if (state == GLUT_DOWN) { // button pressed
			CC.left_button = GLUT_DOWN;
			CC.prevX = x;
			CC.prevY = y;

			camera[target_cam].move_status = 1;
		}
		else if (state == GLUT_UP) { // button released
			CC.left_button = GLUT_UP;
			camera[MAIN_CAM].move_status = camera[CCTV_DYN].move_status = 0;
		}
		break;
	case GLUT_RIGHT_BUTTON: // right click
		if (state == GLUT_DOWN) { // button pressed
			// SHIFT pressed -> to GOURAUD SHADING
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
				if (shader_selected == PHONG) {
					switch_shader_to(GOURAUD);
					fprintf(stdout, "^^^ Switched to Gouraud Shading.\n");
					glutPostRedisplay();
				}
			}
			// SHIFT released -> to PHONG SHADING
			else if (shader_selected == GOURAUD) {
				switch_shader_to(PHONG);
				fprintf(stdout, "^^^ Switched to Phong Shading.\n");
				glutPostRedisplay();
			}

			CC.right_button = GLUT_DOWN;
			CC.prevX = x;
			CC.prevY = y;

			camera[target_cam].move_status = 1;
		}
		else if (state == GLUT_UP) { // button released
			// RIGHT CLICK released -> to PHONG SHADING
			if (shader_selected == GOURAUD) {
				switch_shader_to(PHONG);
				fprintf(stdout, "^^^ Switched to Phong Shading.\n");
				glutPostRedisplay();
			}

			CC.right_button = GLUT_UP;
			camera[MAIN_CAM].move_status = camera[CCTV_DYN].move_status = 0;
		}
		break;
	case SCROLL_UP: // mouse wheel scroll up
		if (camera[target_cam].fov_y - 1.0f > 3.0f) {
			camera[target_cam].fov_y -= 1.0f;
			ProjectionMatrix[target_cam] = glm::perspective(camera[target_cam].fov_y*TO_RADIAN, camera[target_cam].aspect_ratio, camera[target_cam].near_clip, camera[target_cam].far_clip);
			ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
			glutPostRedisplay();
		}
		break;
	case SCROLL_DOWN: // mouse wheel scroll down
		if (camera[target_cam].fov_y + 1.0f < 100.0f) {
			camera[target_cam].fov_y += 1.0f;
			ProjectionMatrix[target_cam] = glm::perspective(camera[target_cam].fov_y*TO_RADIAN, camera[target_cam].aspect_ratio, camera[target_cam].near_clip, camera[target_cam].far_clip);
			ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
			glutPostRedisplay();
		}
		break;
	}
}

void motion(int x, int y) {
	float dispX, dispY;
	int target_cam = (ViewMode == EXTERIOR_MODE ? MAIN_CAM : CCTV_DYN);

	if (!(camera[MAIN_CAM].move_status || camera[CCTV_DYN].move_status)) return;

	dispX = (float)(x - CC.prevX);
	dispY = (float)(CC.prevY - y);
	CC.prevX = x;
	CC.prevY = y;

	if(CC.left_button == GLUT_DOWN) { // left click
		camera_rotate(target_cam, dispX, -camera[target_cam].vaxis);
		camera_rotate(target_cam, dispY, -camera[target_cam].uaxis);
	}
	else { // right click
		camera_rotate(target_cam, dispX, -camera[target_cam].naxis);
	}

	set_ViewMatrix(target_cam);
	ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 1, polygon_fill_on = 1, depth_test_on = 0;
	int target_cam = (ViewMode == EXTERIOR_MODE ? MAIN_CAM : CCTV_DYN);

	if (glutGetModifiers() != GLUT_ACTIVE_SHIFT && shader_selected == GOURAUD) {
		switch_shader_to(PHONG);
		fprintf(stdout, "^^^ Switched to Phong Shading.\n");
		glutPostRedisplay();
	}

	if (key == '5') {
		switch_shader_to(1 - shader_selected);
		fprintf(stdout, "^^^ Switched to %s Shading.\n", shader_selected ? "GOURAUD" : "PHONG");
		glutPostRedisplay();
		return;
	}

	if ((key >= '1') && (key <= '1' + NUMBER_OF_LIGHT_SUPPORTED - 1)) {
		int light_ID = (int)(key - '1');

		fprintf(stdout, "^^^ Turned %s light %d.\n", light[light_ID].light_on ? "off" : "on", light_ID);

		glUseProgram(*shader_program);
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);

		glutPostRedisplay();
		return;
	}

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case '7': // face cull toggle
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;
		}
		break;
	case '8': // draw/fill toggle
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;
	case '9': // depth test toggle
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;
	case 'i': // interior view mode toggle
	case 'I':
		if(ViewMode != INTERIOR_MODE)
			fprintf(stdout, "^^^ Switched to interior view.\n");
		ViewMode = INTERIOR_MODE;
		glutPostRedisplay();
		break;
	case 'o': // exterior view mode toggle
	case 'O':
		if(ViewMode != EXTERIOR_MODE)
			fprintf(stdout, "^^^ Switched to exterior view.\n");
		ViewMode = EXTERIOR_MODE;
		glutPostRedisplay();
		break;
	case 'w': // front & up
	case 'W':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) // translate frontwards
				camera_translate(MAIN_CAM, 130.0f, -camera[MAIN_CAM].naxis);
			else if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate frontwards
				camera_rotate(MAIN_CAM, 5.0f, camera[MAIN_CAM].uaxis);
			else // translate upwards
				camera_translate(MAIN_CAM, 100.0f, camera[MAIN_CAM].vaxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, camera[CCTV_DYN].uaxis); // rotate frontwards
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 'a': // left
	case 'A':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate leftwards
				camera_rotate(MAIN_CAM, 5.0f, camera[MAIN_CAM].vaxis);
			else // translate leftwards
				camera_translate(MAIN_CAM, 100.0f, -camera[MAIN_CAM].uaxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, camera[CCTV_DYN].vaxis); // rotate leftwards
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 's': // back & down
	case 'S':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) // translate backwards
				camera_translate(MAIN_CAM, 130.0f, camera[MAIN_CAM].naxis);
			else if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate backwards
				camera_rotate(MAIN_CAM, 5.0f, -camera[MAIN_CAM].uaxis);
			else // translate downwards
				camera_translate(MAIN_CAM, 100.0f, -camera[MAIN_CAM].vaxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, -camera[CCTV_DYN].uaxis); // rotate backwards
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 'd': // right
	case 'D':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate rightwards
				camera_rotate(MAIN_CAM, 5.0f, -camera[MAIN_CAM].vaxis);
			else // translate rightwards
				camera_translate(MAIN_CAM, 100.0f, camera[MAIN_CAM].uaxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, -camera[CCTV_DYN].vaxis); // rotate rightwards
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 'q': // counterclockwise
	case 'Q':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate counterclockwise
				camera_rotate(MAIN_CAM, 5.0f, -camera[MAIN_CAM].naxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, -camera[CCTV_DYN].naxis); // rotate counterclockwise
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 'e': // clockwise
	case 'E':
		if (ViewMode == EXTERIOR_MODE) {
			if (glutGetModifiers() == GLUT_ACTIVE_ALT) // rotate clockwise
				camera_rotate(MAIN_CAM, 5.0f, camera[MAIN_CAM].naxis);
		}
		else { // INTERIOR_MODE
			camera_rotate(CCTV_DYN, 5.0f, camera[CCTV_DYN].naxis); // rotate clockwise
		}
		set_ViewMatrix(target_cam);
		ViewProjectionMatrix[target_cam] = ProjectionMatrix[target_cam] * ViewMatrix[target_cam];
		glutPostRedisplay();
		break;
	case 'p':
	case 'P':
		pause_animation = (pause_animation + 1) % 2;
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;
	aspect_ratio = (float)width / height;

	// EXTERIOR_MODE : MAIN_CAM, SIDE_CAM, FRONT_CAM, TOP_CAM

	viewport[MAIN_CAM].x = width * 0.4f;
	viewport[MAIN_CAM].y = 0.0f;
	viewport[MAIN_CAM].w = width * 0.6f;
	viewport[MAIN_CAM].h = height;

	viewport[SIDE_CAM].x = 0.0f;
	viewport[SIDE_CAM].y = height * 0.75f;
	viewport[SIDE_CAM].w = width * 0.4f * 0.7f;
	viewport[SIDE_CAM].h = height * 0.25f;

	viewport[FRONT_CAM].x = 0.0f;
	viewport[FRONT_CAM].y = height * 0.5f;
	viewport[FRONT_CAM].w = width * 0.4f;
	viewport[FRONT_CAM].h = height * 0.25f;

	viewport[TOP_CAM].x = 0.0f;
	viewport[TOP_CAM].y = 0.0f;
	viewport[TOP_CAM].w = width * 0.4f;
	viewport[TOP_CAM].h = height * 0.5f;

	// INTERIOR_MODE : CCTV_DYN, CCTV_1, CCTV_2, CCTV_3

	viewport[CCTV_DYN].x = width * 0.4f;
	viewport[CCTV_DYN].y = 0.0f;
	viewport[CCTV_DYN].w = width * 0.6f;
	viewport[CCTV_DYN].h = height;

	viewport[CCTV_1].x = 0.0f;
	viewport[CCTV_1].y = height * 0.67f;
	viewport[CCTV_1].w = width * 0.35f;
	viewport[CCTV_1].h = height * 0.28f;

	viewport[CCTV_2].x = 0.0f;
	viewport[CCTV_2].y = height * 0.36f;
	viewport[CCTV_2].w = width * 0.35f;
	viewport[CCTV_2].h = height * 0.28f;

	viewport[CCTV_3].x = 0.0f;
	viewport[CCTV_3].y = height * 0.05f;
	viewport[CCTV_3].w = width * 0.35f;
	viewport[CCTV_3].h = height * 0.28f;

	for (int i = 0; i < NUMBER_OF_CAMERAS; i++) {
		camera[i].aspect_ratio = (float)viewport[i].w / viewport[i].h;
		ProjectionMatrix[i] = glm::perspective(camera[i].fov_y*TO_RADIAN, camera[i].aspect_ratio, camera[i].near_clip, camera[i].far_clip);
		ViewProjectionMatrix[i] = ProjectionMatrix[i] * ViewMatrix[i];
	}

	// orthographic projection for side, front, top view cameras
	ProjectionMatrix[SIDE_CAM] = glm::ortho(-camera[SIDE_CAM].pos.y, camera[SIDE_CAM].pos.y, -camera[SIDE_CAM].pos.z - 10.0f, camera[SIDE_CAM].pos.z + 10.0f, camera[SIDE_CAM].near_clip, camera[SIDE_CAM].far_clip);
	ViewProjectionMatrix[SIDE_CAM] = ProjectionMatrix[SIDE_CAM] * ViewMatrix[SIDE_CAM];

	ProjectionMatrix[FRONT_CAM] = glm::ortho(-camera[FRONT_CAM].pos.x, camera[FRONT_CAM].pos.x, -camera[FRONT_CAM].pos.z - 10.0f, camera[FRONT_CAM].pos.z + 10.0f, camera[FRONT_CAM].near_clip, camera[FRONT_CAM].far_clip);
	ViewProjectionMatrix[FRONT_CAM] = ProjectionMatrix[FRONT_CAM] * ViewMatrix[FRONT_CAM];

	ProjectionMatrix[TOP_CAM] = glm::ortho(-camera[TOP_CAM].pos.x, camera[TOP_CAM].pos.x, -camera[TOP_CAM].pos.y, camera[TOP_CAM].pos.y, camera[TOP_CAM].near_clip, camera[TOP_CAM].far_clip);
	ViewProjectionMatrix[TOP_CAM] = ProjectionMatrix[TOP_CAM] * ViewMatrix[TOP_CAM];

	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	glm::vec3 prevPos = tiger_data.pos;
	glm::vec4 afterRotation;
	TIGER_PATH_POINT pnt;

	if (!tiger_path_queue.empty())
		pnt = tiger_path_queue.front();

	if (!pause_animation) {
		if (tiger_inRotation < 0.0f) { // not in rotation
			tiger_data.pos += tiger_data.headTo;
			tiger_data.headTo = tiger_data.pos - prevPos;
		}
		else { // in rotation
			tiger_inRotation -= 5.0f;
			if (tiger_inRotation < 0.0f) { // end of rotation
				afterRotation = TigerModellingMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

				if (pnt.angle == 90.0f) {
					tiger_data.headTo = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), tiger_data.headTo);
					tiger_data.headTo *= (pnt.direction == COUNTERCLOCKWISE ? 1 : -1);
				}
				else if (pnt.angle == 180.0f)
					tiger_data.headTo *= -1;
				else if (pnt.angle == 270.0f) {
					tiger_data.headTo = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), tiger_data.headTo);
					tiger_data.headTo *= (pnt.direction == CLOCKWISE ? 1 : -1);
				}

				tiger_data.pos.x = afterRotation.x;
				tiger_data.pos.y = afterRotation.y;

				tiger_path_queue.push(pnt);
				tiger_path_queue.pop();
				if (!tiger_path_queue.empty())
					pnt = tiger_path_queue.front();
			}
		}

		if (!tiger_path_queue.empty() && tiger_inRotation < 0.0f) {
			if (pnt.trigger == X && tiger_data.pos.x == pnt.center.x)
				tiger_inRotation = pnt.angle;
			else if (pnt.trigger == Y && tiger_data.pos.y == pnt.center.y)
				tiger_inRotation = pnt.angle;
		}
	}

	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	car_rotation_angle = timestamp_scene % 360;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + (pause_animation ? 0 : 1)) % INT_MAX);
}

void register_callbacks(void) {
	CC.left_button = CC.right_button = GLUT_UP;

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup_OpenGL_stuffs);
}

void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrixInvTrans");

	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");

	/*
	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");
	*/
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;
	glUseProgram(*shader_program);

	glUniform4f(loc_global_ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 10) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);
}

void initialize_camera(void) {
	glm::mat4 temp;

	ViewMode = EXTERIOR_MODE;
	car_rotation_angle = 0.0f;
	pause_animation = false;

	// MAIN CAMERA
	temp = glm::lookAt(glm::vec3(300.0f, 300.0f, 100.0f), glm::vec3(125.0f, 80.0f, 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera[MAIN_CAM].pos = glm::vec3(300.0f, 300.0f, 100.0f);
	camera[MAIN_CAM].uaxis = glm::vec3(temp[0].x, temp[1].x, temp[2].x); // right
	camera[MAIN_CAM].vaxis = glm::vec3(temp[0].y, temp[1].y, temp[2].y); // up
	camera[MAIN_CAM].naxis = glm::vec3(temp[0].z, temp[1].z, temp[2].z); // back

	camera[MAIN_CAM].fov_y = 60.0f;
	camera[MAIN_CAM].near_clip = 7.0f;
	camera[MAIN_CAM].far_clip = 500.0f;

	set_ViewMatrix(MAIN_CAM);

	// FRONT CAMERA
	camera[FRONT_CAM].pos = glm::vec3(120.0f, -1500.0f, 25.0f);
	camera[FRONT_CAM].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[FRONT_CAM].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[FRONT_CAM].naxis = glm::vec3(0.0f, -1.0f, 0.0f);

	camera[FRONT_CAM].fov_y = 15.0f;
	camera[FRONT_CAM].near_clip = 1.0f;
	camera[FRONT_CAM].far_clip = 3000.0f;

	set_ViewMatrix(FRONT_CAM);

	// SIDE CAMERA
	//temp = glm::lookAt(glm::vec3(800.0f, 85.0f, 25.0f), glm::vec3(0.0f, 90.0f, 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera[SIDE_CAM].pos = glm::vec3(2900.0f, 85.0f, 25.0f);
	camera[SIDE_CAM].uaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	camera[SIDE_CAM].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[SIDE_CAM].naxis = glm::vec3(1.0f, 0.0f, 0.0f);

	camera[SIDE_CAM].fov_y = 25.0f;
	camera[SIDE_CAM].near_clip = 1.0f;
	camera[SIDE_CAM].far_clip = 3000.0f;

	set_ViewMatrix(SIDE_CAM);

	// TOP CAMERA
	//temp = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f), glm::vec3(-10.0f, 0.0f, 0.0f));
	camera[TOP_CAM].pos = glm::vec3(120.0f, 85.0f, 1100.0f);
	camera[TOP_CAM].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[TOP_CAM].vaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	camera[TOP_CAM].naxis = glm::vec3(0.0f, 0.0f, 1.0f);

	camera[TOP_CAM].fov_y = 15.0f;
	camera[TOP_CAM].near_clip = 1.0f;
	camera[TOP_CAM].far_clip = 3000.0f;

	set_ViewMatrix(TOP_CAM);

	// CCTV 1
	camera[CCTV_1].pos = glm::vec3(224.0f, 105.0f, 25.0f);
	camera[CCTV_1].uaxis = glm::vec3(1.0f, 1.0f, 0.0f);
	camera[CCTV_1].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[CCTV_1].naxis = glm::vec3(1.0f, -1.0f, 0.0f);

	camera[CCTV_1].fov_y = 60.0f;
	camera[CCTV_1].near_clip = 0.01f;
	camera[CCTV_1].far_clip = 500.0f;

	set_ViewMatrix(CCTV_1);

	// CCTV 2
	camera[CCTV_2].pos = glm::vec3(115.0f, 80.0f, 25.0f);
	camera[CCTV_2].uaxis = glm::vec3(0.0f, -1.0f, 0.0f);
	camera[CCTV_2].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[CCTV_2].naxis = glm::vec3(-1.0f, 0.0f, 0.0f);

	camera[CCTV_2].fov_y = 60.0f;
	camera[CCTV_2].near_clip = 0.01f;
	camera[CCTV_2].far_clip = 500.0f;

	set_ViewMatrix(CCTV_2);

	// CCTV 3
	camera[CCTV_3].pos = glm::vec3(70.0f, 17.0f, 25.0f);
	camera[CCTV_3].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[CCTV_3].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[CCTV_3].naxis = glm::vec3(0.0f, -1.0f, 0.0f);

	camera[CCTV_3].fov_y = 60.0f;
	camera[CCTV_3].near_clip = 0.01f;
	camera[CCTV_3].far_clip = 500.0f;

	set_ViewMatrix(CCTV_3);

	// DYNAMIC CCTV
	camera[CCTV_DYN].pos = glm::vec3(210.0f, 41.0f, 25.0f);
	camera[CCTV_DYN].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[CCTV_DYN].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[CCTV_DYN].naxis = glm::vec3(0.0f, -1.0f, 0.0f);

	camera[CCTV_DYN].fov_y = 60.0f;
	camera[CCTV_DYN].near_clip = 0.01f;
	camera[CCTV_DYN].far_clip = 500.0f;

	set_ViewMatrix(CCTV_DYN);

	// set move status to false
	for (int i = 0; i < NUMBER_OF_CAMERAS; i++) {
		camera[i].move_status = 0;
		camera[i].aspect_ratio = 1.0f;
	}
}

void initialize_OpenGL(void) {
	glDisable(GL_DEPTH_TEST); //glEnable(GL_DEPTH_TEST); // Default state
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(0.12f, 0.18f, 0.12f, 1.0f);

	//switch_shader_to(PHONG);

	initialize_camera();
	//initialize_lights_and_material();
}

void set_up_scene_lights(void) {
	// spot_light_WC: use light 0
	light[0].light_on = 1;

	light[0].position[0] = 0.0f;
	light[0].position[1] = 0.0f;
	light[0].position[2] = 10.0f;
	light[0].position[3] = 1.0f;

	light[0].ambient_color[0] = 1.0f;
	light[0].ambient_color[1] = 1.0f;
	light[0].ambient_color[2] = 1.0f;
	light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = 1.0f;
	light[0].diffuse_color[1] = 1.0f;
	light[0].diffuse_color[2] = 1.0f;
	light[0].diffuse_color[3] = 1.0f;

	light[0].specular_color[0] = 1.0f;
	light[0].specular_color[1] = 1.0f;
	light[0].specular_color[2] = 1.0f;
	light[0].specular_color[3] = 1.0f;

	glUseProgram(*shader_program);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	//glm::vec4 position_EC = ViewMatrix[MAIN_CAM] * glm::vec4(light[0].position[0], light[0].position[1],
		//light[0].position[2], light[0].position[3]);
	//glUniform4fv(loc_light[0].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUseProgram(0);


	/*
	light[0].spot_direction[0] = 0.0f;
	light[0].spot_direction[1] = 0.0f; // spot light direction in WC
	light[0].spot_direction[2] = -1.0f;
	light[0].spot_cutoff_angle = 50.0f;
	light[0].spot_exponent = 7.0f;
	*/

	/*
	glm::vec4 position_EC = ViewMatrix[MAIN_CAM] * glm::vec4(light[0].position[0], light[0].position[1],
		light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light[0].position, 1, &position_EC[0]);

	/*
	glm::vec3 direction_EC = glm::mat3(ViewMatrix[MAIN_CAM]) * glm::vec3(light[0].spot_direction[0], light[0].spot_direction[1],
		light[0].spot_direction[2]);

	glUniform3fv(loc_light[0].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[0].spot_cutoff_angle, light[0].spot_cutoff_angle);
	glUniform1f(loc_light[0].spot_exponent, light[0].spot_exponent);
	
	/*
	glUseProgram(h_ShaderProgram_PS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glm::vec4 position_EC = ViewMatrix[MAIN_CAM] * glm::vec4(light[0].position[0], light[0].position[1],
		light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light[0].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	
	glm::vec3 direction_EC = glm::mat3(ViewMatrix[MAIN_CAM]) * glm::vec3(light[0].spot_direction[0], light[0].spot_direction[1],
		light[0].spot_direction[2]);
	
	glUniform3fv(loc_light[0].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[0].spot_cutoff_angle, light[0].spot_cutoff_angle);
	glUniform1f(loc_light[0].spot_exponent, light[0].spot_exponent);
	*/	
}

void prepare_scene(void) {
	char car_body[] = "Data/car_body_triangles_v.txt";
	char car_wheel[] = "Data/car_wheel_triangles_v.txt";
	char car_nut[] = "Data/car_nut_triangles_v.txt";
	
	define_axes();
	define_static_objects();
	define_animated_tiger();
	define_camera();
	define_line();
	prepare_hier_obj(HIER_OBJ_CAR_BODY, car_body, HIER_OBJ_TYPE_V);
	prepare_hier_obj(HIER_OBJ_CAR_WHEEL, car_wheel, HIER_OBJ_TYPE_V);
	prepare_hier_obj(HIER_OBJ_CAR_NUT, car_nut, HIER_OBJ_TYPE_V);

	switch_shader_to(PHONG);
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
