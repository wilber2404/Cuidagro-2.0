/**
 * ============================================================================
 * Proyecto CUIDRAGRO - Backend (API REST)
 * Recibe lecturas del nodo ESP32, las almacena en MySQL, evalúa umbrales
 * de alerta y expone endpoints para el dashboard web.
 * ============================================================================
 */

require('dotenv').config();
const express = require('express');
const cors = require('cors');
const mysql = require('mysql2/promise');

const app = express();
app.use(cors());
app.use(express.json());

const PORT = process.env.PORT || 3000;

const pool = mysql.createPool({
  host: process.env.DB_HOST || 'localhost',
  user: process.env.DB_USER || 'root',
  password: process.env.DB_PASSWORD || '',
  database: process.env.DB_NAME || 'cuidragro',
  waitForConnections: true,
  connectionLimit: 10
});

// ---------------------------------------------------------------------------
// POST /api/lecturas -> el ESP32 envía una nueva lectura
// ---------------------------------------------------------------------------
app.post('/api/lecturas', async (req, res) => {
  try {
    const { dispositivo, temperatura, humedad_aire, humedad_suelo, ph } = req.body;

    if ([dispositivo, temperatura, humedad_aire, humedad_suelo, ph].some(v => v === undefined)) {
      return res.status(400).json({ error: 'Faltan campos obligatorios en la lectura.' });
    }

    const [dispositivos] = await pool.query(
      'SELECT id FROM dispositivos WHERE codigo = ?', [dispositivo]
    );
    if (dispositivos.length === 0) {
      return res.status(404).json({ error: 'Dispositivo no registrado.' });
    }
    const dispositivoId = dispositivos[0].id;

    const [resultado] = await pool.query(
      `INSERT INTO lecturas (dispositivo_id, temperatura, humedad_aire, humedad_suelo, ph)
       VALUES (?, ?, ?, ?, ?)`,
      [dispositivoId, temperatura, humedad_aire, humedad_suelo, ph]
    );

    await evaluarUmbrales(resultado.insertId, { temperatura, humedad_aire, humedad_suelo, ph });

    res.status(201).json({ mensaje: 'Lectura registrada correctamente.', id: resultado.insertId });
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error interno al registrar la lectura.' });
  }
});

// ---------------------------------------------------------------------------
// GET /api/lecturas -> últimas lecturas (para el dashboard)
// ---------------------------------------------------------------------------
app.get('/api/lecturas', async (req, res) => {
  try {
    const limite = parseInt(req.query.limite) || 50;
    const [filas] = await pool.query(
      `SELECT l.id, d.nombre AS dispositivo, l.temperatura, l.humedad_aire,
              l.humedad_suelo, l.ph, l.fecha_registro
       FROM lecturas l
       JOIN dispositivos d ON d.id = l.dispositivo_id
       ORDER BY l.fecha_registro DESC
       LIMIT ?`,
      [limite]
    );
    res.json(filas);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error al consultar las lecturas.' });
  }
});

// ---------------------------------------------------------------------------
// GET /api/lecturas/actual -> última lectura registrada
// ---------------------------------------------------------------------------
app.get('/api/lecturas/actual', async (req, res) => {
  try {
    const [filas] = await pool.query(
      `SELECT l.*, d.nombre AS dispositivo
       FROM lecturas l
       JOIN dispositivos d ON d.id = l.dispositivo_id
       ORDER BY l.fecha_registro DESC LIMIT 1`
    );
    res.json(filas[0] || null);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error al consultar la última lectura.' });
  }
});

// ---------------------------------------------------------------------------
// GET /api/alertas -> historial de alertas generadas
// ---------------------------------------------------------------------------
app.get('/api/alertas', async (req, res) => {
  try {
    const [filas] = await pool.query(
      `SELECT * FROM alertas ORDER BY fecha_alerta DESC LIMIT 50`
    );
    res.json(filas);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error al consultar las alertas.' });
  }
});

// ---------------------------------------------------------------------------
// Función auxiliar: evalúa si una lectura excede los umbrales configurados
// ---------------------------------------------------------------------------
async function evaluarUmbrales(lecturaId, valores) {
  const [umbrales] = await pool.query('SELECT * FROM umbrales');
  const mapaCampos = {
    temperatura: 'temperatura',
    humedad_aire: 'humedad_aire',
    humedad_suelo: 'humedad_suelo',
    ph: 'ph'
  };

  for (const umbral of umbrales) {
    const campo = mapaCampos[umbral.variable];
    if (!campo) continue;
    const valor = valores[campo];

    if (valor < umbral.valor_minimo || valor > umbral.valor_maximo) {
      const mensaje = `${umbral.variable} fuera de rango: ${valor} (esperado ${umbral.valor_minimo}-${umbral.valor_maximo})`;
      await pool.query(
        `INSERT INTO alertas (lectura_id, variable, valor, mensaje) VALUES (?, ?, ?, ?)`,
        [lecturaId, umbral.variable, valor, mensaje]
      );
    }
  }
}

app.get('/', (req, res) => {
  res.json({ proyecto: 'CUIDRAGRO', estado: 'API en funcionamiento' });
});

app.listen(PORT, () => {
  console.log(`Servidor CUIDRAGRO escuchando en el puerto ${PORT}`);
});
