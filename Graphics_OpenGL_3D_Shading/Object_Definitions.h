
// The object modelling tasks performed by this file are usually done 
// by reading a scene configuration file or through a help of graphics user interface!!!

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

typedef struct _material {
	glm::vec4 emission, ambient, diffuse, specular;
	GLfloat exponent;
} Material;

#define N_MAX_GEOM_COPIES 5
typedef struct _Object {
	char filename[512];

	GLenum front_face_mode; // clockwise or counter-clockwise
	int n_triangles;

	int n_fields; // 3 floats for vertex, 3 floats for normal, and 2 floats for texcoord
	GLfloat *vertices; // pointer to vertex array data
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax; // bounding box <- compute this yourself

	GLuint VBO, VAO; // Handles to vertex buffer object and vertex array object

	int n_geom_instances;
	glm::mat4 ModelMatrix[N_MAX_GEOM_COPIES];
	Material material[N_MAX_GEOM_COPIES];
} Object;


// STATIC OBJECTS
#define N_MAX_STATIC_OBJECTS		13
Object static_objects[N_MAX_STATIC_OBJECTS]; // allocage memory dynamically every time it is needed rather than using a static array
int n_static_objects = 0;

#define OBJ_BUILDING		0
#define OBJ_TABLE			1
#define OBJ_LIGHT			2
#define OBJ_TEAPOT			3
#define OBJ_NEW_CHAIR		4
#define OBJ_FRAME			5
#define OBJ_NEW_PICTURE		6
#define OBJ_COW				7
#define OBJ_IRONMAN			8
#define OBJ_TANK			9
#define OBJ_GODZILLA		10
#define OBJ_BUS				11
#define OBJ_BIKE			12

int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open the object file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Error: cannot allocate memory for the geometry file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(*object, bytes_per_primitive, n_triangles, fp); // assume the data file has no faults.
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void compute_AABB(Object *obj_ptr) {
	// Do it yourself.
}
	 
