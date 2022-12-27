#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 160
#define HEIGHT 44
/* Primary rotations */
float A, B, C;

/* Cube parameters */
float K1 = 20;	 					// Observer distance form the monitor
float K2 = 60;						// Object distance form the monitor
float cube_width = 25;
float cube_spacing = 0.5;

/* Light source location */
int light_x = 0;
int light_y = 0;
int light_z = -1;

/* Structures for renderig */
float zbuffer[WIDTH * HEIGHT];
char output[WIDTH * HEIGHT];


/* Calculate rotations based on euler angels rotations */
float calculate_X(int i, int j, int k) {
  return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
         j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

float calculate_Y(int i, int j, int k) {
  return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
         j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
         i * cos(B) * sin(C);
}
float calculate_Z(int i, int j, int k) {
  return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

/* Calculate dot product with light (light vector normalized) and surface normal on point */
float calculate_dot(float Nx, float Ny, float Nz){
	return Nx * light_x + Ny * light_y + Nz * light_z / sqrt(pow(light_x,2) + pow(light_y,2) + pow(light_z,2));
}

void calculate_surface(float cubeX, float cubeY, float cubeZ, float cubeNX, float cubeNY, float cubeNZ){
	float x = calculate_X(cubeX, cubeY, cubeZ);
	float y = calculate_Y(cubeX, cubeY, cubeZ);
	float z = calculate_Z(cubeX, cubeY, cubeZ) + K2;

	float ooz = 1.0 / z;

	float Nx = calculate_X(cubeNX, cubeNY, cubeNZ);
	float Ny = calculate_Y(cubeNX, cubeNY, cubeNZ);
	float Nz = calculate_Z(cubeNX, cubeNY, cubeNZ);

	// WHY !!!???
	int xp = (int) (WIDTH/2 + K1*ooz*x*2);
	int yp = (int) (HEIGHT/2 + K1*ooz*y);

	int idx = xp + yp * WIDTH;

	if(idx >= 0 && idx < WIDTH * HEIGHT){
		float L = calculate_dot(Nx, Ny, Nz);
		if( L > 0){
			if( ooz > zbuffer[idx] ){
				int luminance_index = L*12;
				output[idx] = ".,-~:;=!*#$@"[luminance_index];
				zbuffer[idx] = ooz;
			}
		}
	}

}

int main(){
	printf("\x1b[2J");
	A = 0; B = 0; C = 0;
	while (1) {
		memset(output, ' ', WIDTH * HEIGHT);
		memset(zbuffer, 0, WIDTH * HEIGHT * 4);
		// first cube
		for (float cubeX = -cube_width; cubeX < cube_width; cubeX += cube_spacing) {
			for (float cubeY = -cube_width; cubeY < cube_width; cubeY += cube_spacing) {
				calculate_surface(cubeX, cubeY, -cube_width, 0.0, 0.0, -1.0);
				calculate_surface(cubeX, cubeY, cube_width, 0.0, 0.0, 1.0);
				calculate_surface(cube_width, cubeY, cubeX, 1.0, 0.0 ,0.0);
				calculate_surface(-cube_width, cubeY, cubeX, -1.0, 0.0 ,0.0);
				calculate_surface(cubeX, -cube_width, cubeY, 0.0, -1.0, 0.0);
				calculate_surface(cubeX, cube_width, cubeY, 0.0, 1.0, 0.0);
			}

		}
	printf("\x1b[H");
    for (int k = 0; k < WIDTH * HEIGHT; k++) {
      putchar(k % WIDTH ? output[k] : '\n');
    }
    A += 0.05;
    B += 0.05;
    C += 0.01;
    usleep(8000 * 2);
  }
  return 0;

}
