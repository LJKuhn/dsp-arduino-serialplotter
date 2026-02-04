// Buffers.h - Implementaciones de buffers circulares para visualización de datos en tiempo real
//
// ScrollBuffer<T>:
// - Buffer circular optimizado para visualización continua de señales
// - Mantiene una ventana deslizante de tamaño fijo (view) sobre datos acumulados
// - Gestiona automáticamente el offset cuando los datos exceden la capacidad de vista
// - Ideal para gráficos de tiempo real donde solo se visualizan los últimos N puntos
//
// Buffer<T>:
// - Buffer circular thread-safe con lectura/escritura concurrente
// - Usa índices atómicos (start/end) para evitar condiciones de carrera
// - Protegido con mutex durante operaciones de copia de datos
// - Diseñado para comunicación productor-consumidor

#pragma once

#include <functional>
#include <future>
#include <format>
#include <implot.h>

// ScrollBuffer - Buffer circular con ventana deslizante para visualización
// Template genérico que funciona con cualquier tipo numérico (int, double, float, etc.)
template <typename T>
class ScrollBuffer {
public:
	// Constructor
	// capacity: tamaño máximo del buffer interno (cantidad de elementos que puede almacenar)
	// view: tamaño de la ventana visible (cantidad de elementos que se mostrarán)
	ScrollBuffer(uint32_t capacity, uint32_t view) :
		capacity(capacity), view(view)
	{
		m_data = new T[capacity];
	}

	// Constructor de copia - crea una copia independiente del buffer
	ScrollBuffer(const ScrollBuffer& other) {
		capacity = other.capacity;
		m_size = other.m_size;
		view = other.view;
		m_data = new T[capacity];
		uint32_t copy = view < m_size ? view : m_size;
		for (size_t i = 0; i < copy; i++)
			m_data[i] = other.m_data[i + offset];
	}

	// Constructor de movimiento - transfiere la propiedad del buffer
	ScrollBuffer(ScrollBuffer&& other) noexcept {
		capacity = other.capacity;
		m_size = other.m_size;
		view = other.view;
		offset = other.offset;
		m_data = other.m_data;
		other.m_data = nullptr;
	}

	// Operador de asignación por copia
	ScrollBuffer& operator=(const ScrollBuffer& other) {
		if (capacity < other.capacity) {
			delete[] m_data;
			m_data = new T[other.capacity];
		}

		view = other.view;
		m_size = other.m_size;
		offset = 0;
		uint32_t copy = view < m_size ? view : m_size;
		for (size_t i = 0; i < copy; i++)
			m_data[i] = other.m_data[i + offset];

        return *this;
	}

	// Operador de asignación por movimiento
	ScrollBuffer& operator=(ScrollBuffer&& other) {
		delete[] m_data;
		m_size = other.m_size;
		capacity = other.capacity;
		view = other.view;
		offset = other.offset;
		m_data = other.m_data;
		other.m_data = nullptr;
        return *this;
	}

	~ScrollBuffer() {
		delete[] m_data;
	}

	// Escribe múltiples elementos desde un buffer externo
	// Si count > view, solo se escriben los últimos 'view' elementos
	// Gestiona automáticamente el desplazamiento circular del buffer
	void write(T* buffer, uint32_t count) {
		if (count > view) {
			buffer += count - view;
			count = view;
		}

		uint32_t new_size = m_size + count;
		if (new_size <= capacity) {
			std::copy(buffer, &buffer[count], &m_data[m_size]);
			m_size = new_size;
			if (m_size > view)
				offset = new_size - view;
		}
		else if (count == view) {
			std::copy(buffer, &buffer[count], m_data);
			m_size = view;
			offset = 0;
		}
		else {
			uint32_t left_count = view - count;
			uint32_t left = capacity - left_count;
			std::copy(buffer, &buffer[count], &m_data[left_count]);
			std::copy(&m_data[left], &m_data[m_size], m_data);
			m_size = view;
			offset = 0;
		}
	}

	// Limpia el buffer (reinicia índices sin liberar memoria)
	void clear() {
		offset = 0;
		m_size = 0;
	}

	// Agrega un único elemento al final del buffer
	// Si el buffer está lleno, desplaza los datos antiguos automáticamente
	void push(T value) {
		if (m_size == capacity) {
			uint32_t count = view - 1;
			uint32_t start = capacity - count;
			std::copy(&m_data[start], &m_data[capacity], m_data);
			m_data[count] = value;
			m_size = view;
			offset = 0;
			return;
		}

		m_data[m_size++] = value;
		if (m_size > view)
			offset = m_size - view;
	}

	// Devuelve la cantidad de elementos visibles (min(view, m_size))
	uint32_t count() const {
		return view < m_size ? view : m_size;
	}

