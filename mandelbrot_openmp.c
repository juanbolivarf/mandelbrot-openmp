#define _POSIX_C_SOURCE 199309L

/*
 * mandelbrot_openmp.c
 * 
 * Versión paralelizada con OpenMP:
 *   Tarea A: Genera una imagen 8K (7680x4320) del Conjunto de Mandelbrot
 *   Tarea B: Aplica un filtro Gaussiano de radio 7 (kernel 15x15)
 *
 * Esta es la "Línea Base Paralela de la IA" — paralelización directa
 * del código secuencial usando OpenMP.
 *
 * Compilación:
 *   gcc -std=c11 -O2 -fopenmp -o mandelbrot_omp mandelbrot_openmp.c -lm
 *
 * Ejecución:
 *   ./mandelbrot_omp                    # Usa todos los hilos disponibles
 *   OMP_NUM_THREADS=4 ./mandelbrot_omp  # Forzar 4 hilos
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* ── Parámetros de imagen ── */
#define WIDTH  7680
#define HEIGHT 4320

/* ── Parámetros del fractal ── */
#define MAX_ITER 1000
#define X_MIN   -2.5
#define X_MAX    1.0
#define Y_MIN   -1.2
#define Y_MAX    1.2

/* ── Parámetros del filtro Gaussiano ── */
#define KERNEL_RADIUS 7
#define KERNEL_SIZE   (2 * KERNEL_RADIUS + 1)  /* 15x15 */
#define SIGMA         3.5

/* ── Estructura para un píxel RGB ── */
typedef struct {
    unsigned char r, g, b;
} Pixel;

/* ────────────────────────────────────────────────
 * Utilidad: tiempo en segundos con alta resolución
 * ──────────────────────────────────────────────── */
static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ────────────────────────────────────────────────
 * Mapeo de iteraciones a color (paleta suave)
 * ──────────────────────────────────────────────── */
static Pixel iter_to_color(int iter, double zr, double zi) {
    Pixel p;
    if (iter == MAX_ITER) {
        p.r = p.g = p.b = 0;
        return p;
    }

    double log_zn = log(zr * zr + zi * zi) / 2.0;
    double nu = log(log_zn / log(2.0)) / log(2.0);
    double smooth = iter + 1.0 - nu;

    double t = smooth / MAX_ITER;
    double hue = 360.0 * fmod(t * 5.0, 1.0);
    double sat = 0.8;
    double val = 1.0;

    double c = val * sat;
    double x = c * (1.0 - fabs(fmod(hue / 60.0, 2.0) - 1.0));
    double m = val - c;
    double rf, gf, bf;

    if      (hue < 60)  { rf = c; gf = x; bf = 0; }
    else if (hue < 120) { rf = x; gf = c; bf = 0; }
    else if (hue < 180) { rf = 0; gf = c; bf = x; }
    else if (hue < 240) { rf = 0; gf = x; bf = c; }
    else if (hue < 300) { rf = x; gf = 0; bf = c; }
    else                { rf = c; gf = 0; bf = x; }

    p.r = (unsigned char)((rf + m) * 255);
    p.g = (unsigned char)((gf + m) * 255);
    p.b = (unsigned char)((bf + m) * 255);
    return p;
}

/* ────────────────────────────────────────────────
 * TAREA A: Generar el Conjunto de Mandelbrot (OpenMP)
 * ──────────────────────────────────────────────── */
static void generar_mandelbrot(Pixel *img) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            double cr = X_MIN + (X_MAX - X_MIN) * x / (WIDTH - 1);
            double ci = Y_MIN + (Y_MAX - Y_MIN) * y / (HEIGHT - 1);

            double zr = 0.0, zi = 0.0;
            int iter = 0;

            while (zr * zr + zi * zi <= 4.0 && iter < MAX_ITER) {
                double tmp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = tmp;
                iter++;
            }

            img[y * WIDTH + x] = iter_to_color(iter, zr, zi);
        }
    }
}

/* ────────────────────────────────────────────────
 * Generar kernel Gaussiano normalizado
 * ──────────────────────────────────────────────── */
static void generar_kernel_gaussiano(double kernel[KERNEL_SIZE][KERNEL_SIZE]) {
    double sum = 0.0;
    for (int ky = -KERNEL_RADIUS; ky <= KERNEL_RADIUS; ky++) {
        for (int kx = -KERNEL_RADIUS; kx <= KERNEL_RADIUS; kx++) {
            double val = exp(-(kx * kx + ky * ky) / (2.0 * SIGMA * SIGMA));
            kernel[ky + KERNEL_RADIUS][kx + KERNEL_RADIUS] = val;
            sum += val;
        }
    }
    for (int ky = 0; ky < KERNEL_SIZE; ky++)
        for (int kx = 0; kx < KERNEL_SIZE; kx++)
            kernel[ky][kx] /= sum;
}

