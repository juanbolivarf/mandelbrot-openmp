#define _POSIX_C_SOURCE 199309L

/*
 * mandelbrot_benchmark.c
 *
 * Programa integral de benchmarking que cubre:
 *   - Fase 3: Comparación de schedulers (static, dynamic, guided) con distintos chunk sizes
 *   - Fase 4: Histograma con atomic vs reduction + análisis de false sharing
 *   - Fase 5: Filtro con vectorización SPMD
 *   - Medición de tiempos para gráficas de speedup
 *
 * Compilación:
 *   gcc -std=c11 -O2 -fopenmp -o mandelbrot_bench mandelbrot_benchmark.c -lm
 *
 * Para verificar vectorización (Fase 5):
 *   gcc -std=c11 -O2 -fopenmp -ftree-vectorize -fopt-info-vec-optimized \
 *       -o mandelbrot_bench mandelbrot_benchmark.c -lm
 *
 * Ejecución:
 *   ./mandelbrot_bench
 *
 * Variables de entorno para afinidad (10 pts extra):
 *   OMP_PROC_BIND=close OMP_PLACES=cores ./mandelbrot_bench
 *   OMP_PROC_BIND=spread OMP_PLACES=threads ./mandelbrot_bench
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
#define KERNEL_SIZE   (2 * KERNEL_RADIUS + 1)
#define SIGMA         3.5

/* ── Número de bins para el histograma ── */
#define HIST_BINS 256

/* ── Repeticiones para promediar mediciones ── */
#define REPS 3

typedef struct {
    unsigned char r, g, b;
} Pixel;

/* ════════════════════════════════════════════════
 * UTILIDADES
 * ════════════════════════════════════════════════ */

static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

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
    double sat = 0.8, val = 1.0;
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

static void generar_kernel_gaussiano(double kernel[KERNEL_SIZE][KERNEL_SIZE]) {
    double sum = 0.0;
    for (int ky = -KERNEL_RADIUS; ky <= KERNEL_RADIUS; ky++)
        for (int kx = -KERNEL_RADIUS; kx <= KERNEL_RADIUS; kx++) {
            double val = exp(-(kx * kx + ky * ky) / (2.0 * SIGMA * SIGMA));
            kernel[ky + KERNEL_RADIUS][kx + KERNEL_RADIUS] = val;
            sum += val;
        }
    for (int ky = 0; ky < KERNEL_SIZE; ky++)
        for (int kx = 0; kx < KERNEL_SIZE; kx++)
            kernel[ky][kx] /= sum;
}

static int guardar_ppm(const char *filename, const Pixel *img) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) { perror("Error"); return -1; }
    fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    fwrite(img, sizeof(Pixel), (size_t)WIDTH * HEIGHT, fp);
    fclose(fp);
    return 0;
}

/* ════════════════════════════════════════════════
 * TAREA A: Mandelbrot con scheduler configurable
 * ════════════════════════════════════════════════ */