	// Devuelve el tamaño total de datos almacenados
	uint32_t size() const {
		return m_size;
	}

	// Acceso al primer elemento visible
	T& front() {
		return m_data[offset];
	}

	// Acceso al último elemento visible
	T& back() {
		return m_data[offset + count() - 1];
	}

	// Acceso por índice (relativo a la ventana visible)
	T& operator[](uint32_t index) {
		return m_data[(offset + index) % capacity];
	}

	// Puntero directo a los datos visibles (útil para gráficos)
	const T* data() const {
		return m_data + offset;
	}

private:

	uint32_t capacity, view, m_size = 0, offset = 0;
	T* m_data;
};

// Buffer - Buffer circular thread-safe para comunicación productor-consumidor
// Usa índices atómicos y mutex para garantizar seguridad en entornos multi-hilo
template <typename T>
class Buffer {
public:
	// Constructor - reserva capacidad + 1 para distinguir entre lleno y vacío
	Buffer(int capacity) : _capacity(capacity + 1) {
		data = new T[capacity + 1];
	}

	// Constructor de copia
	Buffer(const Buffer& other) {
		_capacity = other._capacity;
		data = new T[_capacity];
		for (size_t i = 0; i < _capacity; i++)
			data[i] = other.data[i];
	}

	// Constructor de movimiento
	Buffer(Buffer&& other) {
		_capacity = other._capacity;
		data = other.data;
		other.data = nullptr;
	}

	// Operador de asignación por copia
	Buffer& operator=(const Buffer& other) {
		if (_capacity < other._capacity) {
			delete[] data;
			data = new T[other._capacity];
		}

		_capacity = other._capacity;
		for (size_t i = 0; i < _capacity; i++)
			data[i] = other.data[i];
	}

	// Operador de asignación por movimiento
	Buffer& operator=(Buffer&& other) {
		delete[] data;
		_capacity = other._capacity;
		data = other.data;
		other.data = nullptr;
	}

	~Buffer() {
		delete[] data;
	}

	// Capacidad útil del buffer (sin contar el espacio de guardia)
	size_t capacity() const {
		return _capacity - 1;
	}

	// Cantidad de elementos actualmente almacenados
	size_t size() const {
		size_t count = end - start;
		if (end < start)
			count += _capacity;
		return count;
	}

	// Espacio disponible para escritura
	size_t available() const {
		return _capacity - size() - 1;
	}

	// Escribe datos en el buffer (thread-safe)
	// Retorna la cantidad de elementos realmente escritos
	size_t write(const T* buffer, int count) {
		size_t free = available();
		if (free == 0)
			return 0;

		if (count > free)
			count = free;

		size_t right_free = _capacity - end;
		size_t right_count = count < right_free ? count : right_free;
		size_t left_count = count - right_count;

		std::lock_guard guard(data_mutex);
		std::copy(buffer, &buffer[right_count], &data[end]);
		if (left_count > 0)
			std::copy(&buffer[right_count], &buffer[count], data);

		end = (end + count) % _capacity;
		return count;
	}

	// Lee datos del buffer (thread-safe)
	// Retorna la cantidad de elementos realmente leídos
	size_t read(T* buffer, int count) {
		size_t length = size();
		if (length == 0)
			return 0;

		if (count > length)
			count = length;

		size_t right_available = _capacity - start;
		size_t right_count = count < right_available ? count : right_available;
		size_t left_count = count - right_count;

		std::lock_guard guard(data_mutex);
		std::copy(&data[start], &data[start + right_count], buffer);
		if (left_count > 0)
			std::copy(data, &data[left_count], &buffer[right_count]);

		start = (start + count) % _capacity;
		return count;
	}

	// Limpia el buffer
	void clear() {
		start = end = 0;
	}

	// Descarta 'count' elementos sin leerlos
	void skip(size_t count) {
		size_t length = size();
		if (count > length)
			count = length;
		start = (start + count) % _capacity;
	}

	// Acceso por índice (relativo al inicio del buffer)
	T& operator[](size_t i) {
		if (i < 0 || i >= size())
			throw std::exception("Out of index");

		size_t real_pos = (i + start) % _capacity;
		return data[real_pos];
	}

	// Debug: imprime el contenido del buffer en consola
	void print() const {
		for (size_t i = 0; i < _capacity; i++)
			std::cout << data[i] << ", ";
		std::cout << "\b\b   \n";
		for (size_t i = 0; i < size(); i++)
			std::cout << data[(i + start) % _capacity] << ", ";
		std::cout << "\b\b   \n";
	}

private:

	int _capacity;
	std::atomic_size_t start = 0, end = 0;  // Índices atómicos para thread-safety
	T* data;

	std::mutex data_mutex;  // Protege las operaciones de copia de datos
};