void prepare_geom_of_static_object(Object *obj_ptr) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = obj_ptr->n_fields * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	obj_ptr->n_triangles = read_geometry(&(obj_ptr->vertices), n_bytes_per_triangle, obj_ptr->filename);

	// Initialize vertex buffer object.
	glGenBuffers(1, &(obj_ptr->VBO));

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glBufferData(GL_ARRAY_BUFFER, obj_ptr->n_triangles*n_bytes_per_triangle, obj_ptr->vertices, GL_STATIC_DRAW);

	compute_AABB(obj_ptr);
	free(obj_ptr->vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &(obj_ptr->VAO));
	glBindVertexArray(obj_ptr->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void define_static_objects(void) {
	// building
	strcpy(static_objects[OBJ_BUILDING].filename, "Data/Building1_vnt.geom");
	static_objects[OBJ_BUILDING].n_fields = 8;

	static_objects[OBJ_BUILDING].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BUILDING]));

	static_objects[OBJ_BUILDING].n_geom_instances = 1;

    static_objects[OBJ_BUILDING].ModelMatrix[0] = glm::mat4(1.0f);
	
	static_objects[OBJ_BUILDING].material[0].emission = glm::vec4(0.1f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].ambient = glm::vec4(0.1f, 0.0f / 225.0f, 0.0f / 255.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].diffuse = glm::vec4(100.0f / 255.0f, 50.0f / 255.0f, 10.0f / 255.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].specular = glm::vec4(0.1f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].exponent = 25.5f;

	// table
	strcpy(static_objects[OBJ_TABLE].filename, "Data/Table_vn.geom");
	static_objects[OBJ_TABLE].n_fields = 6;

	static_objects[OBJ_TABLE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TABLE]));

	static_objects[OBJ_TABLE].n_geom_instances = 2;

	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(157.0f, 76.5f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[0], 
		glm::vec3(0.5f, 0.5f, 0.5f));

	static_objects[OBJ_TABLE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[0].ambient = glm::vec4(0.1f, 0.3f, 0.1f, 1.0f);
	static_objects[OBJ_TABLE].material[0].diffuse = glm::vec4(0.4f, 0.6f, 0.3f, 1.0f);
	static_objects[OBJ_TABLE].material[0].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[0].exponent = 15.0f;

	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(198.0f, 120.0f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[1],
		glm::vec3(0.8f, 0.6f, 0.6f));

	static_objects[OBJ_TABLE].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	static_objects[OBJ_TABLE].material[1].diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	static_objects[OBJ_TABLE].material[1].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[1].exponent = 128.0f*0.078125f;

	// Light
	strcpy(static_objects[OBJ_LIGHT].filename, "Data/Light_vn.geom");
	static_objects[OBJ_LIGHT].n_fields = 6;

	static_objects[OBJ_LIGHT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(static_objects + OBJ_LIGHT);

	static_objects[OBJ_LIGHT].n_geom_instances = 5;

	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 100.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 47.5f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 130.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[2],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, 60.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[3],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[3].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 112.5f, 49.0));
	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[4],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[4].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].exponent = 128.0f*0.4f;

	// teapot
	strcpy(static_objects[OBJ_TEAPOT].filename, "Data/Teapotn_vn.geom");
	static_objects[OBJ_TEAPOT].n_fields = 6;

	static_objects[OBJ_TEAPOT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TEAPOT]));

	static_objects[OBJ_TEAPOT].n_geom_instances = 3;

	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].exponent = 128.0f*0.6;

	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 40.0f, 0.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[1],
		glm::vec3(2.2f, 2.5f, 2.0f));

	static_objects[OBJ_TEAPOT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].diffuse = glm::vec4(0.04136f, 0.61424f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].exponent = 128.0f*0.6;

	static_objects[OBJ_TEAPOT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 30.0f, 0.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[2] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[2],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].diffuse = glm::vec4(0.04136f, 0.04136f, 0.61424f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].exponent = 128.0f*0.6;

	// new_chair
	strcpy(static_objects[OBJ_NEW_CHAIR].filename, "Data/new_chair_vnt.geom");
	static_objects[OBJ_NEW_CHAIR].n_fields = 8;

	static_objects[OBJ_NEW_CHAIR].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_CHAIR]));

	static_objects[OBJ_NEW_CHAIR].n_geom_instances = 1;

	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(215.0f, 120.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		-90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].exponent = 128.0f*0.078125f;

	// frame
	strcpy(static_objects[OBJ_FRAME].filename, "Data/Frame_vn.geom");
	static_objects[OBJ_FRAME].n_fields = 6;

	static_objects[OBJ_FRAME].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_FRAME]));

	static_objects[OBJ_FRAME].n_geom_instances = 1;

	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 116.0f, 30.0f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::scale(static_objects[OBJ_FRAME].ModelMatrix[0],
		glm::vec3(0.6f, 0.6f, 0.6f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::rotate(static_objects[OBJ_FRAME].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_FRAME].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_FRAME].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_FRAME].material[0].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_FRAME].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_FRAME].material[0].exponent = 128.0f*0.1f;


	// new_picture
	strcpy(static_objects[OBJ_NEW_PICTURE].filename, "Data/new_picture_vnt.geom");
	static_objects[OBJ_NEW_PICTURE].n_fields = 8;

	static_objects[OBJ_NEW_PICTURE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_PICTURE]));

	static_objects[OBJ_NEW_PICTURE].n_geom_instances = 1;

	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(189.5f, 116.0f, 30.0f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		glm::vec3(13.5f*0.6f, 13.5f*0.6f, 13.5f*0.6f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_NEW_PICTURE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].exponent = 128.0f*0.6f;

	// COW
	strcpy(static_objects[OBJ_COW].filename, "Data/cow_vn.geom");
	static_objects[OBJ_COW].n_fields = 6;

	static_objects[OBJ_COW].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_COW]));

	static_objects[OBJ_COW].n_geom_instances = 1;

	static_objects[OBJ_COW].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(145.0f, 33.0f, 9.5f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::scale(static_objects[OBJ_COW].ModelMatrix[0],
		glm::vec3(30.0f, 30.0f, 30.0f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
 
	static_objects[OBJ_COW].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[0].exponent = 0.21794872f*0.6f;

	// IRONMAN
	strcpy(static_objects[OBJ_IRONMAN].filename, "Data/IronMan.geom");
	static_objects[OBJ_IRONMAN].n_fields = 8;

	static_objects[OBJ_IRONMAN].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_IRONMAN]));

	static_objects[OBJ_IRONMAN].n_geom_instances = 2;

	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(90.0f, 115.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[0], -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::scale(static_objects[OBJ_IRONMAN].ModelMatrix[0], glm::vec3(8.0f, 8.0f, 8.0f));

	static_objects[OBJ_IRONMAN].material[0].emission = glm::vec4(25.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].ambient = glm::vec4(40.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].diffuse = glm::vec4(170.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].specular = glm::vec4(190.0f / 255.0f, 50.0f / 255.0f, 40.0 / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].exponent = 50.0f;

	static_objects[OBJ_IRONMAN].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 115.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[1] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[1], -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[1] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[1], 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[1] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[1], 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[1] = glm::scale(static_objects[OBJ_IRONMAN].ModelMatrix[1], glm::vec3(8.0f, 8.0f, 8.0f));

	static_objects[OBJ_IRONMAN].material[1].emission = glm::vec4(100.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[1].ambient = glm::vec4(0.3f, 0.15f, 0.1f, 1.0f);
	static_objects[OBJ_IRONMAN].material[1].diffuse = glm::vec4(207.0f / 255.0f, 164.0f / 255.0f, 119.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[1].specular = glm::vec4(215.0f / 255.0f, 188.0f / 255.0f, 160.0f / 255.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[1].exponent = 128.0f*0.1f;

	// TANK
	strcpy(static_objects[OBJ_TANK].filename, "Data/Tank.geom");
	static_objects[OBJ_TANK].n_fields = 8;

	static_objects[OBJ_TANK].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TANK]));

	static_objects[OBJ_TANK].n_geom_instances = 2;

	static_objects[OBJ_TANK].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(90.0f, 140.0f, 0.0f));
	static_objects[OBJ_TANK].ModelMatrix[0] = glm::rotate(static_objects[OBJ_TANK].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_TANK].ModelMatrix[0] = glm::scale(static_objects[OBJ_TANK].ModelMatrix[0], glm::vec3(4.0f, 4.0f, 4.0f));
	static_objects[OBJ_TANK].ModelMatrix[0] = glm::translate(static_objects[OBJ_TANK].ModelMatrix[0], glm::vec3(0.0f, -15.0f, 0.0f));

	static_objects[OBJ_TANK].material[0].emission = glm::vec4(178 / 255.0f, 94 / 255.0f, 218 / 255.0f, 0.63f / 255.0f);
	static_objects[OBJ_TANK].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_TANK].material[0].diffuse = glm::vec4(178 / 255.0f, 94 / 255.0f, 218 / 255.0f, 0.63f / 255.0f);
	static_objects[OBJ_TANK].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_TANK].material[0].exponent = 128.0f*0.1f;

	static_objects[OBJ_TANK].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 148.0f, 0.0f));
	static_objects[OBJ_TANK].ModelMatrix[1] = glm::rotate(static_objects[OBJ_TANK].ModelMatrix[1], -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_TANK].ModelMatrix[1] = glm::scale(static_objects[OBJ_TANK].ModelMatrix[1], glm::vec3(4.0f, 4.0f, 4.0f));
	static_objects[OBJ_TANK].ModelMatrix[1] = glm::translate(static_objects[OBJ_TANK].ModelMatrix[1], glm::vec3(0.0f, -15.0f, 0.0f));

	static_objects[OBJ_TANK].material[1].emission = glm::vec4(94 / 255.0f, 218 / 255.0f, 184 / 255.0f, 0.63 / 255.0f);
	static_objects[OBJ_TANK].material[1].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_TANK].material[1].diffuse = glm::vec4(94 / 255.0f, 218 / 255.0f, 184 / 255.0f, 0.63 / 255.0f);
	static_objects[OBJ_TANK].material[1].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_TANK].material[1].exponent = 128.0f*0.1f;

	// GODZILLA
	strcpy(static_objects[OBJ_GODZILLA].filename, "Data/Godzilla.geom");
	static_objects[OBJ_GODZILLA].n_fields = 8;

	static_objects[OBJ_GODZILLA].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_GODZILLA]));

	static_objects[OBJ_GODZILLA].n_geom_instances = 1;

	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 65.0f, 0.0f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::scale(static_objects[OBJ_GODZILLA].ModelMatrix[0], glm::vec3(0.15f, 0.15f, 0.25f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::rotate(static_objects[OBJ_GODZILLA].ModelMatrix[0], 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::rotate(static_objects[OBJ_GODZILLA].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_GODZILLA].material[0].emission = glm::vec4(218 / 255.0f, 199 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_GODZILLA].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].diffuse = glm::vec4(218 / 255.0f, 199 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_GODZILLA].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].exponent = 128.0f*0.1f;

	// BUS
	strcpy(static_objects[OBJ_BUS].filename, "Data/Bus.geom");
	static_objects[OBJ_BUS].n_fields = 8;

	static_objects[OBJ_BUS].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BUS]));

	static_objects[OBJ_BUS].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 135.0f, 0.0f));
	static_objects[OBJ_BUS].ModelMatrix[0] = glm::scale(static_objects[OBJ_BUS].ModelMatrix[0], glm::vec3(1.2f, 1.2f, 1.3f));
	static_objects[OBJ_BUS].ModelMatrix[0] = glm::rotate(static_objects[OBJ_BUS].ModelMatrix[0], 45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_BUS].ModelMatrix[0] = glm::rotate(static_objects[OBJ_BUS].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_BUS].n_geom_instances = 1;

	static_objects[OBJ_BUS].material[0].emission = glm::vec4(218 / 255.0f, 94 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_BUS].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_BUS].material[0].diffuse = glm::vec4(218 / 255.0f, 94 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_BUS].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_BUS].material[0].exponent = 128.0f*0.1f;

	// BIKE
	strcpy(static_objects[OBJ_BIKE].filename, "Data/Bike.geom");
	static_objects[OBJ_BIKE].n_fields = 8;

	static_objects[OBJ_BIKE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BIKE]));

	static_objects[OBJ_BIKE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 25.0f, 0.0f));
	static_objects[OBJ_BIKE].ModelMatrix[0] = glm::scale(static_objects[OBJ_BIKE].ModelMatrix[0], glm::vec3(10.0f, 10.0f, 10.0f));
	static_objects[OBJ_BIKE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_BIKE].ModelMatrix[0], 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_BIKE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_BIKE].ModelMatrix[0], -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_BIKE].n_geom_instances = 1;

	static_objects[OBJ_BIKE].material[0].emission = glm::vec4(156 / 255.0f, 218 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_BIKE].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_BIKE].material[0].diffuse = glm::vec4(156 / 255.0f, 218 / 255.0f, 94 / 255.0f, 0.72 / 255.0f);
	static_objects[OBJ_BIKE].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_BIKE].material[0].exponent = 128.0f*0.1f;

	n_static_objects = 13;
}