/* ────────────────────────────────────────────────
 * TAREA B: Aplicar filtro Gaussiano (OpenMP)
 * ──────────────────────────────────────────────── */
static void aplicar_filtro_gaussiano(const Pixel *src, Pixel *dst) {
    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generar_kernel_gaussiano(kernel);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            double sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;

            for (int ky = -KERNEL_RADIUS; ky <= KERNEL_RADIUS; ky++) {
                for (int kx = -KERNEL_RADIUS; kx <= KERNEL_RADIUS; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny < 0) ny = 0;
                    if (ny >= HEIGHT) ny = HEIGHT - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= WIDTH) nx = WIDTH - 1;

                    double w = kernel[ky + KERNEL_RADIUS][kx + KERNEL_RADIUS];
                    Pixel p = src[ny * WIDTH + nx];
                    sum_r += w * p.r;
                    sum_g += w * p.g;
                    sum_b += w * p.b;
                }
            }

            dst[y * WIDTH + x].r = (unsigned char)fmin(fmax(sum_r, 0.0), 255.0);
            dst[y * WIDTH + x].g = (unsigned char)fmin(fmax(sum_g, 0.0), 255.0);
            dst[y * WIDTH + x].b = (unsigned char)fmin(fmax(sum_b, 0.0), 255.0);
        }
    }
}

/* ────────────────────────────────────────────────
 * Guardar imagen en formato PPM (P6 binario)
 * ──────────────────────────────────────────────── */
static int guardar_ppm(const char *filename, const Pixel *img) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error al abrir archivo de salida");
        return -1;
    }
    fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    fwrite(img, sizeof(Pixel), WIDTH * HEIGHT, fp);
    fclose(fp);
    return 0;
}

/* ────────────────────────────────────────────────
 * main
 * ──────────────────────────────────────────────── */
int main(void) {
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    printf("=== Mandelbrot OpenMP (Línea Base IA) ===\n");
    printf("Hilos OpenMP: %d\n", num_threads);
    printf("Imagen: %d x %d (%d píxeles)\n", WIDTH, HEIGHT, WIDTH * HEIGHT);
    printf("Iteraciones máx: %d\n", MAX_ITER);
    printf("Kernel Gaussiano: %dx%d (sigma=%.1f)\n\n", KERNEL_SIZE, KERNEL_SIZE, SIGMA);

    size_t img_size = (size_t)WIDTH * HEIGHT * sizeof(Pixel);
    printf("Memoria requerida: %.1f MB por buffer\n\n", img_size / (1024.0 * 1024.0));

    Pixel *img_fractal = (Pixel *)malloc(img_size);
    Pixel *img_filtrada = (Pixel *)malloc(img_size);
    if (!img_fractal || !img_filtrada) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        return EXIT_FAILURE;
    }

    /* ── Tarea A: Generar Mandelbrot ── */
    printf("Generando Mandelbrot...\n");
    double t0 = get_time();
    generar_mandelbrot(img_fractal);
    double t1 = get_time();
    printf("  Tiempo Tarea A (Mandelbrot): %.3f s\n\n", t1 - t0);

    guardar_ppm("mandelbrot_original.ppm", img_fractal);
    printf("  Imagen original guardada.\n\n");

    /* ── Tarea B: Aplicar filtro Gaussiano ── */
    printf("Aplicando filtro Gaussiano...\n");
    double t2 = get_time();
    aplicar_filtro_gaussiano(img_fractal, img_filtrada);
    double t3 = get_time();
    printf("  Tiempo Tarea B (Filtro):      %.3f s\n\n", t3 - t2);

    guardar_ppm("mandelbrot_filtrado.ppm", img_filtrada);
    printf("  Imagen filtrada guardada.\n\n");

    /* ── Resumen ── */
    printf("=== Resumen de tiempos ===\n");
    printf("  Hilos:                %d\n", num_threads);
    printf("  Tarea A (Mandelbrot): %.3f s\n", t1 - t0);
    printf("  Tarea B (Filtro):     %.3f s\n", t3 - t2);
    printf("  Total:                %.3f s\n", t3 - t0);

    free(img_fractal);
    free(img_filtrada);

    return EXIT_SUCCESS;
}
