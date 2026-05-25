# Proyecto Final — Programación Paralela y Concurrente (D04)

**Universidad de Guadalajara — CUCEI**

Paralelización y optimización de la generación del Conjunto de Mandelbrot (8K) y filtro de convolución Gaussiano 2D mediante OpenMP.

## Estructura del repositorio

| Archivo | Descripción |
|---------|-------------|
| `mandelbrot_secuencial.c` | Código base secuencial (Tarea A: Mandelbrot 8K + Tarea B: filtro Gaussiano 15×15) |
| `mandelbrot_openmp.c` | Línea base paralela generada con IA (OpenMP, `schedule(static)`) |
| `mandelbrot_benchmark.c` | Optimizaciones manuales: schedulers, histograma (atomic vs reduction), SPMD vectorizado, afinidad |

## Compilación

```bash
# Secuencial
gcc -std=c11 -O2 -o mandelbrot_seq mandelbrot_secuencial.c -lm

# OpenMP básico
gcc -std=c11 -O2 -fopenmp -o mandelbrot_omp mandelbrot_openmp.c -lm

# Benchmark completo (con verificación de vectorización)
gcc -std=c11 -O2 -fopenmp -ftree-vectorize -fopt-info-vec-optimized \
    -o mandelbrot_bench mandelbrot_benchmark.c -lm
```

## Ejecución

```bash
# Secuencial
./mandelbrot_seq

# OpenMP (usa todos los hilos disponibles)
./mandelbrot_omp

# Benchmark completo
./mandelbrot_bench

# Afinidad de hilos (10 pts extra)
OMP_PROC_BIND=close OMP_PLACES=cores ./mandelbrot_bench
OMP_PROC_BIND=spread OMP_PLACES=threads ./mandelbrot_bench
```

## Hardware de prueba

- **CPU:** Intel Core i7-13620H (10 núcleos: 6 P-cores + 4 E-cores, 16 hilos)
- **RAM:** 40 GB
- **Caché:** L1 6×48KB + 4×32KB, L2 6×1280KB + 1×2048KB, L3 24MB
- **SO:** Windows 11 + WSL2 (Ubuntu)

## Resultados principales

| Métrica | Valor |
|---------|-------|
| Tiempo secuencial total | 36.92 s |
| Tiempo OpenMP (16 hilos, static) | 5.46 s (speedup 6.76×) |
| Mejor scheduler (dynamic, chunk=8) | 1.66 s (speedup 12.83×) |
| Histograma: atomic vs reduction | 1.806 s vs 0.019 s (93× más lento) |
| Filtro SPMD vectorizado | 5.6% mejora sobre estándar |

## Autor

**Juan Esteban Bolívar Ferrer** — 226392489  
Profesor: Ángel Ignacio Paredes López
