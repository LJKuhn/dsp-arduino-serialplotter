// FFT.h - An�lisis de frecuencia mediante Transformada R�pida de Fourier
//
// La clase FFT encapsula la funcionalidad de FFTW3 para realizar an�lisis
// espectral de se�ales en tiempo real. Calcula las amplitudes de las frecuencias
// presentes en una se�al y permite identificar la frecuencia dominante y el offset DC.
//
// Caracter�sticas:
// - Usa FFTW3 (Fastest Fourier Transform in the West) para c�lculos optimizados
// - Calcula solo las frecuencias positivas (transformada real a compleja)
// - Identifica autom�ticamente la frecuencia dominante
// - Calcula el offset DC (componente de frecuencia 0)
// - Interfaz simple para visualizaci�n con ImPlot

#pragma once

#include <fftw3.h>
#include <vector>

// Estructura para almacenar información de armónicas detectadas
struct Harmonic {
	double frequency;   // Frecuencia en Hz
	double amplitude;   // Amplitud en Voltios
	int bin_index;      // Índice del bin en el espectro FFT
};

class FFT {
	fftw_complex* complex;    // Salida de la FFT (n�meros complejos)
	fftw_plan p;              // Plan de ejecuci�n de FFTW (optimizado)

	int samples_size;         // Tama�o del buffer de entrada (muestras temporales)
	int amplitudes_size;      // Tama�o del buffer de salida (frecuencias)
	std::vector<double> samples;     // Buffer de entrada (dominio del tiempo)
	std::vector<double> amplitudes;  // Buffer de salida (magnitudes de frecuencias)

	double offset = 0;        // Offset DC (componente de frecuencia 0)
	int n_frequency = 0;      // �ndice de la frecuencia dominante
	std::vector<Harmonic> detected_harmonics;  // Almacena las armónicas detectadas
public:
	// Constructor
	// sample_count: n�mero de muestras a analizar (debe ser potencia de 2 para mejor rendimiento)
	explicit FFT(int sample_count);
	
	~FFT();

	// Dibuja el espectro de frecuencias usando ImPlot
	// sampling_frequency: frecuencia de muestreo en Hz (determina el rango de frecuencias)
	void Plot(double sampling_frequency);

	// Carga datos para an�lisis FFT
	// data: puntero a array de muestras en dominio del tiempo
	// count: cantidad de muestras (si es menor que sample_count, se rellena con ceros)
	void SetData(const double* data, uint32_t count);

	// Ejecuta la FFT y calcula las amplitudes de frecuencia
	// Tambi�n identifica la frecuencia dominante y el offset DC
	void Compute();

	// Retorna el offset DC (componente de frecuencia 0) de la se�al
	double Offset() const;
	
	// Retorna la frecuencia dominante en Hz
	// sampling_frequency: frecuencia de muestreo usada al capturar la se�al
	double Frequency(double sampling_frequency) const;	
	// Acceso al espectro completo de amplitudes
	const std::vector<double>& GetAmplitudes() const { return amplitudes; }
	int GetAmplitudesSize() const { return amplitudes_size; }
	
	// Detección de armónicas (múltiplos de la frecuencia fundamental)
	// sampling_frequency: frecuencia de muestreo en Hz
	// count: número de armónicas a detectar (por defecto 3)
	// Retorna: vector con información de cada armónica detectada
	std::vector<Harmonic> FindHarmonics(double sampling_frequency, int count = 3);
	
	// Obtener amplitud de un bin específico del espectro
	double GetAmplitudeAt(int bin) const;};