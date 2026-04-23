// FFT.cpp - Implementaci�n de an�lisis espectral mediante FFT
//
// Usa la librer�a FFTW3 para realizar transformadas de Fourier optimizadas.
// Calcula el espectro de amplitudes desde se�ales en dominio del tiempo.

#include "FFT.h"

#include <algorithm>
#include <implot.h>
#include <cmath>

// Calcula la magnitud de un n�mero complejo (sqrt(real� + imag�))
double magnitude(const fftw_complex complex) {
    return std::sqrt(complex[0] * complex[0] + complex[1] * complex[1]);
}

// ════════════════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR DE FFT - Inicialización del Motor de Análisis Espectral
// ════════════════════════════════════════════════════════════════════════════════════════
//
// PROPÓSITO:
// Prepara todo lo necesario para realizar transformadas de Fourier rápidas (FFT) sobre
// señales digitales en tiempo real. Usa FFTW3, una de las implementaciones más eficientes
// de FFT disponibles.
//
// PARÁMETROS:
// - sample_count: Número de muestras temporales a analizar (debe ser potencia de 2 para
//   mejor rendimiento, aunque FFTW3 trabaja con cualquier tamaño)
//
// DETALLES TÉCNICOS:
// 1. samples_size = sample_count: Tamaño del buffer de entrada (dominio temporal)
// 
// 2. amplitudes_size = sample_count / 2 + 1: Tamaño del buffer de salida (dominio frecuencial)
//    • Solo necesitamos la mitad + 1 porque explotamos la simetría de Hermite
//    • Para señales reales, el espectro es simétrico: X[f] = X*[-f]
//    • Ahorro de memoria: 2× menos espacio, 2× menos cálculos
//
// 3. Reserva de memoria con fftw_malloc():
//    • fftw_malloc() garantiza alineación SIMD (16 bytes), crucial para AVX/SSE
//    • Alineación incorrecta puede reducir rendimiento hasta 10×
//    • complex[]: array de números complejos (cada uno tiene parte real e imaginaria)
//
// 4. Creación del "plan" con fftw_plan_dft_r2c_1d():
//    • El plan es una estrategia optimizada de cómo ejecutar la FFT
//    • "r2c" = Real to Complex: entrada real, salida compleja (explota simetría)
//    • "1d" = Unidimensional (también existe 2D para imágenes, 3D para volúmenes)
//    • FFTW_ESTIMATE: modo rápido, usa heurísticas en lugar de medir (< 1ms)
//      Alternativas:
//      - FFTW_MEASURE: prueba varios algoritmos y elige el más rápido (~1 segundo)
//      - FFTW_PATIENT: búsqueda exhaustiva (~10 segundos)
//      - FFTW_EXHAUSTIVE: prueba todo combinatoriamente (~minutos)
//
// RENDIMIENTO:
// Para 1024 muestras:
// - DFT naive: O(N²) = ~1,000,000 operaciones (~5 ms en PC moderna)
// - FFT: O(N log N) = ~10,000 operaciones (~0.05 ms) → Speedup 100×
//
// MEMORIA:
// - Entrada: sample_count × 8 bytes (doubles)
// - Salida: (sample_count/2 + 1) × 16 bytes (complejos)
// - Para 3840 muestras: ~30 KB entrada + ~31 KB salida = ~61 KB total
// ════════════════════════════════════════════════════════════════════════════════════════

FFT::FFT(int sample_count) :
        samples_size(sample_count),
        amplitudes_size(sample_count / 2 + 1),  // Solo frecuencias positivas (simetría de Hermite)
        samples(sample_count),
        amplitudes(amplitudes_size)
{
    // Reservar memoria alineada para salida compleja (crucial para SIMD)
    complex = (fftw_complex*)fftw_malloc(amplitudes_size * sizeof(fftw_complex));
    
    // Crear plan de ejecución optimizado (real → complejo, 1D)
    // FFTW_ESTIMATE: usa heurísticas rápidas sin medir rendimiento
    p = fftw_plan_dft_r2c_1d(sample_count, samples.data(), complex, FFTW_ESTIMATE);
}

FFT::~FFT()
{
    fftw_free(complex);
}

void FFT::Plot(double sampling_frequency) {
    // Dibujar espectro con color verde #1CC809
    ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    
    // PlotStems: gr�fico de barras verticales (ideal para espectros discretos)
    // El espaciado entre frecuencias es sampling_frequency / samples_size
    ImPlot::PlotStems("", amplitudes.data(), amplitudes_size, 0, sampling_frequency / samples_size);
    
    ImPlot::PopStyleColor();
}

void FFT::SetData(const double* data, uint32_t count) {
    if (count >= samples_size)
        count = samples_size;
    else
        // Si hay menos muestras que el tama�o del buffer, rellenar con ceros (zero-padding)
        std::fill(samples.begin() + count, samples.end(), 0);

    // Copiar datos de entrada al buffer interno
    std::copy(data, data + count, samples.begin());
}

