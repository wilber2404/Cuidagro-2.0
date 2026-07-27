# CUIDRAGRO

Prototipo IoT para el monitoreo en tiempo real de variables ambientales (temperatura, humedad relativa, humedad del suelo y pH) en cultivos de café. Caso de aplicación: **Finca Villa Dolly**, vereda Bajo Tablazo, Manizales, Caldas.

Proyecto de grado 202016907 — Ingeniería de Sistemas, UNAD.

Integrantes
- Wilber Domínguez Mosquera

## Estructura del repositorio

```
CUIDRAGRO/
├── firmware/        # Código del microcontrolador ESP32 (Arduino IDE)
├── backend/         # API REST (Node.js + Express + MySQL)
├── frontend/        # Dashboard web de visualización en tiempo real
├── database/        # Script SQL del esquema de base de datos
└── docs/            # Diagramas de arquitectura y diseño
```

## Arquitectura general

1. **Capa de percepción:** sensores DHT22 (temperatura/humedad), sensor capacitivo de humedad de suelo y sensor de pH.
2. **Capa de procesamiento y comunicación:** microcontrolador ESP32, que adquiere las señales analógicas/digitales y las transmite vía WiFi mediante HTTP POST en formato JSON.
3. **Capa de almacenamiento:** base de datos MySQL que persiste cada lectura con marca de tiempo.
4. **Capa de aplicación:** API REST en Node.js/Express y dashboard web para visualización en tiempo real e histórico.

## Puesta en marcha

### 1. Base de datos
```bash
mysql -u root -p < database/schema.sql
```

### 2. Backend
```bash
cd backend
cp .env.example .env   # ajustar credenciales de MySQL
npm install
npm start
```

### 3. Firmware ESP32
Abrir `firmware/CUIDRAGRO_ESP32/CUIDRAGRO_ESP32.ino` en el Arduino IDE, instalar las librerías `DHT sensor library` y `ArduinoJson`, configurar el SSID/contraseña de la red y la URL del servidor, y cargar al ESP32.

### 4. Dashboard
Abrir `frontend/index.html` en el navegador (con el backend corriendo en `localhost:3000`).

## Licencia académica

Desarrollado con fines académicos para la Universidad Nacional Abierta y a Distancia (UNAD).
