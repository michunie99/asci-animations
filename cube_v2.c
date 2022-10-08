/* 
Second version of cube ASCII animation render
this version uses structures to store points 
and their normals. It'll be used to allow
for generating diffrent shapes easier
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define squared(x) (x*x)

/* 
Below are definitions for structures used to hold
coordinates, normals and shapes for render
*/

/* point coordinates */
typedef struct point{
    float x;
    float y;
    float z;
} point;

/* p1-starting point, p2-end point */
typedef struct vector{
    point p1;
    point p2;   
}vector;

/* Dot product of 2 vectors */
float dot_product(vector* v1, vector* v2){
    
    return  (v1->p2.x-v1->p1.x)*(v2->p2.x-v2->p1.x) +
            (v1->p2.y-v1->p1.y)*(v2->p2.y-v2->p1.y) +
            (v1->p2.z-v1->p1.z)*(v2->p2.z-v2->p1.z);
}

/* Normilize a vector of a given lenght */
void normalize(vector* v){
    /* Vector lenght */
    float lenght =  squared(v->p2.x-v->p1.x) + 
                    squared(v->p2.y-v->p1.y) + 
                    squared(v->p2.z-v->p1.z);
    
    /* Normalize the vector */
    v->p1.x /= lenght;
    v->p1.y /= lenght;
    v->p1.z /= lenght;

    return ;
}

/* Rotate point around (0,0) */
point rotate_point(float A, float B, float C, point p){
    /* Calculate sin and cos */
    float sinA = sin(A), cosA = cos(A);
    float sinB = sin(B), cosB = cos(B);
    float sinC = sin(C), cosC = cos(C);

    point ans;

    /* Calculate coordinates of the rotated point */
    ans.x = p.y*sinA*sinB*cosC - p.z*cosA*sinB*cosC+
            p.y*cosA*sinC + p.z*sinA*sinC + p.x*cosB*cosC;

    ans.y = p.y*cosA*cosC + p.z*sinA*cosC -
            p.y*sinA*sinB*sinC + p.z*cosA*sinB*sinC -
            p.x*cosB*sinC;

    ans.z = p.z*cosA*cosB - p.y*sinA*cosB + p.x*sinB;

    return ans;
}

// TODO point translation

vector rotate_vector(float A, float B, float C, vector v){
    vector ans;

    /* Rotate beginig and and points */
    ans.p1 = rotate_point(A,B,C,v.p1);
    ans.p2 = rotate_point(A,B,C,v.p2);

    return ans;
}

/* Structure to hold shape parameters */

typedef struct vertex{
    point point;
    vector normal;
}vertex;

typedef struct object{

    /* All the points */
    vertex* vertecies;

    /* number of vertecies */
    int n;

    /* Rotations */
    float A;
    float B;
    float C;

    /* Translations */
    float X;
    float Y;
    float Z;
}object;

void init_object(object* obj, vertex* vertecies, int n){
    
    /* Initialize shape vertecies */
    obj->n = n;
    obj->vertecies = vertecies;

    /* Initizlize rotations to 0 */
    obj->A=0;
    obj->B=0;
    obj->C=0;

    /* Initizlize translations to 0 */
    obj->X=0;
    obj->Y=0;
    obj->Z=0;
}

void del_object(object* obj){
    free(obj->vertecies);
}

/* Strucutures for rendering */
typedef struct screen{
    int width;
    int height;
}screen;

typedef struct camera{
    int K1;     // camera focal lenght
}camera;

typedef struct light_source{
    vector position;
}light_source;

