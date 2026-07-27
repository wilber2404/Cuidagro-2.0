-- ============================================================================
-- Proyecto CUIDRAGRO - Esquema de base de datos
-- Motor: MySQL 8.0
-- ============================================================================

CREATE DATABASE IF NOT EXISTS cuidragro
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE cuidragro;

-- Tabla de dispositivos (nodos ESP32) registrados en el sistema
CREATE TABLE IF NOT EXISTS dispositivos (
  id            INT AUTO_INCREMENT PRIMARY KEY,
  codigo        VARCHAR(50) NOT NULL UNIQUE,
  nombre        VARCHAR(100) NOT NULL,
  ubicacion     VARCHAR(150) DEFAULT 'Finca Villa Dolly, Manizales, Caldas',
  activo        BOOLEAN NOT NULL DEFAULT TRUE,
  fecha_registro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Tabla de lecturas de variables ambientales
CREATE TABLE IF NOT EXISTS lecturas (
  id              BIGINT AUTO_INCREMENT PRIMARY KEY,
  dispositivo_id  INT NOT NULL,
  temperatura     DECIMAL(5,2)  NOT NULL COMMENT 'Grados Celsius',
  humedad_aire    DECIMAL(5,2)  NOT NULL COMMENT 'Porcentaje (%)',
  humedad_suelo   DECIMAL(5,2)  NOT NULL COMMENT 'Porcentaje (%)',
  ph              DECIMAL(4,2)  NOT NULL COMMENT 'Escala 0-14',
  fecha_registro  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (dispositivo_id) REFERENCES dispositivos(id),
  INDEX idx_fecha (fecha_registro)
);

-- Tabla de umbrales de alerta por variable
CREATE TABLE IF NOT EXISTS umbrales (
  id            INT AUTO_INCREMENT PRIMARY KEY,
  variable      VARCHAR(30) NOT NULL,
  valor_minimo  DECIMAL(5,2) NOT NULL,
  valor_maximo  DECIMAL(5,2) NOT NULL
);

-- Tabla de alertas generadas cuando una lectura excede un umbral
CREATE TABLE IF NOT EXISTS alertas (
  id            BIGINT AUTO_INCREMENT PRIMARY KEY,
  lectura_id    BIGINT NOT NULL,
  variable      VARCHAR(30) NOT NULL,
  valor         DECIMAL(6,2) NOT NULL,
  mensaje       VARCHAR(255) NOT NULL,
  fecha_alerta  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (lectura_id) REFERENCES lecturas(id)
);

-- Datos iniciales
INSERT INTO dispositivos (codigo, nombre, ubicacion) VALUES
  ('cuidragro-esp32-001', 'Nodo sensor - Lote 1', 'Finca Villa Dolly, Manizales, Caldas')
  ON DUPLICATE KEY UPDATE nombre = VALUES(nombre);

INSERT INTO umbrales (variable, valor_minimo, valor_maximo) VALUES
  ('temperatura', 18.0, 24.0),
  ('humedad_aire', 60.0, 90.0),
  ('humedad_suelo', 40.0, 80.0),
  ('ph', 5.5, 6.5);