void FFT::Compute() {
    // Ejecutar la FFT seg�n el plan precomputado
    fftw_execute(p);
    
    // Convertir n�meros complejos a magnitudes (amplitudes de frecuencia)
    // Dividir por amplitudes_size para normalizar
    std::transform(complex, complex + amplitudes_size, amplitudes.begin(), [&](const fftw_complex complex) {
        return sqrt(complex[0] * complex[0] + complex[1] * complex[1]) / amplitudes_size;
    });

    // La primera componente (�ndice 0) es el offset DC (frecuencia 0)
    offset = amplitudes[0];

    // Buscar la frecuencia con mayor amplitud (excluyendo DC)
    n_frequency = 1;
    double max_frequency = amplitudes[1];
    for (int i = 1; i < amplitudes.size(); i++)
    {
        if (amplitudes[i] > max_frequency) {
            max_frequency = amplitudes[i];
            n_frequency = i;
        }
    }
}

double FFT::Offset() const
{
    return offset;
}

double FFT::Frequency(double sampling_frequency) const
{
    // Convertir �ndice de frecuencia a Hz
    // Resoluci�n de frecuencia = sampling_frequency / samples_size
    return n_frequency * sampling_frequency / samples_size;
}
std::vector<Harmonic> FFT::FindHarmonics(double sampling_frequency, int count) {
	detected_harmonics.clear();
	
	// Validar que hay datos disponibles
	if (amplitudes.empty() || n_frequency == 0)
		return detected_harmonics;
	
	// ════════════════════════════════════════════════════════════════════════════════════════
	// ALGORITMO DE DETECCIÓN DE ARMÓNICAS
	// ════════════════════════════════════════════════════════════════════════════════════════
	//
	// CONCEPTO:
	// Las armónicas son frecuencias múltiplos enteros de una frecuencia fundamental.
	// Para una señal con f₀ = 440 Hz (nota La):
	//   - 1ª armónica: 440 Hz (fundamental)
	//   - 2ª armónica: 880 Hz (octava)
	//   - 3ª armónica: 1320 Hz (quinta + octava)
	//
	// ESTRATEGIA:
	// 1. Usar la frecuencia dominante detectada en Compute() como fundamental
	// 2. Para cada armónica n, buscar pico en n × f₀
	// 3. Permitir tolerancia (±3 bins) por resolución finita de FFT
	//
	// EJEMPLO PRÁCTICO:
	// fs = 3840 Hz, N = 3840 muestras → Resolución = 1 Hz/bin
	// Si f₀ = 440.2 Hz:
	//   - Target bin para 2ª armónica: 880 Hz → bin 880
	//   - Búsqueda en bins 877-883
	//   - Detecta pico en bin 881 → 881 Hz exacto
	//
	// LIMITACIONES:
	// - Solo detecta hasta frecuencia de Nyquist (fs/2)
	// - Resolución frecuencial limitada por tamaño de ventana
	// - Puede haber falsos positivos si hay ruido significativo
	// ════════════════════════════════════════════════════════════════════════════════════════
	
	// Paso 1: Calcular frecuencia fundamental (ya detectada en Compute())
	double fundamental_freq = n_frequency * sampling_frequency / samples_size;
	
	// Paso 2: Buscar las 'count' primeras armónicas (1×f₀, 2×f₀, 3×f₀, ...)
	for (int n = 1; n <= count; n++) {
		// Calcular frecuencia objetivo de esta armónica
		double target_freq = fundamental_freq * n;
		
		// Convertir frecuencia (Hz) a índice de bin en el espectro FFT
		// Fórmula: bin = freq × N / fs
		int target_bin = static_cast<int>(std::round(target_freq * samples_size / sampling_frequency));
		
		// Verificar límite de Nyquist (fs/2)
		if (target_bin >= amplitudes_size)
			break;  // Armónica fuera de rango útil
		
		// Paso 3: Buscar pico local alrededor del bin objetivo
		// Ventana de ±3 bins tolera variaciones por:
		//   - Resolución finita de FFT
		//   - Spectral leakage (fuga espectral)
		//   - Ligeras variaciones en frecuencia de oscilador
		int search_start = std::max(1, target_bin - 3);
		int search_end = std::min(amplitudes_size - 1, target_bin + 3);
		
		int peak_bin = target_bin;
		double peak_amplitude = amplitudes[target_bin];
		
		// Encontrar el bin con máxima amplitud en la ventana de búsqueda
		for (int i = search_start; i <= search_end; i++) {
			if (amplitudes[i] > peak_amplitude) {
				peak_amplitude = amplitudes[i];
				peak_bin = i;
			}
		}
		
		// Paso 4: Calcular frecuencia exacta del pico encontrado
		// Fórmula: freq = bin × fs / N
		double exact_freq = peak_bin * sampling_frequency / samples_size;
		
		// Paso 5: Crear y almacenar información de la armónica detectada
		Harmonic h;
		h.frequency = exact_freq;     // Frecuencia en Hz
		h.amplitude = peak_amplitude; // Amplitud en Voltios (normalizada)
		h.bin_index = peak_bin;       // Índice del bin para referencia
		
		detected_harmonics.push_back(h);
	}
	
	return detected_harmonics;
}

double FFT::GetAmplitudeAt(int bin) const {
	if (bin >= 0 && bin < amplitudes_size)
		return amplitudes[bin];
	return 0.0;
}