/* Function for redering objects */
void render_objects(object* obj, int n, screen* s, camera* c, light_source* ls){
    /* zbuffer and output buffer */
    float *zbuffer = malloc(sizeof(float)*s->width*s->height);
    if(!zbuffer)
        exit(1);

    char * output = malloc(sizeof(char)*s->width*s->height);
    if(!zbuffer)
        exit(1);


    /* Clear output and zbuffer */
    memset(output, ' ', s->width * s->height);
    memset(zbuffer, 0, s->width * s->height * 4);

    // /*Reset screen */
    // printf("\x1b[2J");

    /* Normalize light source vector */
    normalize(&(ls->position));

    /* Iterate throught objects */
    for(int i=0; i<n; i++){
        /* Go throught all vertecies in an object */
        for(int j=0; j<(obj[i].n); j++){
            /* For each vertex calculate rotation */
            point r_point = rotate_point(   obj[i].A,
                                            obj[i].B,
                                            obj[i].C,
                                            obj[i].vertecies[j].point);

            // TODO add translations as a function
            r_point.x +=  obj[i].X;
            r_point.y +=  obj[i].Y;
            r_point.z +=  obj[i].Z;

            vector r_normal = rotate_vector(    obj[i].A,
                                                obj[i].B,
                                                obj[i].C,
                                                obj[i].vertecies[j].normal);
            
            normalize(&r_normal);
            /* Calcualate inverse of the z vector */
            float ooz = 1.0 / r_point.z;

            /* Calculate position on the screen */
            int xp = (int) (s->width/2 + c->K1*ooz*r_point.x*2);
	        int yp = (int) (s->height/2 + c->K1*ooz*r_point.y);

            int idx = xp + yp * s->width;

            if(idx >= 0 && idx < s->width * s->height){
                /* Calculate dot product of light and the surface */
                float L = dot_product(&r_normal, &(ls->position));
                if( L > 0){
                    if( ooz > zbuffer[idx] ){
                        int luminance_index = L*12;
                        output[idx] = ".,-~:;=!*#$@"[luminance_index];
                        zbuffer[idx] = ooz;
                    }
                }
            }
        }

    }

    /* Print the output */
    printf("\x1b[H");
    for (int k = 0; k < s->width * s->height; k++) {
      putchar(k % s->width ? output[k] : '\n');
    }
    free(zbuffer);
    free(output);
}


/* All object should have a center in (0,0,0) */
vertex* create_cube(int width, int* n){
    const float cube_spacing = 0.3;

    /* Allocate the buffer for the cube vertecies */
    vertex* cube = malloc( sizeof(vertex)*
                    6*squared((int)(2*width/cube_spacing)));

    //A Littel hack for now
    *n = 6*squared((int)(width/cube_spacing));
    /* If error during allocation return NULL pointer */
    if(!cube)
        return NULL;

    int i = 0;
    /* Loop on all the surfaces of a cube */
    for (float cubeX = -width; cubeX < width; cubeX += cube_spacing) {
			for (float cubeY = -width; cubeY < width; cubeY += cube_spacing) {
                /* Face 1 */
            	cube[i].point =  (point){  .x = cubeX, 
                                    .y = cubeY,
                                    .z = -width};

                cube[i].normal.p1 = (point){   .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i].normal.p2 = (point){   .x = 0.0, 
                                        .y = 0.0, 
                                        .z = -1.0};
                /* Face 2 */
            	cube[i*2].point =  (point){    .x = cubeX, 
                                        .y = cubeY,
                                        .z = width};

                cube[i*2].normal.p1 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i*2].normal.p2 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 1.0};

                /* Face 3 */
            	cube[i*3].point =  (point){    .x = width, 
                                        .y = cubeY,
                                        .z = cubeX};

                cube[i*3].normal.p1 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i*3].normal.p2 = (point){ .x = 1.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                /* Face 4 */
            	cube[i*4].point =  (point){    .x = -width, 
                                        .y = cubeY,
                                        .z = cubeX};

                cube[i*4].normal.p1 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i*4].normal.p2 = (point){ .x = -1.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                /* Face 5 */
            	cube[i*5].point =  (point){    .x = cubeX, 
                                        .y = -width,
                                        .z = cubeY};

                cube[i*5].normal.p1 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i*5].normal.p2 = (point){ .x = 0.0, 
                                        .y = -1.0, 
                                        .z = 0};

                /* Face 6 */
            	cube[i*6].point =  (point){    .x = cubeX, 
                                        .y = width,
                                        .z = cubeY};

                cube[i*6].normal.p1 = (point){ .x = 0.0, 
                                        .y = 0.0, 
                                        .z = 0.0};

                cube[i*6].normal.p2 = (point){ .x = 0.0, 
                                        .y = 1.0, 
                                        .z = 0.0};
                
                i++;
			}

		}

    return cube;
}


int main(){
    printf("\x1b[2J");

    screen s = {.height=44,
                .width=160};

    camera cam = {.K1=20};

    light_source ls;

    ls.position.p1 = (point){  .x=0,
                        .y=0,
                        .z=0};

    ls.position.p2 = (point){  .x=0,
                        .y=0,
                        .z=-1};

    object cube_1;
    int n;
    init_object(&cube_1,create_cube(20,&n),n);

    cube_1.Z = 60;
    float A, B ,C;

    while(1){
        render_objects(&cube_1, 1, &s, &cam , &ls);
        A += 0.05;
        B += 0.05;
        C += 0.01;
        usleep(8000 * 2);
    }

    del_object(&cube_1);
    return 0;
}