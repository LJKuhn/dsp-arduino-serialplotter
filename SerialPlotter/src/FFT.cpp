// FFT.cpp - Implementación de análisis espectral mediante FFT
//
// Usa la librería FFTW3 para realizar transformadas de Fourier optimizadas.
// Calcula el espectro de amplitudes desde señales en dominio del tiempo.

#include "FFT.h"

#include <algorithm>
#include <implot.h>
#include <cmath>

// Calcula la magnitud de un número complejo (sqrt(real² + imag²))
double magnitude(const fftw_complex complex) {
    return std::sqrt(complex[0] * complex[0] + complex[1] * complex[1]);
}

FFT::FFT(int sample_count) :
        samples_size(sample_count),
        amplitudes_size(sample_count / 2 + 1),  // Solo frecuencias positivas (simetría de Hermite)
        samples(sample_count),
        amplitudes(amplitudes_size)
{
    // Reservar memoria para la salida compleja de FFTW
    complex = (fftw_complex*)fftw_malloc(amplitudes_size * sizeof(fftw_complex));
    
    // Crear plan de FFTW (real a complejo, 1D)
    // FFTW_ESTIMATE: usa heurísticas rápidas en lugar de medir rendimiento
    p = fftw_plan_dft_r2c_1d(sample_count, samples.data(), complex, FFTW_ESTIMATE);
}

FFT::~FFT()
{
    fftw_free(complex);
}

void FFT::Plot(double sampling_frequency) {
    // Dibujar espectro con color verde #1CC809
    ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    
    // PlotStems: gráfico de barras verticales (ideal para espectros discretos)
    // El espaciado entre frecuencias es sampling_frequency / samples_size
    ImPlot::PlotStems("", amplitudes.data(), amplitudes_size, 0, sampling_frequency / samples_size);
    
    ImPlot::PopStyleColor();
}

void FFT::SetData(const double* data, uint32_t count) {
    if (count >= samples_size)
        count = samples_size;
    else
        // Si hay menos muestras que el tamaño del buffer, rellenar con ceros (zero-padding)
        std::fill(samples.begin() + count, samples.end(), 0);

    // Copiar datos de entrada al buffer interno
    std::copy(data, data + count, samples.begin());
}

void FFT::Compute() {
    // Ejecutar la FFT según el plan precomputado
    fftw_execute(p);
    
    // Convertir números complejos a magnitudes (amplitudes de frecuencia)
    // Dividir por amplitudes_size para normalizar
    std::transform(complex, complex + amplitudes_size, amplitudes.begin(), [&](const fftw_complex complex) {
        return sqrt(complex[0] * complex[0] + complex[1] * complex[1]) / amplitudes_size;
    });

    // La primera componente (índice 0) es el offset DC (frecuencia 0)
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
    // Convertir índice de frecuencia a Hz
    // Resolución de frecuencia = sampling_frequency / samples_size
    return n_frequency * sampling_frequency / samples_size;
}