void set_material_static_object(Object obj, int instance) {
	Material_Parameters material_static_object;

	material_static_object.ambient_color[0] = obj.material[instance].ambient.r;
	material_static_object.ambient_color[1] = obj.material[instance].ambient.g;
	material_static_object.ambient_color[2] = obj.material[instance].ambient.b;
	material_static_object.ambient_color[3] = obj.material[instance].ambient.a;

	material_static_object.diffuse_color[0] = obj.material[instance].diffuse.r;
	material_static_object.diffuse_color[1] = obj.material[instance].diffuse.g;
	material_static_object.diffuse_color[2] = obj.material[instance].diffuse.b;
	material_static_object.diffuse_color[3] = obj.material[instance].diffuse.a;

	material_static_object.specular_color[0] = obj.material[instance].specular.r;
	material_static_object.specular_color[1] = obj.material[instance].specular.g;
	material_static_object.specular_color[2] = obj.material[instance].specular.b;
	material_static_object.specular_color[3] = obj.material[instance].specular.a;

	material_static_object.specular_exponent = obj.material[instance].exponent;

	material_static_object.emissive_color[0] = obj.material[instance].emission.r;
	material_static_object.emissive_color[1] = obj.material[instance].emission.g;
	material_static_object.emissive_color[2] = obj.material[instance].emission.b;
	material_static_object.emissive_color[3] = obj.material[instance].emission.a;

	// assume ShaderProgram_PS is used
	glUniform4fv(loc_material.ambient_color, 1, material_static_object.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_static_object.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_static_object.specular_color);
	glUniform1f(loc_material.specular_exponent, material_static_object.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_static_object.emissive_color);
}

