// FFT.h - Análisis de frecuencia mediante Transformada Rápida de Fourier
//
// La clase FFT encapsula la funcionalidad de FFTW3 para realizar análisis
// espectral de señales en tiempo real. Calcula las amplitudes de las frecuencias
// presentes en una señal y permite identificar la frecuencia dominante y el offset DC.
//
// Características:
// - Usa FFTW3 (Fastest Fourier Transform in the West) para cálculos optimizados
// - Calcula solo las frecuencias positivas (transformada real a compleja)
// - Identifica automáticamente la frecuencia dominante
// - Calcula el offset DC (componente de frecuencia 0)
// - Interfaz simple para visualización con ImPlot

#pragma once

#include <fftw3.h>
#include <vector>

class FFT {
	fftw_complex* complex;    // Salida de la FFT (números complejos)
	fftw_plan p;              // Plan de ejecución de FFTW (optimizado)

	int samples_size;         // Tamaño del buffer de entrada (muestras temporales)
	int amplitudes_size;      // Tamaño del buffer de salida (frecuencias)
	std::vector<double> samples;     // Buffer de entrada (dominio del tiempo)
	std::vector<double> amplitudes;  // Buffer de salida (magnitudes de frecuencias)

	double offset = 0;        // Offset DC (componente de frecuencia 0)
	int n_frequency = 0;      // Índice de la frecuencia dominante

public:
	// Constructor
	// sample_count: número de muestras a analizar (debe ser potencia de 2 para mejor rendimiento)
	explicit FFT(int sample_count);
	
	~FFT();

	// Dibuja el espectro de frecuencias usando ImPlot
	// sampling_frequency: frecuencia de muestreo en Hz (determina el rango de frecuencias)
	void Plot(double sampling_frequency);

	// Carga datos para análisis FFT
	// data: puntero a array de muestras en dominio del tiempo
	// count: cantidad de muestras (si es menor que sample_count, se rellena con ceros)
	void SetData(const double* data, uint32_t count);

	// Ejecuta la FFT y calcula las amplitudes de frecuencia
	// También identifica la frecuencia dominante y el offset DC
	void Compute();

	// Retorna el offset DC (componente de frecuencia 0) de la señal
	double Offset() const;
	
	// Retorna la frecuencia dominante en Hz
	// sampling_frequency: frecuencia de muestreo usada al capturar la señal
	double Frequency(double sampling_frequency) const;
};