/* schedule(static) — por defecto */
static void mandelbrot_static(Pixel *img) {
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

/* schedule(dynamic, chunk) */
static void mandelbrot_dynamic(Pixel *img, int chunk) {
    #pragma omp parallel for schedule(dynamic, chunk)
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

/* schedule(guided, chunk) */
static void mandelbrot_guided(Pixel *img, int chunk) {
    #pragma omp parallel for schedule(guided, chunk)
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

/* ════════════════════════════════════════════════
 * TAREA B: Filtro Gaussiano estándar (OpenMP)
 * ════════════════════════════════════════════════ */
static void filtro_gaussiano_omp(const Pixel *src, Pixel *dst) {
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

/* ════════════════════════════════════════════════
 * TAREA B (SPMD): Filtro con vectorización forzada
 * ════════════════════════════════════════════════ */
static void filtro_gaussiano_spmd(const Pixel *src, Pixel *dst) {
    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generar_kernel_gaussiano(kernel);

    /*
     * Para SPMD/vectorización, separamos los canales en arreglos
     * contiguos de doubles para que el compilador pueda vectorizar
     * el bucle interno sobre x con instrucciones SIMD (AVX2).
     */
    #pragma omp parallel
    {
        /* Buffers locales por hilo para una fila de salida */
        double *row_r = (double *)malloc(WIDTH * sizeof(double));
        double *row_g = (double *)malloc(WIDTH * sizeof(double));
        double *row_b = (double *)malloc(WIDTH * sizeof(double));

        #pragma omp for schedule(static)
        for (int y = 0; y < HEIGHT; y++) {
            /* Inicializar acumuladores a cero */
            memset(row_r, 0, WIDTH * sizeof(double));
            memset(row_g, 0, WIDTH * sizeof(double));
            memset(row_b, 0, WIDTH * sizeof(double));

            /* Convolución: iterar sobre el kernel, vectorizar sobre x */
            for (int ky = -KERNEL_RADIUS; ky <= KERNEL_RADIUS; ky++) {
                int sy = y + ky;
                if (sy < 0) sy = 0;
                if (sy >= HEIGHT) sy = HEIGHT - 1;

                const Pixel *src_row = &src[sy * WIDTH];

                for (int kx = -KERNEL_RADIUS; kx <= KERNEL_RADIUS; kx++) {
                    double w = kernel[ky + KERNEL_RADIUS][kx + KERNEL_RADIUS];

                    /*
                     * Este es el bucle que debe vectorizarse.
                     * Compilar con: gcc -O2 -ftree-vectorize -fopt-info-vec-optimized
                     * para verificar que se vectorizó.
                     *
                     * La directiva omp simd fuerza la vectorización SPMD.
                     */
                    #pragma omp simd
                    for (int x = 0; x < WIDTH; x++) {
                        int nx = x + kx;
                        /* Clamping sin branch para ayudar vectorización */
                        nx = nx < 0 ? 0 : (nx >= WIDTH ? WIDTH - 1 : nx);

                        row_r[x] += w * src_row[nx].r;
                        row_g[x] += w * src_row[nx].g;
                        row_b[x] += w * src_row[nx].b;
                    }
                }
            }

            /* Escribir resultados clampeados */
            Pixel *dst_row = &dst[y * WIDTH];
            #pragma omp simd
            for (int x = 0; x < WIDTH; x++) {
                dst_row[x].r = (unsigned char)(row_r[x] < 0 ? 0 : (row_r[x] > 255 ? 255 : row_r[x]));
                dst_row[x].g = (unsigned char)(row_g[x] < 0 ? 0 : (row_g[x] > 255 ? 255 : row_g[x]));
                dst_row[x].b = (unsigned char)(row_b[x] < 0 ? 0 : (row_b[x] > 255 ? 255 : row_b[x]));
            }
        }

        free(row_r);
        free(row_g);
        free(row_b);
    }
}

/* ════════════════════════════════════════════════
 * FASE 4: Histograma — versión con ATOMIC
 * (Propensa a false sharing y contención)
 * ════════════════════════════════════════════════ */
static void histograma_atomic(const Pixel *img, long hist_r[HIST_BINS],
                               long hist_g[HIST_BINS], long hist_b[HIST_BINS]) {
    memset(hist_r, 0, HIST_BINS * sizeof(long));
    memset(hist_g, 0, HIST_BINS * sizeof(long));
    memset(hist_b, 0, HIST_BINS * sizeof(long));

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        #pragma omp atomic
        hist_r[img[i].r]++;
        #pragma omp atomic
        hist_g[img[i].g]++;
        #pragma omp atomic
        hist_b[img[i].b]++;
    }
}

/* ════════════════════════════════════════════════
 * FASE 4: Histograma — versión con FALSE SHARING
 * (Arreglo compartido indexado por thread_id — MAL diseño)
 * ════════════════════════════════════════════════ */
static void histograma_false_sharing(const Pixel *img, long hist_r[HIST_BINS],
                                      long hist_g[HIST_BINS], long hist_b[HIST_BINS]) {
    int max_threads = omp_get_max_threads();

    /*
     * Arreglo compartido donde cada hilo escribe en posiciones contiguas.
     * Esto provoca FALSE SHARING: las líneas de caché L1 (64 bytes) contienen
     * datos de múltiples hilos, causando invalidaciones constantes.
     *
     * Estructura: hist_shared[thread_id][bin] — los thread_id son contiguos
     * en la primera dimensión, así que hilos adyacentes invalidan la caché
     * del otro.
     */
    long *hist_shared_r = (long *)calloc((size_t)max_threads * HIST_BINS, sizeof(long));
    long *hist_shared_g = (long *)calloc((size_t)max_threads * HIST_BINS, sizeof(long));
    long *hist_shared_b = (long *)calloc((size_t)max_threads * HIST_BINS, sizeof(long));

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long *my_r = &hist_shared_r[tid * HIST_BINS];
        long *my_g = &hist_shared_g[tid * HIST_BINS];
        long *my_b = &hist_shared_b[tid * HIST_BINS];

        #pragma omp for schedule(static)
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            my_r[img[i].r]++;
            my_g[img[i].g]++;
            my_b[img[i].b]++;
        }
    }

    /* Reducir resultados */
    memset(hist_r, 0, HIST_BINS * sizeof(long));
    memset(hist_g, 0, HIST_BINS * sizeof(long));
    memset(hist_b, 0, HIST_BINS * sizeof(long));
    for (int t = 0; t < max_threads; t++) {
        for (int i = 0; i < HIST_BINS; i++) {
            hist_r[i] += hist_shared_r[t * HIST_BINS + i];
            hist_g[i] += hist_shared_g[t * HIST_BINS + i];
            hist_b[i] += hist_shared_b[t * HIST_BINS + i];
        }
    }

    free(hist_shared_r);
    free(hist_shared_g);
    free(hist_shared_b);
}

/* ════════════════════════════════════════════════
 * FASE 4: Histograma — versión con REDUCTION (óptima)
 * (Variables locales por hilo, sin false sharing)
 * ════════════════════════════════════════════════ */
static void histograma_reduction(const Pixel *img, long hist_r[HIST_BINS],
                                  long hist_g[HIST_BINS], long hist_b[HIST_BINS]) {
    int max_threads = omp_get_max_threads();

    /*
     * Cada hilo trabaja con su propia copia del histograma, alineada
     * a 64 bytes para evitar false sharing entre líneas de caché.
     * Al final se reduce manualmente.
     *
     * Nota: OpenMP no soporta reduction sobre arreglos en C (sí en Fortran),
     * así que simulamos la cláusula reduction con arreglos locales
     * explícitamente paddeados.
     */

    /* Padding a línea de caché (64 bytes = 8 longs) */
    #define CACHE_LINE_LONGS 8
    #define PADDED_BINS ((HIST_BINS + CACHE_LINE_LONGS - 1) / CACHE_LINE_LONGS * CACHE_LINE_LONGS)

    long *local_r = (long *)calloc((size_t)max_threads * PADDED_BINS, sizeof(long));
    long *local_g = (long *)calloc((size_t)max_threads * PADDED_BINS, sizeof(long));
    long *local_b = (long *)calloc((size_t)max_threads * PADDED_BINS, sizeof(long));

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long *my_r = &local_r[tid * PADDED_BINS];
        long *my_g = &local_g[tid * PADDED_BINS];
        long *my_b = &local_b[tid * PADDED_BINS];

        #pragma omp for schedule(static)
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            my_r[img[i].r]++;
            my_g[img[i].g]++;
            my_b[img[i].b]++;
        }
    }

    /* Reducción final */
    memset(hist_r, 0, HIST_BINS * sizeof(long));
    memset(hist_g, 0, HIST_BINS * sizeof(long));
    memset(hist_b, 0, HIST_BINS * sizeof(long));
    for (int t = 0; t < max_threads; t++) {
        for (int i = 0; i < HIST_BINS; i++) {
            hist_r[i] += local_r[t * PADDED_BINS + i];
            hist_g[i] += local_g[t * PADDED_BINS + i];
            hist_b[i] += local_b[t * PADDED_BINS + i];
        }
    }

    free(local_r);
    free(local_g);
    free(local_b);
}