void draw_static_object(Object *obj_ptr, int instance_ID, int camera_ID) {
	glFrontFace(obj_ptr->front_face_mode);

	set_material_static_object(*obj_ptr, instance_ID);

	ModelViewMatrix[camera_ID] = ViewMatrix[camera_ID] * obj_ptr->ModelMatrix[instance_ID];
	ModelViewProjectionMatrix = ProjectionMatrix[camera_ID] * ModelViewMatrix[camera_ID];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_ID]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_ID][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	//glUniform3f(loc_primitive_color, obj_ptr->material[instance_ID].diffuse.r,
		//obj_ptr->material[instance_ID].diffuse.g, obj_ptr->material[instance_ID].diffuse.b);
	//glFrontFace(GL_CCW);
	glBindVertexArray(obj_ptr->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * obj_ptr->n_triangles);
	glBindVertexArray(0);
}

// AXES
GLuint VBO_axes, VAO_axes;
GLfloat vertices_axes[6][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } };

void define_axes(void) {
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_axes), &vertices_axes[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

#define WC_AXIS_LENGTH		60.0f
void draw_axes(int camera_id) {
	ModelViewMatrix[camera_id] = glm::scale(ViewMatrix[camera_id], glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(VAO_axes);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

// HIERARCHICAL MODEL CAR
#define N_HIERARCHICAL_OBJECTS	3

#define HIER_OBJ_CAR_BODY		0
#define HIER_OBJ_CAR_WHEEL		1
#define HIER_OBJ_CAR_NUT		2

GLuint hier_obj_VBO[N_HIERARCHICAL_OBJECTS];
GLuint hier_obj_VAO[N_HIERARCHICAL_OBJECTS];

int hier_obj_n_triangles[N_HIERARCHICAL_OBJECTS];
GLfloat *hier_obj_vertices[N_HIERARCHICAL_OBJECTS];

Material_Parameters material_car[N_HIERARCHICAL_OBJECTS];

// codes for the 'general' triangular-mesh object
typedef enum _HIER_OBJ_TYPE { HIER_OBJ_TYPE_V = 0, HIER_OBJ_TYPE_VN, HIER_OBJ_TYPE_VNT } HIER_OBJ_TYPE;
// HIER_OBJ_TYPE_V: (x, y, z)
// HIER_OBJ_TYPE_VN: (x, y, z, nx, ny, nz)
// HIER_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int HIER_OBJ_ELEMENTS_PER_VERTEX[3] = { 3, 6, 8 };

int read_geometry_file(GLfloat **object, char *filename, HIER_OBJ_TYPE hier_obj_type) {
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}

	fscanf(fp, "%d", &n_triangles);
	*object = (float *)malloc(3 * n_triangles*HIER_OBJ_ELEMENTS_PER_VERTEX[hier_obj_type] * sizeof(float));
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * HIER_OBJ_ELEMENTS_PER_VERTEX[hier_obj_type]; i++)
		fscanf(fp, "%f", flt_ptr++);
	fclose(fp);

	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	return n_triangles;
}

void prepare_hier_obj(int hier_obj_ID, char *filename, HIER_OBJ_TYPE hier_obj_type) {
	int n_bytes_per_vertex;

	n_bytes_per_vertex = HIER_OBJ_ELEMENTS_PER_VERTEX[hier_obj_type] * sizeof(float);
	hier_obj_n_triangles[hier_obj_ID] = read_geometry_file(&hier_obj_vertices[hier_obj_ID], filename, hier_obj_type);

	// Initialize vertex array object.
	glGenVertexArrays(1, &hier_obj_VAO[hier_obj_ID]);
	glBindVertexArray(hier_obj_VAO[hier_obj_ID]);
	glGenBuffers(1, &hier_obj_VBO[hier_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, hier_obj_VBO[hier_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * hier_obj_n_triangles[hier_obj_ID] * n_bytes_per_vertex,
		hier_obj_vertices[hier_obj_ID], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	/*
	if (hier_obj_type >= HIER_OBJ_TYPE_VN) {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	if (hier_obj_type >= HIER_OBJ_TYPE_VNT) {
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	*/
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(hier_obj_vertices[hier_obj_ID]);

	if (!hier_obj_ID) {
		// material for CAR_BODY
		material_car[HIER_OBJ_CAR_BODY].ambient_color[0] = 50 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].ambient_color[1] = 80 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].ambient_color[2] = 10 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].ambient_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_BODY].diffuse_color[0] = 134 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].diffuse_color[1] = 211 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].diffuse_color[2] = 39 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].diffuse_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_BODY].specular_color[0] = 190 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].specular_color[1] = 240 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].specular_color[2] = 10 / 255.0f;
		material_car[HIER_OBJ_CAR_BODY].specular_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_BODY].specular_exponent = 3.0f;

		material_car[HIER_OBJ_CAR_BODY].emissive_color[0] = 0.0f;
		material_car[HIER_OBJ_CAR_BODY].emissive_color[1] = 0.1f;
		material_car[HIER_OBJ_CAR_BODY].emissive_color[2] = 0.0f;
		material_car[HIER_OBJ_CAR_BODY].emissive_color[3] = 1.0f;


		// material for CAR_WHEEL
		material_car[HIER_OBJ_CAR_WHEEL].ambient_color[0] = 150 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].ambient_color[1] = 100 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].ambient_color[2] = 12 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].ambient_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_WHEEL].diffuse_color[0] = 211 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].diffuse_color[1] = 156 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].diffuse_color[2] = 39 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].diffuse_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_WHEEL].specular_color[0] = 240 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].specular_color[1] = 200 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].specular_color[2] = 90 / 255.0f;
		material_car[HIER_OBJ_CAR_WHEEL].specular_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_WHEEL].specular_exponent = 3.0f;

		material_car[HIER_OBJ_CAR_WHEEL].emissive_color[0] = 0.3f;
		material_car[HIER_OBJ_CAR_WHEEL].emissive_color[1] = 0.0f;
		material_car[HIER_OBJ_CAR_WHEEL].emissive_color[2] = 0.0f;
		material_car[HIER_OBJ_CAR_WHEEL].emissive_color[3] = 1.0f;


		// material for CAR_NUT
		material_car[HIER_OBJ_CAR_NUT].ambient_color[0] = 120 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].ambient_color[1] = 40 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].ambient_color[2] = 40 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].ambient_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_NUT].diffuse_color[0] = 215 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].diffuse_color[1] = 46 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].diffuse_color[2] = 42 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].diffuse_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_NUT].specular_color[0] = 230 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].specular_color[1] = 90 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].specular_color[2] = 80 / 255.0f;
		material_car[HIER_OBJ_CAR_NUT].specular_color[3] = 1.0f;

		material_car[HIER_OBJ_CAR_NUT].specular_exponent = 3.0f;

		material_car[HIER_OBJ_CAR_NUT].emissive_color[0] = 0.4f;
		material_car[HIER_OBJ_CAR_NUT].emissive_color[1] = 0.0f;
		material_car[HIER_OBJ_CAR_NUT].emissive_color[2] = 0.0f;
		material_car[HIER_OBJ_CAR_NUT].emissive_color[3] = 1.0f;
	}
}

void set_material_car_object(int hier_obj_ID) {
	glUniform4fv(loc_material.ambient_color, 1, material_car[hier_obj_ID].ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_car[hier_obj_ID].diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_car[hier_obj_ID].specular_color);
	glUniform1f(loc_material.specular_exponent, material_car[hier_obj_ID].specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_car[hier_obj_ID].emissive_color);
}

void draw_hier_obj(int hier_obj_ID) {
	set_material_car_object(hier_obj_ID);

	//glFrontFace(GL_CCW);
	glBindVertexArray(hier_obj_VAO[hier_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * hier_obj_n_triangles[hier_obj_ID]);
	glBindVertexArray(0);
}

#define rad 1.7f
#define ww 1.0f
void draw_wheel_and_nut(int camera_id) {
	// angle is used in Hierarchical_Car_Correct later
	int i;
	
	//glUniform3f(loc_primitive_color, 211 / 255.0f, 156 / 255.0f, 39 / 255.0f);
	draw_hier_obj(HIER_OBJ_CAR_WHEEL); // draw wheel

	for (i = 0; i < 5; i++) {
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN*72.0f*i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_NUT;
		ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		
		//glUniform3f(loc_primitive_color, 215 / 255.0f, 46 / 255.0f, 42 / 255.0f);
		draw_hier_obj(HIER_OBJ_CAR_NUT); // draw i-th nut
	}
}

#define CAR_WHEEL_ROTATION_SPEED 10
void draw_car(int camera_id) {
	ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_BODY;
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	
	//glUniform3f(loc_primitive_color, 134 / 255.0f, 211 / 255.0f, 39 / 255.0f);
	draw_hier_obj(HIER_OBJ_CAR_BODY); // draw body

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 20.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, car_rotation_angle * CAR_WHEEL_ROTATION_SPEED * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_WHEEL;
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(camera_id);  // draw wheel 0, front left

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, car_rotation_angle * CAR_WHEEL_ROTATION_SPEED * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_WHEEL;
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(camera_id);  // draw wheel 1, back left

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 20.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, car_rotation_angle * CAR_WHEEL_ROTATION_SPEED * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_WHEEL;
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(camera_id);  // draw wheel 2, front right

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, car_rotation_angle * CAR_WHEEL_ROTATION_SPEED * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * ModelMatrix_CAR_WHEEL;
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(camera_id);  // draw wheel 3, back right
}

void free_hier_obj(int hier_obj_ID) {
	glDeleteVertexArrays(1, &hier_obj_VAO[hier_obj_ID]);
	glDeleteBuffers(1, &hier_obj_VBO[hier_obj_ID]);
}

#define N_TIGER_FRAMES 12
Object tiger[N_TIGER_FRAMES];

Material_Parameters material_tiger;

struct {
	int cur_frame = 0;
	float rotation_angle = 0.0f;
	float xCor, yCor, zCor;
	glm::vec3 pos;
	glm::vec3 headTo;
} tiger_data;

void define_animated_tiger(void) {
	TIGER_PATH_POINT pnt; 

	pnt.trigger = X;
	pnt.direction = COUNTERCLOCKWISE;
	pnt.center = glm::vec3(70.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = Y;
	pnt.direction = CLOCKWISE;
	pnt.center = glm::vec3(90.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = X;
	pnt.direction = CLOCKWISE;
	pnt.center = glm::vec3(195.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = Y;
	pnt.direction = CLOCKWISE;
	pnt.center = glm::vec3(195.0f, 60.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 270.0f;
	tiger_path_queue.push(pnt);
	
	pnt.trigger = X;
	pnt.direction = COUNTERCLOCKWISE;
	pnt.center = glm::vec3(195.0f, 85.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 180.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = X;
	pnt.direction = COUNTERCLOCKWISE;
	pnt.center = glm::vec3(90.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = Y;
	pnt.direction = CLOCKWISE;
	pnt.center = glm::vec3(70.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = X;
	pnt.direction = CLOCKWISE;
	pnt.center = glm::vec3(40.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 270.0f;
	tiger_path_queue.push(pnt);

	pnt.trigger = Y;
	pnt.direction = COUNTERCLOCKWISE;
	pnt.center = glm::vec3(60.0f, 90.0f, 0.0f);
	pnt.radius = 10.0f;
	pnt.angle = 90.0f;
	tiger_path_queue.push(pnt);

	tiger_inRotation = -1;
	tiger_data.pos = glm::vec3(40, 80, 0);
	tiger_data.headTo = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(tiger[i].filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);

		tiger[i].n_fields = 8;
		tiger[i].front_face_mode = GL_CW;
		prepare_geom_of_static_object(&(tiger[i]));

		tiger[i].n_geom_instances = 1;

		tiger[i].ModelMatrix[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

		tiger[i].material[0].emission = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
		tiger[i].material[0].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
		tiger[i].material[0].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
		tiger[i].material[0].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
		tiger[i].material[0].exponent = 51.2f;
	}

	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.628281f;
	material_tiger.specular_color[1] = 0.555802f;
	material_tiger.specular_color[2] = 0.366065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;
}

void set_material_tiger(void) {
	// assume ShaderProgram_PS is used
	glUniform4fv(loc_material.ambient_color, 1, material_tiger.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_tiger.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_tiger.specular_color);
	glUniform1f(loc_material.specular_exponent, material_tiger.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_tiger.emissive_color);
}

void draw_animated_tiger(int camera_id) {
	glm::vec3 orgHeadTo = glm::vec3(0.0f, -1.0f, 0.0f);
	TIGER_PATH_POINT pnt;

	if (tiger_inRotation >= 0.0f) {
		pnt = tiger_path_queue.front();
		
		TigerModellingMatrix = glm::translate(glm::mat4(1.0f), pnt.center);
		if (pnt.direction == COUNTERCLOCKWISE) {
			TigerModellingMatrix = glm::rotate(TigerModellingMatrix, (pnt.angle - tiger_inRotation) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
			TigerModellingMatrix = glm::rotate(TigerModellingMatrix, -(pnt.angle - tiger_inRotation) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		TigerModellingMatrix = glm::translate(TigerModellingMatrix, tiger_data.pos - pnt.center);
		if(tiger_data.headTo.x >= 0.0f)
			TigerModellingMatrix = glm::rotate(TigerModellingMatrix, acos((glm::dot(orgHeadTo, tiger_data.headTo) / (glm::length(orgHeadTo) * glm::length(tiger_data.headTo)))), glm::vec3(0.0f, 0.0f, 1.0f));
		else
			TigerModellingMatrix = glm::rotate(TigerModellingMatrix, -acos((glm::dot(orgHeadTo, tiger_data.headTo) / (glm::length(orgHeadTo) * glm::length(tiger_data.headTo)))), glm::vec3(0.0f, 0.0f, 1.0f));
		TigerModellingMatrix *= tiger[tiger_data.cur_frame].ModelMatrix[0];

		ModelViewMatrix[camera_id] = ViewMatrix[camera_id] * TigerModellingMatrix;
	}
	else {
		ModelViewMatrix[camera_id] = glm::translate(ViewMatrix[camera_id], tiger_data.pos);
		if (tiger_data.headTo.x >= 0.0f)
			ModelViewMatrix[camera_id] = glm::rotate(ModelViewMatrix[camera_id], acos((glm::dot(orgHeadTo, tiger_data.headTo) / (glm::length(orgHeadTo) * glm::length(tiger_data.headTo)))), glm::vec3(0.0f, 0.0f, 1.0f));
		else
			ModelViewMatrix[camera_id] = glm::rotate(ModelViewMatrix[camera_id], -acos((glm::dot(orgHeadTo, tiger_data.headTo) / (glm::length(orgHeadTo) * glm::length(tiger_data.headTo)))), glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix[camera_id] *= tiger[tiger_data.cur_frame].ModelMatrix[0];
	}
	
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[camera_id]));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[camera_id][0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);


	//glUniform3f(loc_primitive_color, tiger[tiger_data.cur_frame].material[0].diffuse.r,
		//tiger[tiger_data.cur_frame].material[0].diffuse.g, tiger[tiger_data.cur_frame].material[0].diffuse.b);

	//glFrontFace(GL_CW);
	glBindVertexArray(tiger[tiger_data.cur_frame].VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tiger[tiger_data.cur_frame].n_triangles);
	glBindVertexArray(0);

	
	glUseProgram(h_ShaderProgram_simple);
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes(camera_id);
	glLineWidth(1.0f);
}

GLuint points_VBO, points_VAO;
GLfloat point_vertices[][3] = { { 0.0f, 0.0f, 0.0f } };
/*
MAIN_CAM 0
FRONT_CAM 1
SIDE_CAM 2
TOP_CAM 3
CCTV_1 4
CCTV_2 5
CCTV_3 6
CCTV_DYN 7
*/
float camera_color[NUMBER_OF_CAMERAS][3] = {
	{ 23 / 255.0f, 222 / 255.0f, 217 / 255.0f }, // MAIN_CAM
	{ 0, 0, 0}, // FRONT_CAM
	{ 0, 0, 0 }, // SIDE_CAM
	{ 0, 0, 0 }, // TOP_CAM
	{ 100 / 255.0f, 10 / 255.0f, 200 / 255.0f }, // CCTV_1
	{ 100 / 255.0f, 10 / 255.0f, 200 / 255.0f }, // CCTV_2
	{ 100 / 255.0f, 10 / 255.0f, 200 / 255.0f }, // CCTV_3
	{ 0, 1, 0 } // CCTV_DYN
};

void define_camera(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &points_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point_vertices), &point_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &points_VAO);
	glBindVertexArray(points_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_camera(int camera_id, int camera_to_draw) {
	float* rgb = camera_color[camera_to_draw];

	ModelViewMatrix[camera_id] = glm::translate(ViewMatrix[camera_id], camera[camera_to_draw].pos);
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(points_VAO);
	glUniform3f(loc_primitive_color, rgb[0], rgb[1], rgb[2]);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
}

GLuint line_VBO, line_VAO;
GLfloat line_vertices[][3] = { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };

void define_line(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &line_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), &line_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &line_VAO);
	glBindVertexArray(line_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void swap(float* x, float* y) {
	float t = *x;
	*x = *y;
	*y = t;
}

void draw_line(int camera_id, float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b) {
	float length, zAng, xyAng;
	
	if (x2 < x1) {
		swap(&x1, &x2);
		swap(&y1, &y2);
		swap(&z1, &z2);
	}
	
	length = glm::sqrt(glm::pow((x2 - x1), 2) + glm::pow((y2 - y1), 2) + glm::pow((z2 - z1), 2)); // line length
	zAng = glm::atan(y2 - y1, x2 - x1); // angle to rotate along z-axis
	xyAng = glm::asin((z2 - z1) / length); // angle to rotate along tangent in xy-plane

	ModelViewMatrix[camera_id] = glm::translate(ViewMatrix[camera_id], glm::vec3(x1, y1, z1));
	ModelViewMatrix[camera_id] = glm::rotate(ModelViewMatrix[camera_id], xyAng, glm::cross(glm::vec3(x2 - x1, y2 - y1, z2 - z1), glm::vec3(0.0f, 0.0f, 1.0f)));
	ModelViewMatrix[camera_id] = glm::rotate(ModelViewMatrix[camera_id], zAng, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[camera_id] = glm::scale(ModelViewMatrix[camera_id], glm::vec3(length, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ProjectionMatrix[camera_id] * ModelViewMatrix[camera_id];
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(line_VAO);
	glUniform3f(loc_primitive_color, r / 255.0f, g / 255.0f, b / 255.0f);
	glLineWidth(2.0);
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.0);
	glBindVertexArray(0);
}

void cleanup_OpenGL_stuffs(void) {
	for (int i = 0; i < n_static_objects; i++) {
		glDeleteVertexArrays(1, &(static_objects[i].VAO));
		glDeleteBuffers(1, &(static_objects[i].VBO));
	}

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		glDeleteVertexArrays(1, &(tiger[i].VAO));
		glDeleteBuffers(1, &(tiger[i].VBO));
	}

	for (int i = 0; i < N_HIERARCHICAL_OBJECTS; i++)
		free_hier_obj(i);

	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);
}