/* ════════════════════════════════════════════════
 * UTILIDAD: Medir tiempo promediado sobre REPS
 * ════════════════════════════════════════════════ */
typedef void (*mandelbrot_fn)(Pixel *);
typedef void (*mandelbrot_chunk_fn)(Pixel *, int);

static double medir_mandelbrot(mandelbrot_fn fn, Pixel *img) {
    double total = 0.0;
    for (int r = 0; r < REPS; r++) {
        double t0 = get_time();
        fn(img);
        total += get_time() - t0;
    }
    return total / REPS;
}

static double medir_mandelbrot_chunk(mandelbrot_chunk_fn fn, Pixel *img, int chunk) {
    double total = 0.0;
    for (int r = 0; r < REPS; r++) {
        double t0 = get_time();
        fn(img, chunk);
        total += get_time() - t0;
    }
    return total / REPS;
}

/* ════════════════════════════════════════════════
 * MAIN: Ejecutar todos los benchmarks
 * ════════════════════════════════════════════════ */
int main(void) {
    int max_threads = omp_get_max_threads();

    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║     BENCHMARK MANDELBROT — PROYECTO FINAL         ║\n");
    printf("╚════════════════════════════════════════════════════╝\n\n");

    printf("Configuración:\n");
    printf("  Imagen:       %d x %d\n", WIDTH, HEIGHT);
    printf("  MAX_ITER:     %d\n", MAX_ITER);
    printf("  Kernel:       %dx%d (sigma=%.1f)\n", KERNEL_SIZE, KERNEL_SIZE, SIGMA);
    printf("  Max hilos:    %d\n", max_threads);
    printf("  Repeticiones: %d\n", REPS);

    /* Mostrar info de afinidad */
    const char *proc_bind = getenv("OMP_PROC_BIND");
    const char *places = getenv("OMP_PLACES");
    printf("  OMP_PROC_BIND: %s\n", proc_bind ? proc_bind : "(no establecido)");
    printf("  OMP_PLACES:    %s\n\n", places ? places : "(no establecido)");

    size_t img_size = (size_t)WIDTH * HEIGHT * sizeof(Pixel);
    Pixel *img = (Pixel *)malloc(img_size);
    Pixel *img_dst = (Pixel *)malloc(img_size);
    if (!img || !img_dst) {
        fprintf(stderr, "Error de memoria\n");
        return EXIT_FAILURE;
    }

    /* ──────────────────────────────────────────────
     * PARTE 1: Medición secuencial (1 hilo) — línea base
     * ────────────────────────────────────────────── */
    printf("━━━ PARTE 1: Línea base secuencial (1 hilo) ━━━\n");
    omp_set_num_threads(1);
    double t_seq_A = medir_mandelbrot(mandelbrot_static, img);
    double t_seq_B;
    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            filtro_gaussiano_omp(img, img_dst);
            total += get_time() - t0;
        }
        t_seq_B = total / REPS;
    }
    printf("  Tarea A (Mandelbrot): %.3f s\n", t_seq_A);
    printf("  Tarea B (Filtro):     %.3f s\n", t_seq_B);
    printf("  Total secuencial:     %.3f s\n\n", t_seq_A + t_seq_B);

    /* Guardar imagen de referencia */
    guardar_ppm("mandelbrot_original.ppm", img);
    filtro_gaussiano_omp(img, img_dst);
    guardar_ppm("mandelbrot_filtrado.ppm", img_dst);

    /* Restaurar hilos */
    omp_set_num_threads(max_threads);

    /* ──────────────────────────────────────────────
     * PARTE 2: Schedulers (Fase 3) — Tarea A
     * ────────────────────────────────────────────── */
    printf("━━━ PARTE 2: Comparación de Schedulers (Tarea A) ━━━\n");
    printf("%-12s %-8s %10s %10s\n", "Scheduler", "Chunk", "Tiempo(s)", "Speedup");
    printf("──────────────────────────────────────────────────\n");

    double t_static = medir_mandelbrot(mandelbrot_static, img);
    printf("%-12s %-8s %10.3f %10.2fx\n", "static", "default", t_static, t_seq_A / t_static);

    int chunks[] = {1, 4, 8, 16, 32, 64};
    int n_chunks = sizeof(chunks) / sizeof(chunks[0]);

    for (int i = 0; i < n_chunks; i++) {
        double t = medir_mandelbrot_chunk(mandelbrot_dynamic, img, chunks[i]);
        printf("%-12s %-8d %10.3f %10.2fx\n", "dynamic", chunks[i], t, t_seq_A / t);
    }
    for (int i = 0; i < n_chunks; i++) {
        double t = medir_mandelbrot_chunk(mandelbrot_guided, img, chunks[i]);
        printf("%-12s %-8d %10.3f %10.2fx\n", "guided", chunks[i], t, t_seq_A / t);
    }
    printf("\n");

    /* ──────────────────────────────────────────────
     * PARTE 3: Speedup vs número de hilos
     * ────────────────────────────────────────────── */
    printf("━━━ PARTE 3: Speedup vs Número de Hilos ━━━\n");
    printf("%-8s %10s %10s %10s %10s\n", "Hilos", "T_A(s)", "T_B(s)", "SpeedA", "SpeedB");
    printf("──────────────────────────────────────────────────\n");

    /* Probar desde 1 hasta 2x hilos lógicos */
    int thread_counts[] = {1, 2, 4, 6, 8, 10, 12, 16, 20, 24, 32};
    int n_thread_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    for (int ti = 0; ti < n_thread_tests; ti++) {
        int nt = thread_counts[ti];
        if (nt > 2 * max_threads) break;

        omp_set_num_threads(nt);

        /* Tarea A con dynamic,8 (buen balance para Mandelbrot) */
        double tA = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            mandelbrot_dynamic(img, 8);
            tA += get_time() - t0;
        }
        tA /= REPS;

        /* Tarea B con filtro estándar */
        double tB = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            filtro_gaussiano_omp(img, img_dst);
            tB += get_time() - t0;
        }
        tB /= REPS;

        printf("%-8d %10.3f %10.3f %10.2fx %10.2fx\n",
               nt, tA, tB, t_seq_A / tA, t_seq_B / tB);
    }
    printf("\n");

    omp_set_num_threads(max_threads);

    /* ──────────────────────────────────────────────
     * PARTE 4: Histograma — atomic vs reduction vs false sharing
     * ────────────────────────────────────────────── */
    printf("━━━ PARTE 4: Histograma (atomic vs false_sharing vs reduction) ━━━\n");

    long hist_r[HIST_BINS], hist_g[HIST_BINS], hist_b[HIST_BINS];

    /* Atomic */
    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            histograma_atomic(img, hist_r, hist_g, hist_b);
            total += get_time() - t0;
        }
        printf("  atomic:        %.4f s\n", total / REPS);
    }

    /* False sharing */
    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            histograma_false_sharing(img, hist_r, hist_g, hist_b);
            total += get_time() - t0;
        }
        printf("  false_sharing: %.4f s\n", total / REPS);
    }

    /* Reduction (con padding) */
    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            histograma_reduction(img, hist_r, hist_g, hist_b);
            total += get_time() - t0;
        }
        printf("  reduction:     %.4f s\n", total / REPS);
    }

    /* Verificación rápida del histograma */
    long total_pixels = 0;
    for (int i = 0; i < HIST_BINS; i++) total_pixels += hist_r[i];
    printf("  Verificación: %ld píxeles contados (esperado: %d)\n\n",
           total_pixels, WIDTH * HEIGHT);

    /* ──────────────────────────────────────────────
     * PARTE 5: Filtro SPMD vectorizado vs estándar
     * ────────────────────────────────────────────── */
    printf("━━━ PARTE 5: Filtro estándar vs SPMD vectorizado ━━━\n");

    omp_set_num_threads(max_threads);

    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            filtro_gaussiano_omp(img, img_dst);
            total += get_time() - t0;
        }
        printf("  Filtro estándar: %.3f s\n", total / REPS);
    }

    {
        double total = 0.0;
        for (int r = 0; r < REPS; r++) {
            double t0 = get_time();
            filtro_gaussiano_spmd(img, img_dst);
            total += get_time() - t0;
        }
        printf("  Filtro SPMD:     %.3f s\n", total / REPS);
    }

    printf("\n");
    printf("Benchmark completado.\n");
    printf("Para los 10 pts extra, re-ejecutar con:\n");
    printf("  OMP_PROC_BIND=close OMP_PLACES=cores ./mandelbrot_bench\n");
    printf("  OMP_PROC_BIND=spread OMP_PLACES=threads ./mandelbrot_bench\n");

    free(img);
    free(img_dst);
    return EXIT_SUCCESS;
}
